Time/Project management
=======================

Start of project: 24th Feb 2021 (Week 1)

Key modules: Lexer, Parser, Code Generator, Makefile, test script

Week 1: Finish the basic requirements

Week 2: Finish intermediate requirements

Week 3: Test for intermediates funcitionality and design the advanced functionality

Week 4: Finish intermediate requirements

Our internal deadline of project: 22nd March 2021 (Week 5) to give buffer time in case any issues arises

Key responsibilities
======================
Joshua:
-   Building the framework of the compiler

Chern Heng
-   Testing and helping with the building and testing of different aspects of the compiler

Tasks:
======================

1. Build a lexer and parser that can parse a basic function with no arguments, that does variable assignments and arithmetic expressions
    - As of 12am 27th Feb 2021, the progress is still building up the general components of the compiler
    - As of 3am 27th Feb 2021, project milestone, AST works for the basic requirements

2. Build the code generation for MIPS
    - As of 12am 1st Mar 2021, AST expressions of arithmetic and logical operations was tested and ensured that it followed precedence. Building of code generation is underway.
    - As of 3am 4th Mar 2021, the code generation for variable declarations, functions with no arguments, assignment, and operators are completed and tested. If else conditionals and while loops were tested, together with break. Next things to do are to do code generation for for loops and continue keyword, and beginning intermediate requirements, which is to handle functions with multiple arguments.

3. Continually add to the framework of lexer and parser and code generation so that it is be able to handle intermediate requirements