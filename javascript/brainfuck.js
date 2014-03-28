var ui = new InterpreterUI(); 

$(document).ready(function () {
    $("#btn-start").click(function (event) {
        event.preventDefault();
        ui.start();
    });
    $("#btn-pause").click(function (event) {
        event.preventDefault();
        ui.pause();
    });
    $("#btn-step").click(function (event) {
        event.preventDefault();
        ui.step();
    });
    $("#btn-stop").click(function (event) {
        event.preventDefault();
        ui.stop();
    });
    $("#btn-optimize").click(function (event) {
        event.preventDefault();
        $('#program').val(optimize($('#program').val()));
    });
    resizePanels();
})

$(window).resize(function (event) {
    resizePanels();
});

function resizePanels() {
    var height = $(window).height() + 30;

    $('#program').css('height', 0);
    $('#input').css('height', 0);
    $('#debug').css('height', 0);
    $('#output').css('height', 0);

    height -= ($('#button-bar').height() + 
               $('#panels-body').height());

    $('#program').css('height', height * .3);
    $('#input').css('height', height * .3);
    $('#debug').css('height', height * .7);
    $('#output').css('height', height * .7);
}

function optimize(program) {
    /* remove invalid ops: */
    var valid_op = function(op) {
        return '-+<>[],.'.indexOf(op) > -1;
    }

    program = program.split('').filter(valid_op).join('');

    return program;
}

function renderMemory(memory, current, limit) {
    var ret = '';

    function zeropad(string, length) {
        while(string.length < length) {
            string = '0' + string;
        }

        return string;
    }

    for (var row = 0; (row * 8) < limit; ++row) {
        ret += zeropad((row * 8).toString(16), 6) + ' ';
        for (var column = 0; 
             column < 8 && (row * 8 + column) < limit; 
             ++column) {
            var index = row * 8 + column,
                prefix = index == current ? '*' : ' ';
            ret += prefix + zeropad(memory[index].toString(16), 2) + ' ';
        }
        ret += '\n';
    }

    return ret;
}

function InterpreterUI() {
    this.start_date = undefined;
    this.cycles = 0;
    this.interpreter = undefined;
    this.state = new UIStopped();
}

InterpreterUI.prototype.start = function() {
    this.state = this.state.start();
}

InterpreterUI.prototype.pause = function() {
    this.state = this.state.pause();
}

InterpreterUI.prototype.step = function() {
    this.state = this.state.step();
}

InterpreterUI.prototype.stop = function() {
    this.state = this.state.stop();
}

InterpreterUI.prototype.onStart = function() {
    var self = this;

    this.start_date = Date.now();
    this.cycles = 0;

    $('#output').val('');

    $('#cycles-count').html(this.cycles);
    $('#running-time').html("0.00 seconds");

    this.interpreter = new Interpreter(
        $('#program').val(), 
        $("#input").val(), 
        function() {
            self.onTick()
        },
        function(err) {
            self.onFinish(err);
        }
    );
}

InterpreterUI.prototype.onTick = function () {
    this.cycles += 1;
    delta = (Date.now() - this.start_date) / 1000;

    $('#program').get(0).setSelectionRange(this.interpreter.pc, 
                                           this.interpreter.pc+1);

    $('#debug').html(renderMemory(this.interpreter.memory, 
                                  this.interpreter.mem_ptr, 
                                  512));
    $('#output').val(this.interpreter.output);
    $('#program-counter').html(this.interpreter.pc);
    $('#memory-ptr').html(this.interpreter.mem_ptr);
    $('#input-ptr').html(this.interpreter.input_ptr);
    $('#cycles-count').html(this.cycles);
    $('#running-time').html(delta.toFixed(2) + " seconds");
}

InterpreterUI.prototype.onFinish = function (err) {
    $('#btn-start').removeClass("disabled");
    $('#btn-start-label').html("Start");
    $('#btn-pause').addClass("disabled");
    $('#btn-step').removeClass("disabled");
    $('#btn-stop').addClass("disabled");

    this.state = new UIStopped();
}

function UIStopped() {}

UIStopped.prototype.start = function() {
    ui.onStart();
    ui.interpreter.start(parseInt($('#inst-per-cycle').val()));

    $('#btn-start').addClass("disabled");
    $('#btn-pause').removeClass("disabled");
    $('#btn-step').addClass("disabled");
    $('#btn-stop').removeClass("disabled");

    return new UIRunning();
}

UIStopped.prototype.pause = function() {
    throw "Invalid transition";
}

UIStopped.prototype.step = function() {
    ui.onStart();
    ui.interpreter.step();

    $('#btn-start').removeClass("disabled");
    $('#btn-start-label').html("Resume");    
    $('#btn-pause').addClass("disabled");
    $('#btn-step').removeClass("disabled");
    $('#btn-stop').removeClass("disabled");

    return new UIPaused();
}

UIStopped.prototype.stop = function() {
    throw "Invalid transition";
}

function UIRunning() {}

UIRunning.prototype.start = function() {
    throw "Invalid transition";
}

UIRunning.prototype.pause = function() {
    ui.interpreter.pause();
    
    $('#btn-start').removeClass("disabled");
    $('#btn-start-label').html("Resume");
    $('#btn-pause').addClass("disabled");
    $('#btn-step').removeClass("disabled");
    $('#btn-stop').removeClass("disabled");

    return new UIPaused();
}

UIRunning.prototype.step = function() {
    throw "Invalid transition";
}

UIRunning.prototype.stop = function() {
    ui.interpreter.stop();
    return new UIStopped();
}

function UIPaused() {}

UIPaused.prototype.start = function () {
    ui.interpreter.start(parseInt($('#inst-per-cycle').val()));

    $('#btn-start').addClass("disabled");
    $('#btn-pause').removeClass("disabled");
    $('#btn-step').addClass("disabled");
    $('#btn-stop').removeClass("disabled");

    return new UIRunning();
}

UIPaused.prototype.pause = function () {
    throw "Invalid transition";
}

UIPaused.prototype.step = function () {
    ui.interpreter.step();
    return this;
}

UIPaused.prototype.stop = function () {
    ui.interpreter.stop();
    return new UIStopped();
}

////////////
function Interpreter(program, input, onTick, onFinish) {
    this.program = program;
    this.input = input;
    this.output = '';

    this.onTick = onTick;
    this.onFinish = onFinish;

    this.stopRequested = false;
    this.intervalId = undefined;
    this.jumps = {};

    this.memory = {0: 0};
    this.mem_ptr = 0;
    this.input_ptr = 0;
    this.pc = 0;

    this.state = new Stopped(this);

    this.init();
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
        this.memory[i] = 0;
    }    
}

Interpreter.prototype.runCycle = function(instPerCycle) {
    try {
        for(var i = 0; 
            i < instPerCycle && this.pc < this.program.length; 
            ++i) {
            var opcode = this.program[this.pc];
            switch(opcode) {
                case '>':
                    ++this.mem_ptr;
                    break;
                case '<':
                    --this.mem_ptr;
                    break;
                case '+':
                    this.memory[this.mem_ptr]++;
                    break;
                case '-':
                    this.memory[this.mem_ptr]--;
                    break;
                case '.':
                    this.output += String.fromCharCode(this.memory[this.mem_ptr]);
                    break;
                case ',':
                    if (this.input_ptr < this.input.length) {
                        this.memory[this.mem_ptr] = 
                            this.input.charCodeAt(this.input_ptr++);
                    } else {
                        this.tick();
                        throw "EOF";
                    }
                    break;
                case '[':
                    if (this.memory[this.mem_ptr] == 0) { 
                        this.pc = this.jumps[this.pc];
                    }
                    break;
                case ']':
                    if (this.memory[this.mem_ptr] != 0) {
                        this.pc = this.jumps[this.pc];
                    }
                    break;
                default:
                    break;
            }
            ++this.pc;
        }

        this.tick();

        if (this.pc == this.program.length) {
            throw "EOP";
        }
    } catch(e) {
        console.log("Received: ", e);
        this.finish(e);
    }
}

Interpreter.prototype.start = function (instPerCycle) {
    this.state = this.state.start(instPerCycle);
}

Interpreter.prototype.pause = function () {
    this.state = this.state.pause();
}

Interpreter.prototype.step = function () {
    this.state = this.state.step();
}

Interpreter.prototype.stop = function () {
    this.state = this.state.stop();
}

Interpreter.prototype.tick = function () {
    if (this.stopRequested) {
        throw "STOP REQUESTED";
    }

    this.onTick(this);
}

Interpreter.prototype.finish = function (e) {
    clearInterval(this.intervalId);
    this.state = new Stopped(this);
    this.onFinish(e, this);
}

////
function Stopped(interpreter) {
    this.interpreter = interpreter;
}

Stopped.prototype.start = function (instPerCycle) {
    var self = this;

    this.interpreter.intervalId = setInterval(function () {
        self.interpreter.runCycle(instPerCycle);
    }, 0);

    return new Running(this.interpreter);
}

Stopped.prototype.pause = function () {
    throw "Invalid transition";
}

Stopped.prototype.step = function () {
    this.interpreter.runCycle(1);
    return new Paused(this.interpreter);
}

Stopped.prototype.stop = function () {
    throw "Invalid transition";
}

////
function Running(interpreter) {
    this.interpreter = interpreter;
}

Running.prototype.start = function () {
    throw "Invalid transition";
}

Running.prototype.pause = function () {
    clearInterval(this.interpreter.intervalId);
    return new Paused(this.interpreter);
}

Running.prototype.step = function () {
    throw "Invalid transition";
}

Running.prototype.stop = function () {
    this.interpreter.stopRequested = true;
    return new Stopped(this.interpreter);
}

////
function Paused(interpreter) {
    this.interpreter = interpreter;
}

Paused.prototype.start = function (instPerCycle) {
    var self = this;

    this.interpreter.intervalId = setInterval(function () {
        self.interpreter.runCycle(instPerCycle);
    }, 0);

    return new Running(this.interpreter);    
}

Paused.prototype.paused = function () {
    throw "Invalid transition";
}

Paused.prototype.step = function () {
    this.interpreter.runCycle(1);
    return this;
}

Paused.prototype.stop = function () {
    this.interpreter.finish();
    return new Stopped(this.interpreter);
}
