$(document).ready(function () {
    $("#btn-start").click(onStart);
    $("#btn-stop").click(onStop);
})

var interpreter = undefined;

function onStart(event) {
    event.preventDefault();

    var start = Date.now(),
        cycles = 0,
        output = '';

    interpreter = new Interpreter(
        $('#program').val(), 
        $("#input").val(), 
        function () {
            start = Date.now();
            $('#output').val('');
            $('#cycles-count').html(cycles);
            $('#running-time').html("0.00 seconds");
            $('#btn-start').addClass("disabled");
            $('#btn-stop').removeClass("disabled");
        },
        function (self) {
            cycles += 1;
            delta = (Date.now() - start) / 1000;

            // $('#input').get(0).setSelectionRange(in_ptr, in_ptr+1);
            // $('#program').get(0).setSelectionRange(pc, pc+1);

            $('#output').val(self.output);
            $('#cycles-count').html(cycles);
            $('#running-time').html(delta.toFixed(2) + " seconds");
        },
        function (err, self) {
            $('#btn-start').removeClass("disabled");
            $('#btn-stop').addClass("disabled");
        }
    );

    interpreter.init();
    interpreter.start(parseInt($('#inst-per-cycle').val()));
}

function onStop(event) {
    event.preventDefault();
    interpreter.stop();
}

function Interpreter(program, input, onStart, onTick, onFinish) {
    this.program = program;
    this.input = input;
    this.output = '';

    this.onStart = onStart;
    this.onTick = onTick;
    this.onFinish = onFinish;

    this.stopRequested = false;
    this.intervalId = undefined;
    this.jumps = {};
    this.state = {
        memory: {0: 0},
        ptr: 0,
        in_ptr: 0,
        pc: 0,
    };
}

Interpreter.prototype.init = function(mem_size) {
    /* precompute jumps: */
    for(var pc = 0, stack = []; pc < this.program.length; ++pc) {
        var opcode = this.program[pc];

        if (opcode == '[') {
            stack.push(pc);
        } else if (opcode == ']') {
            var target = stack.pop();
            this.jumps[target] = pc;
            this.jumps[pc] = target;
        }
    }

    /* preload memory: */
    for(var i = 0; i < (mem_size || 30000); ++i) {
        this.state.memory[i] = 0;
    }    
}

Interpreter.prototype.runCycle = function(instPerCycle) {
    try {
        for(var i = 0; 
            i < instPerCycle && this.state.pc < this.program.length; 
            ++i) {
            var opcode = this.program[this.state.pc];
            switch(opcode) {
                case '>':
                    ++this.state.ptr;
                    break;
                case '<':
                    --this.state.ptr;
                    break;
                case '+':
                    this.state.memory[this.state.ptr]++;
                    break;
                case '-':
                    this.state.memory[this.state.ptr]--;
                    break;
                case '.':
                    this.output += String.fromCharCode(this.state.memory[this.state.ptr]);
                    break;
                case ',':
                    if (this.state.in_ptr < this.input.length) {
                        this.state.memory[this.state.ptr] = 
                            this.input.charCodeAt(this.state.in_ptr++);
                    } else {
                        this.tick();
                        throw "EOF";
                    }
                    break;
                case '[':
                    if (this.state.memory[this.state.ptr] == 0) { 
                        this.state.pc = this.jumps[this.state.pc];
                    }
                    break;
                case ']':
                    if (this.state.memory[this.state.ptr] != 0) {
                        this.state.pc = this.jumps[this.state.pc];
                    }
                    break;
                default:
                    break;
            }
            ++this.state.pc;
        }

        this.tick();

        if (this.state.pc == this.program.length) {
            throw "EOP";
        }
    } catch(e) {
        console.log("Received: ", e);
        this.finish(e);
    }
}

Interpreter.prototype.start = function (instPerCycle) {
    var self = this;

    this.onStart();

    this.intervalId = setInterval(function () {
        self.runCycle(instPerCycle)
    }, 0);
}

Interpreter.prototype.pause = function () {
    clearInterval(this.intervalId);    
}

Interpreter.prototype.stop = function () {
    this.stopRequested = true;
}

Interpreter.prototype.tick = function () {
    if (this.stopRequested) {
        throw "STOP REQUESTED";
    }

    this.onTick(this);
}

Interpreter.prototype.finish = function (e) {
    clearInterval(this.intervalId);    
    this.onFinish(e, this);
}