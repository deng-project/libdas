#!/bin/sh

READDIR=Compile_Test

# For each das file generated from blobbed GLTF
for f in ${READDIR}/**/*.das; do
    dastool validate "$f" 1>/dev/null 2>/tmp/dastool.stderr
    if [ ! -s /tmp/dastool.stderr ]; then
        echo "OK"
    else
        echo "FAIL"
        cat /tmp/dastool.stderr
        echo "File name: $f"
        exit
    fi
done
