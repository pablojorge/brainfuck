ALL = main

all: $(ALL)

test: tests
	./$<

%: %.rs
	rustc $<

clean:
	rm -rf $(ALL) tests
