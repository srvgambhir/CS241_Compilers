# CS241_Compilers

As a course long project in CS 241, we built a compiler for the WLP4 Programming languages. WLP4 is a simplied version of C++, with each program consisting of a series of C++ functions, the last of which is the main function (this main function is called wain).

Starting from a .wlp4 source file the complete sequence of commands to generate MIPS machine language and run it is:

- wlp4scan < foo.wlp4 > foo.scanned
- wlp4parse < foo.scanned > foo.wlp4i
- ./wlp4gen < foo.wlp4i > foo.asm
- cs241.binasm < foo.asm > foo.mips
- mips.twoints foo.mips OR mips.array foo.mips

The following components of the WLP4 compiler translates WLP4 source code into MIPS assembly language:


