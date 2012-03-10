ALL = brainfuck bf2c

all: $(ALL)

brainfuck: brainfuck.c
	$(CC) $< -o $@

bf2c: bf2c.hs
	ghc --make $@ -o $@

define translate
	./bf2c < $1 | indent -kr > $2
endef

%.c: %.b bf2c 
	$(call translate,$<,$@)

%.c: %.bf bf2c 
	$(call translate,$<,$@)

%: %.c
	$(CC) -O3 $< -o $@

clean:
	rm -rf $(ALL) *.hi *.o

