## Laado Compiler Collection

Welcome to the Laado compiler collection. This is a collection of various experimental programming languages meant to explore and solve specific problems.

However, this is not just a collection of languages. This is meant to be a complete compiler collection. Many of the languages use LLVM for the backend, but we also have in-development sub-projects meant to be replacements for various external dependencies.

Here is our current project list:
* as -> Assemblers for various architectures (current x86 and RISC-V).
* compiler -> A library for generalizing our frontends to various backends
* libelf -> Utilities for the ELF binary file format.
* libjava -> A library for generating Java class files (no external libraries needed)
* test -> The test system
* riya-lang -> The first language, a mini educational language
* orka-lang-> A language based on Riya, but with added features such as for loops, for-all loops, floating-point operations, and object oriented programming
* lex -> A simple lexical analyzer generator
* ast_interpreter -> A source level interpreter that can run the included AST

### Licensing

This project is not licensed and placed in the public domain. Please see my website for more information on this decision.

