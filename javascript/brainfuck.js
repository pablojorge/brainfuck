$(document).ready(function () {
    // $("#btn-run").click(onRun);
    // $("#btn-optimize").click(onOptimize);
    // $("#btn-translate").click(onTranslate);
    // $("#btn-interpret").click(onInterpret);
    $("#btn-start").click(onStart);
    $("#btn-stop").click(onStop);
})

// function onOptimize(event) {
//     event.preventDefault();

//     $('#optimized').val(optimize($('#program').val()));
// }

// function onTranslate(event) {
//     event.preventDefault();

//     $('#translated').val(translate($('#optimized').val()).join('\n'));
// }

// function onRun(event) {
//     event.preventDefault();

//     var output = [];
//     var start = Date.now(), end;

//     $('#output').val('');

//     run($('#translated').val(), 
//         $("#input").val(), 
//         function (char) {
//             output.push(char);
//         });

//     end = Date.now();

//     $('#output').val(output.join(''));

//     alert("Ran in " + ((end - start)/1000) + " seconds");
// }

// function onInterpret(event) {
//     event.preventDefault();

//     var start = Date.now(), end;

//     $('#output').val('');

//     interpret($('#program').val(), 
//               $("#input").val(), 
//               function (char) {
//                   $('#output').val($('#output').val() + char);
//               },
//               function () {
//                   start = Date.now();
//               },
//               function () {
//                   end = Date.now();
//                   alert("Ran in " + ((end-start)/1000) + " seconds");
//               });
// }

var stop_requested = false;

function onStart(event) {
    event.preventDefault();

    var start = Date.now(),
        cycles = 0,
        output = '';

    interpret(
        $('#program').val(), 
        $("#input").val(), 
        parseInt($('#inst-per-cycle').val()),
        function (char) {
            output += char;
        },
        function () {
            start = Date.now();
            stop_requested = false;
            $('#output').val('');
            $('#cycles-count').html(cycles);
            $('#running-time').html("0.00 seconds");
            $('#btn-start').addClass("disabled");
            $('#btn-stop').removeClass("disabled");
        },
        function () {
            cycles += 1;
            delta = (Date.now() - start) / 1000;

            $('#output').val(output);
            $('#cycles-count').html(cycles);
            $('#running-time').html(delta.toFixed(2) + " seconds");

            if (stop_requested) {
                // this will force the interpreter to abort:
                throw "STOP REQUESTED";
            }
        },
        function () {
            $('#btn-start').removeClass("disabled");
            $('#btn-stop').addClass("disabled");
        }
    );
}

function onStop(event) {
    event.preventDefault();

    stop_requested = true;
}

function optimize(program) {
    /* remove invalid ops: */
    var valid_op = function(op) {
        return '-+<>[],.'.indexOf(op) > -1;
    }

    program = program.split('').filter(valid_op).join('');

    return program;
}

// function translate(program) {
//     var prologue = [
//         'var memory = {};',
//         'var ptr = 0;',
//         'for(var i = 0; i < 30000; ++i) {',
//         '   memory[i] = 0;',
//         '}',
//         'try {',
//     ];
//     var body = [];
//     var epilogue = [
//         '} catch(e) {',
//         '} finally {',
//         '}',
//     ];
//     var opcodes = {
//         '>' : '++ptr;',
//         '<' : '--ptr;',
//         '+' : 'memory[ptr]++;',
//         '-' : 'memory[ptr]--;',
//         '.' : 'putchar(memory[ptr]);',
//         ',' : 'memory[ptr] = getchar();',
//         '[' : 'while(memory[ptr] > 0) {',
//         ']' : '}',
//     }

//     program.split('').forEach(function (char) {
//         if (char in opcodes) {
//             body.push(opcodes[char]);
//         }
//     })

//     return prologue.concat(body).concat(epilogue);
// }

// function run(program, input, output_cb) {
//     fun = new Function("getchar", "putchar", program);

//     var in_idx = 0;

//     getchar = function() {
//         if (in_idx < input.length) {
//             return input.charCodeAt(in_idx++);
//         } else {
//             throw "EOF";
//         }
//     }

//     putchar = function(charCode) {
//         output_cb(String.fromCharCode(charCode));
//     }

//     fun(getchar, putchar);
// }

function interpret(program, input, inst_per_cycle, 
                   onOutput, onStart, onTick, onFinish) {
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
        try {
            for(var i = 0; i < inst_per_cycle && pc < program.length; i++) {
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
                        onOutput(String.fromCharCode(memory[ptr]));
                        break;
                    case ',':
                        if (in_ptr < input.length) {
                            memory[ptr] = input.charCodeAt(in_ptr++);
                        } else {
                            onTick();
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

            onTick();

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