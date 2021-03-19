#!/bin/bash

if [ $# -lt 1 ]; then
    echo "specify test folder"
    exit 1
fi

FOLDER_NAME="$1"
if [ -z $(ls compiler_tests | grep $FOLDER_NAME) ]; then
    echo "test folder not found"
    exit 1
fi
FOLDER="compiler_tests/$FOLDER_NAME"
TEST_LIST=$(ls -1 $FOLDER | grep _driver.c | awk -F "_driver.c" '{print $1}' )
if [ -z "$TEST_LIST" ]; then
    echo "no tests to run"
    exit 1
fi
make clean
make
TEST_COUNT=0
PASS_COUNT=0
if [ -z $(ls temp | grep $FOLDER_NAME) ]; then
    mkdir temp/$FOLDER_NAME
fi
for f in $TEST_LIST
do
    TEST_COUNT=$(( $TEST_COUNT + 1 ))
    bin/c_compiler -S "$FOLDER/$f.c" -o "temp/$FOLDER_NAME/$f.s" >/dev/null
    mips-linux-gnu-gcc -mfp32 -o "temp/$FOLDER_NAME/$f.o" -c "temp/$FOLDER_NAME/$f.s"
    mips-linux-gnu-gcc -mfp32 -static -o "temp/$FOLDER_NAME/$f" "temp/$FOLDER_NAME/$f.o" "$FOLDER/${f}_driver.c"
    qemu-mips "temp/$FOLDER_NAME/$f"
    RESULT=$?
    echo "==============================="
    echo "Test: $f"
    if [ $RESULT -eq 0 ]; then
        echo "Result: Pass"
        PASS_COUNT=$(( $PASS_COUNT + 1 ))
    else
        echo "Result: Fail"
    fi
    echo "==============================="
    echo ""
done
echo "Passed: $PASS_COUNT out of $TEST_COUNT"