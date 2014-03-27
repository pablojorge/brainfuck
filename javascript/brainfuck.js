function onOptimize() {
    $('#optimized').val(optimize($('#program').val()));
}

function onTranslate() {
    $('#translated').val(translate($('#optimized').val()).join('\n'));
}

function onRun() {
    var output = [];
    var start = Date.now(), end;

    $('#output').val('');

    run($('#translated').val(), 
        $("#input").val(), 
        function (char) {
            output.push(char);
        });

    end = Date.now();

    $('#output').val(output.join(''));

    alert("Ran in " + ((end - start)/1000) + " seconds");
}

function onInterpret() {
    var start = Date.now(), end;

    $('#output').val('');

    interpret($('#program').val(), 
              $("#input").val(), 
              function (char) {
                  $('#output').val($('#output').val() + char);
              },
              function () {
                  start = Date.now();
              },
              function () {
                  end = Date.now();
                  alert("Ran in " + ((end-start)/1000) + " seconds");
              });
}

function optimize(program) {
    /* remove invalid ops: */
    var valid_op = function(op) {
        return '-+<>[],.'.indexOf(op) > -1;
    }

    program = program.split('').filter(valid_op).join('');

    return program;
}

function translate(program) {
    var prologue = [
        'var memory = {};',
        'var ptr = 0;',
        'for(var i = 0; i < 30000; ++i) {',
        '   memory[i] = 0;',
        '}',
        'try {',
    ];
    var body = [];
    var epilogue = [
        '} catch(e) {',
        '} finally {',
        '}',
    ];
    var opcodes = {
        '>' : '++ptr;',
        '<' : '--ptr;',
        '+' : 'memory[ptr]++;',
        '-' : 'memory[ptr]--;',
        '.' : 'putchar(memory[ptr]);',
        ',' : 'memory[ptr] = getchar();',
        '[' : 'while(memory[ptr] > 0) {',
        ']' : '}',
    }

    program.split('').forEach(function (char) {
        if (char in opcodes) {
            body.push(opcodes[char]);
        }
    })

    return prologue.concat(body).concat(epilogue);
}

function run(program, input, output_cb) {
    fun = new Function("getchar", "putchar", program);

    var in_idx = 0;

    getchar = function() {
        if (in_idx < input.length) {
            return input.charCodeAt(in_idx++);
        } else {
            throw "EOF";
        }
    }

    putchar = function(charCode) {
        output_cb(String.fromCharCode(charCode));
    }

    fun(getchar, putchar);
}

function interpret(program, input, output_cb, onStart, onFinish) {
    program = optimize(program);

    /* precompute jumps: */
    var jumps = {};
    for(var pc = 0, stack = []; pc < program.length; ++pc) {
        var opcode = program[pc];

        if (opcode == '[') {
            stack.push(pc);
        } else if (opcode == ']') {
            var target = stack.pop();
            jumps[target] = pc;
            jumps[pc] = target;
        }
    }

    /* main interpreter loop: */
    var memory = {0: 0},
        ptr = 0,
        in_ptr = 0,
        pc = 0;
    
    for(var i = 0; i < 30000; ++i) {
        memory[i] = 0;
    }

    intervalId = setInterval(function() {
        INST_PER_CYCLE = 100000;

        try {
            for(var i = 0; i < INST_PER_CYCLE && pc < program.length; i++) {
                var opcode = program[pc];
                switch(opcode) {
                    case '>':
                        ++ptr;
                        break;
                    case '<':
                        --ptr;
                        break;
                    case '+':
                        memory[ptr]++;
                        break;
                    case '-':
                        memory[ptr]--;
                        break;
                    case '.':
                        output_cb(String.fromCharCode(memory[ptr]));
                        break;
                    case ',':
                        if (in_ptr < input.length) {
                            memory[ptr] = input.charCodeAt(in_ptr++);
                        } else {
                            throw "EOF";
                        }
                        break;
                    case '[':
                        if (memory[ptr] == 0) { 
                            pc = jumps[pc];
                        }
                        break;
                    case ']':
                        if (memory[ptr] != 0) {
                            pc = jumps[pc];
                        }
                        break;
                    default:
                        throw ("unexpected opcode: " + opcode);
                }
                ++pc;
            }

            if (pc == program.length) {
                throw "EOP";
            }
        } catch(e) {
            console.log("Received: ", e);
            onFinish();
            clearInterval(intervalId);
        }
    }, 0);

    onStart();
}