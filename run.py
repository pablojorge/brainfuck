#!/usr/bin/env python3

import os
import sys
import json
import time
import functools
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


def merge_dicts(a, b):
    return dict(tuple(a.items()) +
                tuple(b.items()))


class Command(object):
    def __init__(self, cmd, args, kwargs, env, input_, parser):
        self.__cmd = cmd
        self.__args = args
        self.__kwargs = kwargs
        self.__env = env
        self.__input = input_
        self.__parser = parser

    def __getattr__(self, name):
        return Command(self.__cmd + [name],
                       self.__args,
                       self.__kwargs,
                       self.__env, 
                       self.__input, 
                       self.__parser)

    def _parser(self, parser):
        return Command(self.__cmd,
                       self.__args,
                       self.__kwargs, 
                       self.__env, 
                       self.__input,
                       parser)

    def _input(self, input_):
        return Command(self.__cmd,
                       self.__args,
                       self.__kwargs,
                       self.__env, 
                       input_, 
                       self.__parser)

    def _env(self, env):
        return Command(self.__cmd,
                       self.__args,
                       self.__kwargs,
                       merge_dicts(self.__env, env),
                       self.__input, 
                       self.__parser)

    def _args(self, *args, **kwargs):
        args = self.__args + list(args)

        kwargs_ = dict(self.__kwargs)
        kwargs_.update(kwargs)

        return Command(self.__cmd,
                       args,
                       kwargs,
                       self.__env,
                       self.__input,
                       self.__parser)

    def __build(self):
        fix_underscore = lambda s: s.replace('_', '-')

        cmd = list(map(fix_underscore, self.__cmd))

        for arg, value in self.__kwargs.items():
            cmd.append("--%s=%s" % (fix_underscore(arg), value))

        for arg in self.__args:
            cmd.append(arg)

        return cmd

    def __str__(self):
        return "Command({})".format(self.__build())

    def _run(self):
        try:
            process = subprocess.run(
                self.__build(),
                input=self.__input,
                env=merge_dicts(os.environ, self.__env),
                capture_output=True,
                encoding='utf8',
                check=True
            )
        except subprocess.CalledProcessError as p:
            raise Exception(p.stderr)

        return self.__parser(process.stdout)

    def __call__(self, *args, **kwargs):
        return self._args(*args, **kwargs)._run()


def command(program, args=[], kwargs={}, env={}, input_="", parser=str):
    return Command([program], args=args, kwargs=kwargs, env=env, input_=input_, parser=parser)


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
    results = []

    with change_dir(env.cwd):
        builder = env.builder()
        if builder:
            print("Building env '{}'...".format(env.desc))
            output = builder._run()
        else:
            print("Skipping build in env '{}'...".format(env.desc))

        for test in tests:
            sys.stdout.write("Running '{}' in env '{}'... ".format(test.desc, env.desc))
            sys.stdout.flush()

            with Timer() as timer:
                test.command(env.runner())._run()

            sys.stdout.write("{}\n".format(timer))
            results.append(str(timer))

    return results


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
    def runner(self): return command("./brainfuck-adt")

class CPPJIT():
    def runner(self): return command("./brainfuck-jit")

class CPPOOP():
    def runner(self): return command("./brainfuck-oop")


class CPPADTClang(CPPADT, CPPClang, Environment):
    def __init__(self):
        super().__init__("C++-ADT (Clang)", "./cpp")

class CPPJITClang(CPPJIT, CPPClang, Environment):
    def __init__(self):
        super().__init__("C++-JIT (Clang)", "./cpp")

class CPPOOPClang(CPPOOP, CPPClang, Environment):
    def __init__(self):
        super().__init__("C++-OOP (Clang)", "./cpp")

class CPPADTGCC(CPPADT, CPPGCC, Environment):
    def __init__(self):
        super().__init__("C++-ADT (GCC)", "./cpp")

class CPPJITGCC(CPPJIT, CPPGCC, Environment):
    def __init__(self):
        super().__init__("C++-JIT (GCC)", "./cpp")

class CPPOOPGCC(CPPOOP, CPPGCC, Environment):
    def __init__(self):
        super().__init__("C++-OOP (GCC)", "./cpp")

class Rust(Environment):
    def __init__(self):
        super().__init__("Rust", "./rust")

    def builder(self): return cargo.build
    def runner(self): return command("./target/debug/main")

class RustJIT(Environment):
    def __init__(self):
        super().__init__("RustJIT", "./rust")

    def builder(self): return cargo.build
    def runner(self): return command("./target/debug/jit")

class Go(Environment):
    def __init__(self):
        super().__init__("Go", "./go")

    def builder(self): return None
    def runner(self):
        return command("go") \
                ._args("run", "brainfuck.go") \
                ._env({"GOPATH": os.getcwd()})

class PyPy(Environment):
    def __init__(self):
        super().__init__("PyPy", "./python")

    def builder(self): return None
    def runner(self): return command("pypy")._args("./brainfuck-simple.py")


class Hello(Test):
    def __init__(self):
        super().__init__("Hello")

    def command(self, runner) -> Command:
        return runner._args("../programs/hello.bf")


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


def markdown_table(headers, rows, align=-1):
    # prepend or append 'f' 'c' times to 'v', depending
    # on alignment (-1: left, 1: right):
    fill = lambda v, f, c: \
        (f*c) + v if align > 0 else v + (f*c)

    # calculate the width of each column
    widths = functools.reduce(
        lambda a, r: tuple(
            map(max, zip(a, map(len, r)))
        ),
        rows,
        map(len, headers)
    )

    # helpers to fill with spaces each column
    # and to render a row in markdown format
    space_fill = lambda f, w: map(
        lambda p: fill(p[0], ' ', (p[1] + 1 - len(p[0]))),
        zip(f, w)
    )
    markdown_row = lambda f, w: "|{}|".format(
        "|".join(space_fill(f, w))
    )

    # render table
    headers = markdown_row(headers, widths)
    separator = markdown_row(
        map(lambda w: fill(':', '-', w-1), widths),
        widths
    )
    rows = "\n".join(
        map(lambda f: markdown_row(f, widths), rows)
    )

    return "\n".join([headers, separator, rows])


def main():
    print("Running on {}".format(WorkingCopy('.')))

    envs = [
        C(),
        CPPADTClang(),
        CPPJITClang(),
        CPPOOPClang(),
        CPPADTGCC(),
        CPPJITGCC(),
        CPPOOPGCC(),
        Rust(),
        RustJIT(),
        Go(),
        PyPy(),
    ]
    tests = [
        Hello(),
        Primes(200),
        Mandelbrot(),
    ]

    headers = [''] + [t.desc for t in tests]
    rows = []

    for env in envs:
        results = run(tests, env)
        rows.append([env.desc] + results)

    print(markdown_table(headers, rows, align=1))


if __name__ == '__main__':
    main()
