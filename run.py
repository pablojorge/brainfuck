#!/usr/bin/env python3

import os
import sys
import json
import time
import contextlib
import subprocess


@contextlib.contextmanager
def change_dir(dirname):
    curdir = os.getcwd()
    try:
        os.chdir(dirname)
        yield
    finally:
        os.chdir(curdir)


class Command(object):
    def __init__(self, cmd, args, input_, parser):
        self.__cmd = cmd
        self.__args = args
        self.__input = input_
        self.__parser = parser

    def __getattr__(self, name):
        return Command(self.__cmd + [name], self.__args, self.__input, self.__parser)

    def _parser(self, parser):
        return Command(self.__cmd, self.__args, self.__input, parser)

    def _input(self, input_):
        return Command(self.__cmd, self.__args, input_, self.__parser)

    def _args(self, *args, **kwargs):
        fix_underscore = lambda s: s.replace('_', '-')

        _args = []

        for arg, value in kwargs.items():
            _args.append("--%s=%s" % (fix_underscore(arg), value))

        for arg in args:
            _args.append(arg)

        return Command(self.__cmd, _args, self.__input, self.__parser)

    def __build(self):
        fix_underscore = lambda s: s.replace('_', '-')
        return list(map(fix_underscore, self.__cmd)) + self.__args

    def __str__(self):
        return "Command({})".format(self.__build())

    def _run(self):
        try:
            process = subprocess.run(
                self.__build(),
                input=self.__input,
                capture_output=True,
                encoding='utf8',
                check=True
            )
        except subprocess.CalledProcessError as p:
            raise Exception(p.stderr)

        return self.__parser(process.stdout)

    def __call__(self, *args, **kwargs):
        return self._args(*args, **kwargs)._run()


def command(program, args=[], input_="", parser=str):
    return Command([program], args=args, input_=input_, parser=parser)


class Timer(object):
    def __init__(self):
        self.__start, self.__end = None, None

    def start(self): self.__start = time.time()
    def stop(self):  self.__end   = time.time()

    def __enter__(self):
        self.start()
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        self.stop()

    def elapsed(self):
        assert self.__start is not None, "start() not called"
        end = self.__end if self.__end is not None else time.time()
        return end - self.__start

    def get_ms(self): return int(round(self.elapsed() * 1000, 0))

    def __str__(self): return "{}ms".format(self.get_ms())


non_empty = lambda l: [x for x in l if x]
git = command("git", parser=lambda s: non_empty(s.split("\n")))


class WorkingCopy(object):
    def __init__(self, path):
        self.__path = os.path.abspath(path)

    def commit(self):
        with change_dir(self.__path):
            return git.show()[0].split()[1]

    def tags(self):
        with change_dir(self.__path):
            return git.tag("-l", points_at=self.commit())

    def dirty(self):
        with change_dir(self.__path):
            return any(git.status("--porcelain"))

    def branch(self):
        with change_dir(self.__path):
            return next(filter(lambda x: x.startswith('*'), git.branch())).split()[1]

    def __str__(self):
        return "%s%s (%s) [%s]" % (
            self.commit(),
            "-dirty" if self.dirty() else "",
            self.branch(),
            ", ".join(self.tags())
        )


make = command("make", parser=lambda s: non_empty(s.split("\n")))
cargo = command("cargo", parser=lambda s: non_empty(s.split("\n")))


class Environment:
    def __init__(self, desc, cwd):
        self.desc = desc
        self.cwd = cwd

    def builder(self) -> Command: raise NotImplemented()
    def runner(self) -> Command: raise NotImplemented()


class Test:
    def __init__(self, desc):
        self.desc = desc

    def command(self, runner) -> Command: raise NotImplemented()


def run(tests, env):
    with change_dir(env.cwd):
        builder = env.builder()
        print("Building env '{}'...".format(env.desc))
        output = builder._run()

        for test in tests:
            sys.stdout.write("Running '{}' in env '{}'... ".format(test.desc, env.desc))
            sys.stdout.flush()

            with Timer() as timer:
                test.command(env.runner())._run()

            sys.stdout.write("{}\n".format(timer))


class C(Environment):
    def __init__(self):
        super().__init__("C", "./c")

    def builder(self): return make
    def runner(self): return command("./brainfuck")


class CPPClang():
    def builder(self): return make.clean.all._args("CXX=c++")

class CPPGCC():
    def builder(self): return make.clean.all._args("CXX=g++-10")

class CPPADT():
    def runner(self): return command("./brainfuck")

class CPPOOP():
    def runner(self): return command("./brainfuck-oop")


class CPPADTClang(CPPADT, CPPClang, Environment):
    def __init__(self):
        super().__init__("C++-ADT (Clang)", "./cpp")

class CPPOOPClang(CPPOOP, CPPClang, Environment):
    def __init__(self):
        super().__init__("C++-OOP (Clang)", "./cpp")

class CPPADTGCC(CPPADT, CPPGCC, Environment):
    def __init__(self):
        super().__init__("C++-ADT (GCC)", "./cpp")

class CPPOOPGCC(CPPOOP, CPPGCC, Environment):
    def __init__(self):
        super().__init__("C++-OOP (GCC)", "./cpp")

class Rust(Environment):
    def __init__(self):
        super().__init__("Rust", "./rust")

    def builder(self): return cargo.build
    def runner(self): return cargo.run


class Primes(Test):
    def __init__(self, up_to):
        super().__init__("Primes up to {}".format(up_to))
        self.up_to = up_to

    def command(self, runner) -> Command:
        return runner._input("{}\n".format(self.up_to))._args("../programs/primes.bf")


class Mandelbrot(Test):
    def __init__(self):
        super().__init__("Mandelbrot")

    def command(self, runner) -> Command:
        return runner._args("../programs/mandelbrot.bf")


def main():
    print("Running on {}".format(WorkingCopy('.')))

    envs = [
        C(),
        CPPADTClang(),
        CPPADTGCC(),
        CPPOOPClang(),
        CPPOOPGCC(),
        Rust()
    ]
    tests = [
        Primes(200),
        Mandelbrot()
    ]

    for env in envs:
        run(tests, env)


if __name__ == '__main__':
    main()
