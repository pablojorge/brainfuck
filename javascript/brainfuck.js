function onOptimize() {
	$('#optimized').val(optimize($('#program').val()));
}

function onTranslate() {
	$('#translated').val(translate($('#optimized').val()).join('\n'));
}

function onRun() {
	$('#output').val('');

	run($('#translated').val(), 
		$("#input").val(), 
		function (char) {
	  		$('#output').val($('#output').val() + char);
		});
}

function onInterpret() {
	$('#output').val('');

	interpret($('#program').val(), 
			  $("#input").val(), 
			  function (char) {
		  		  $('#output').val($('#output').val() + char);
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
		'	memory[i] = 0;',
		'}',
		'try {',
	];
	var body = [];
	var epilogue = [
		'} catch(e) {',
		'} finally {',
		'}',
	];
	var	opcodes = {
		'>' : '++ptr;',
		'<' : '--ptr;',
		'+' : 'memory[ptr]++;',
		'-' : 'memory[ptr]--;',
		'.' : 'putchar(memory[ptr]);',
		',' : 'memory[ptr] = getchar();',
		'[' : 'while(memory[ptr] > 0) {',
		']' : '}',
	}

	for(var i = 0; i < program.length; ++i) {
		char = program.charAt(i);
		if (char in opcodes) {
			body.push(opcodes[char]);
		}
	}

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

function interpret(program, input, output_cb) {
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

	setInterval(function() {
		INST_PER_CYCLE = 100000;

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
						return;
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
	}, 0);
}