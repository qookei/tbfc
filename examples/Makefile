EXAMPLES = mandlebrot sierpinski hello rot13 hanoi

LD ?= ld
TBFC ?= ../tbfc
NASM ?= nasm

all: $(EXAMPLES)

%: %.o
	$(LD) -melf_i386 -o $@ $< -nostdlib

%.o: %.asm
	$(NASM) -felf32 -o $@ $<

%.asm: %.bf
	$(TBFC) $< 2>$@

.PHONY: clean

clean:
	rm $(EXAMPLES)
