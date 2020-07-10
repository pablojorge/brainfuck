.PHONY: build
build:
	wasm-pack build --target web

.PHONY: serve
serve: build
	echo "Connect to http://localhost:8000"
	python -m SimpleHTTPServer
