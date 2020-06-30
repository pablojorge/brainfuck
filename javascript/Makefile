WASM_ROOT=../wasm
WASM_PKG=$(WASM_ROOT)/pkg

.PHONY: build
build:
	$(MAKE) -C $(WASM_ROOT) build
	cp $(WASM_PKG)/wasm_brainfuck.js $(WASM_PKG)/wasm_brainfuck_bg.wasm ./pkg

.PHONY: serve
serve: build
	echo "Connect to http://localhost:8000"
	python -m SimpleHTTPServer