#!/bin/sh

TEST_PATH=Compile_Test

# For eached blobbed file
for f in $TEST_PATH/Blobbed/*.das; do
    dastool list "$f" -v 1>/dev/null 2>/dev/null

    ## Check if command succeeded
    if [ $? -eq 0 ]; then
        echo OK
    else
        echo "FAIL"
        echo "File name: $f"
        exit
    fi
done

# For eached embedded file
for f in $TEST_PATH/Embedded/*.das; do
    dastool list "$f" -v 1>/dev/null 2>/dev/null

    ## Check if command succeeded
    if [ $? -eq 0 ]; then
        echo OK
    else
        echo "FAIL"
        echo "File name: $f"
        exit
    fi
done
