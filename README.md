# tbfc
tbfc is a tiny (about 270 lines) brainfuck compiler which generates 32-bit assembly.

The compiler performs some optimizations, namely it reduces increments and decrements (both of the tape pointer and of the tape cell) into a single operation.

The generated code is ready to be compiled with `nasm`, linked with `ld` and ran on (probably) any x86 Linux system.
Making the compiler target different operating systems is as simple as changing the generated prologue and epilogue.

## Building

To compile, you need a C++17 compiler, and make.

To compile with G++, simply do:
```
$ make
```

If you wish to compile with a different compiler, just set the CXX environment variable, like so:
```
$ CXX=clang++ make # replace clang++ with your compiler of choice
```

## Usage
Using tbfc is simple, it prints nothing to stdout, and the generated code to stderr, so to save the code to a file do the following:
```
$ tbfc input 2>output
```

Then, to compile the code, `nasm` is recommended (the code is generated with it in mind). To do so, do the following:
```
$ nasm -felf32 <input>.asm <output>.o
```

After that, you need to link the code into the final executable, like so:
```
$ ld -melf_i386 <input>.o -o <output> -nostdlib
```

After all that is done, you should have a working binary.

## Compliance
The generated code uses a 30000 cell tape, where each cell is 8-bit. On overflow/underflow of the tape pointer, it wraps around to the end/beginning of the tape. On overflow/underflow of the value in the cell, the value also wraps around.

The compiler appears to compile most (everything I tested) programs fine, including [The Lost Kindgom](https://jonripley.com/i-fiction/games/LostKingdomBF.html).

Included are some examples and a makefile to compile and link them.

## Speed
Generating the assembly is rather fast (doing so for The Lost Kingdom took about 2 seconds on my machine).

The generated code is also pretty fast.

Further improvements could be made by better optimizing the IR and generated assembly (eliminating loops, replacing tape pointer movement with static offsets, etc.), but that will be done at a later time, if at all.
