#!/bin/bash

# Try to find a portable way of getting rid of
# any stray carriage returns
if which dos2unix ; then
    DOS2UNIX="dos2unix"
elif which fromdos ; then
    DOS2UNIX="fromdos"
else
    # This works on a GNU version of sed. I think this
    # will work in OSX as well, but don't have a machine
    # on which to test that. From reading the OSX docs,
    # it looks compatible.
    # The code \x0D is the ASCII code of carriage-return,
    
    # so it the regex should delete any CRs at the end of
    # a line (or anywhere in a line)
    DOS2UNIX="cat"
    # Tested for combinations of:
    # - Ubuntu
    # - Cygwin
    # and inputs formats:
    # - LF
    # - CRLF
fi

if [ $1 -eq 1 ] ; then
    echo "========================================"
    bin/c_compiler -S dev/test.c -o dev/output.s

    echo "========================================"
    echo "Compiling with driver program"
    mips-linux-gnu-gcc -mfp32 -o dev/test_program.o -c dev/output.s
    mips-linux-gnu-gcc -mfp32 -static -o dev/test_program dev/test_program.o dev/test_driver.c
    qemu-mips dev/test_program
    echo "Exit code: $?"
    exit 0;
fi

if [ $1 -eq 3 ] ; then
    echo "========================================"
    echo "Compiling with driver program"
    mips-linux-gnu-gcc -mfp32 -o dev/test_program.o -c dev/output.s
    mips-linux-gnu-gcc -mfp32 -static -o dev/test_program dev/test_program.o dev/test_driver.c
    qemu-mips dev/test_program
    echo "Exit code: $?"
    exit 0;
fi


echo "========================================"
echo " Cleaning the temporaries and outputs"
make clean
echo " Building bin/c_compiler"
make -B bin/c_compiler
if [[ "$?" -ne "0" ]]; then
    echo "Error while building compiler."
    exit 1;
fi

if [ $1 -eq 2 ] ; then

    echo "========================================"
    echo "Compiling tests"
    FILES=$(ls -1 compiler_tests/$2 | grep -v driver | cut -d "." -f 1)
    for i in $FILES; do
        bin/c_compiler -S compiler_tests/$2/${i}.c -o temp/${i}.s
        mips-linux-gnu-gcc -mfp32 -o temp/${i}.o -c temp/${i}.s
        mips-linux-gnu-gcc -mfp32 -static -o temp/${i} temp/${i}.o compiler_tests/$2/${i}_driver.c
        qemu-mips temp/${i}
        echo "Exit code: $?"
    done
    exit 0;
fi



echo "========================================"

bin/c_compiler -S dev/test.c -o dev/output.s

echo "========================================"
echo "Compiling with driver program"
mips-linux-gnu-gcc -mfp32 -o dev/test_program.o -c dev/output.s
mips-linux-gnu-gcc -mfp32 -static -o dev/test_program dev/test_program.o dev/test_driver.c
qemu-mips dev/test_program
echo "Exit code: $?"