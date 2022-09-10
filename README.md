# CS241_Compilers

As a course long project in CS 241, we built a compiler for the WLP4 Programming languages. WLP4 is a simplied version of C, with each program consisting of a series of C functions, the last of which is the main function (this main function is called wain).

Starting from a .wlp4 source file the complete sequence of commands to generate MIPS machine language and run it is:

- wlp4scan < foo.wlp4 > foo.scanned
- wlp4parse < foo.scanned > foo.wlp4i
- wlp4gen < foo.wlp4i > foo.asm
- binasm < foo.asm > foo.mips
- mips.twoints foo.mips OR mips.array foo.mips

The following components of the WLP4 compiler translate WLP4 source code into MIPS assembly language:

- wlp4scan - This Simplified Maximal Munch scanner reads a WLP4 program from standard input and tokenizes it; this is in the folder a5/A5P1
- wlp4parse - This is a parser for WLP4 that takes in the tokenized output from wlp4scan and produces a parse tree. The left-to-right preorder traversal of this tree is stored in a .wlp4i (WLP4 Intermediate) file. This parser is implemented with the LR(1) parsing algorithm using an SLR(1) DFA. The parser is in the folder a6/A6P5
- wlp4gen - This program completes the translation of WLP4 source code. It is a MIPS assembly code generator that takes as input a .wlp4i file. It also serves as the context sensitive analyzer (semantic analysis) of the compiler. The completed generator is in the folder a9

We now have WLP4 source code translated into MIPS assembly language. Finally, this MIPS assembly is translated into MIPS machine language. This is done using a MIPS assembler (binasm in the sequence of commands above). This assembler is in the folder a3 (the asm.cc file)

The final output file can be ran using mips.twoints or mips.array, which are emulators for MIPS machine language.
