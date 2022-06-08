#!/bin/sh

OUTDIR=Compile_Test
STLA_PATH=/mnt/raid_pt/Documents/FileSamples/stla
STLB_PATH=/mnt/raid_pt/Documents/FileSamples/stlb

# Create $OUTDIR if it doesn't exist
if [ ! -d $OUTDIR ]; then
    mkdir $OUTDIR
fi

# Create $OUTDIR/STL_Ascii if it doesn't exist
if [ ! -d $OUTDIR/STL_Ascii ]; then
    mkdir $OUTDIR/STL_Ascii
fi

# Create $OUTDIR/STL_Binary if it doesn't exist
if [ ! -d $OUTDIR/STL_Binary ]; then
    mkdir $OUTDIR/STL_Binary
fi

# For each ascii stl file perform compilation
for f in $STLA_PATH/*.stl; do
    dastool convert "$f" -o $OUTDIR/STL_Ascii/

    ## Check if command succeeded
    if [ $? -eq 0 ]; then
        echo OK
    else
        echo FAIL
        echo "File name: $f"
        exit
    fi
done


# For each binary stl file perform compilation
for f in $STLB_PATH/*.stl; do
    dastool convert "$f" -o $OUTDIR/STL_Binary/

    ## Check if command succeeded
    if [ $? -eq 0 ]; then
        echo OK
    else
        echo FAIL
        echo "Fail name: $f"
        exit
    fi
done
