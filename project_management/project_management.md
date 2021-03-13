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

Meet up every Friday to share the progress of the week

Tasks:
======================

1. Build a lexer and parser that can parse a basic function with no arguments, that does variable assignments and arithmetic expressions
    - As of 12am 27th Feb 2021, the progress is still building up the general components of the compiler
    - As of 3am 27th Feb 2021, project milestone, AST works for the basic requirements

2. Build the code generation for MIPS
    - As of 12am 1st Mar 2021, AST expressions of arithmetic and logical operations was tested and ensured that it followed precedence. Building of code generation is underway.
    - As of 3am 4th Mar 2021, the code generation for variable declarations, functions with no arguments, assignment, and operators are completed and tested. If else conditionals and while loops were tested, together with break. Next things to do are to do code generation for for loops and continue keyword, and beginning intermediate requirements, which is to handle functions with multiple arguments.

3. Continually add to the framework of lexer and parser and code generation so that it is be able to handle intermediate requirements

Basic Features:
======================
-   a file containing just a single function with no arguments (DONE) (4th Mar)
-   variables of int type (DONE) (4th Mar)
-   local variables (DONE) (4th Mar)
arithmetic and logical expressions (DONE) (4th Mar)
if-then-else statements (DONE) (4th Mar)
while loops (DONE) (4th Mar)

Intermediate features:
======================
    files containing multiple functions that call each other (Test)
~~functions that take up to four parameters (DONE) (8th Mar)~~
for loops (DONE) (6th Mar)
    arrays declared globally (i.e. outside of any function in your file)
    arrays declared locally (i.e. inside a function)
    reading and writing elements of an array
recursive function calls
    the enum keyword
switch statements (DONE) (12th Mar)
the break and continue keywords (DONE) (6th Mar)

Advanced features:
======================
variables of double, float, char, unsigned, structs, and pointer types
calling externally-defined functions (i.e. the file being compiled declares a function, but its definition is provided in a different file that is linked in later on)
functions that take more than four parameters (DONE) (10th Mar)
mutually recursive function calls
locally scoped variable declarations (e.g. a variable that is declared inside the body of a while loop, such as while(...) { int x = ...; ... } (Test) DONE (4th Mar)
    the typedef keyword
    the sizeof(...) function (which takes either a type or a variable)
taking the address of a variable using the & operator (DONE) (5th Mar)
dereferencing a pointer-variable using the * operator (DONE) (5th Mar)
pointer arithmetic (DONE) (5th Mar)
    character literals, including escape sequences like \n
strings (as NULL-terminated character arrays)
    declaration and use of structs

Functions more than 4 args, call multiple functions cascade

