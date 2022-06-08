#!/bin/sh

OUTDIR=Compile_Test
WOBJ_PATH=/mnt/raid_pt/Documents/FileSamples/wobj

# Create $OUTDIR if it doesn't exist
if [ ! -d $OUTDIR ]; then
    mkdir $OUTDIR
fi

# Create $OUTDIR/Wavefront_Obj if it doesn't exist
if [ ! -d $OUTDIR/Wavefront_Obj ]; then
    mkdir $OUTDIR/Wavefront_Obj
fi

# For each Wavefront obj file
for f in $WOBJ_PATH/*.obj; do
    dastool convert "$f" -o $OUTDIR/Wavefront_Obj/

    ## Check if command succeeded
    if [ $? -eq 0 ]; then
        echo OK
    else
        echo FAIL
        echo "File name: $f"
        exit
    fi
done
