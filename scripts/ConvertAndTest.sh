#!/bin/sh

# Source paths
ASCII_STL_PATH=../third_party/3DModelSamples/stla/*.stl
BINARY_STL_PATH=../third_party/3DModelSamples/stlb/*.stl
WAVEFRONT_OBJ_PATH=../third_party/3DModelSamples/wobj/*.obj
GLTF_BLOBBED_PATH=../third_party/glTF-Sample-Models/2.0/**/glTF/*.gltf
GLTF_EMBEDDED_PATH=../third_party/glTF-Sample-Models/2.0/**/glTF-Embedded/*.gltf
dastool_Binary=../build/dastool

ASCII_STL_DESTINATION=./Models/stla/
BINARY_STL_DESTINATION=./Models/stlb/
WAVEFRONT_OBJ_DESTINATION=./Models/obj/
GLTF_BLOBBED_DESTINATION=./Models/glTF/Blobbed/
GLTF_EMBEDDED_DESTINATION=./Models/glTF/Embedded/


# Helper function to make all destination directories
make_destination() {
    if [ ! -d $ASCII_STL_DESTINATION ]; then
        mkdir -p $ASCII_STL_DESTINATION
    fi

    if [ ! -d $BINARY_STL_DESTINATION ]; then
        mkdir -p $BINARY_STL_DESTINATION
    fi

    if [ ! -d $WAVEFRONT_OBJ_DESTINATION ]; then
        mkdir -p $WAVEFRONT_OBJ_DESTINATION
    fi

    if [ ! -d $GLTF_BLOBBED_DESTINATION ]; then
        mkdir -p $GLTF_BLOBBED_DESTINATION
    fi

    if [ ! -d $GLTF_EMBEDDED_DESTINATION ]; then
        mkdir -p $GLTF_EMBEDDED_DESTINATION
    fi
}


convert_to_das() {
    set +f
    for f in $1; do
        echo "Converting $f to $2"
        valgrind $dastool_Binary convert "$f" -o $2
    done
}

make_destination

set -f
convert_to_das ${ASCII_STL_PATH} ${ASCII_STL_DESTINATION}
set -f
convert_to_das ${BINARY_STL_PATH} ${BINARY_STL_DESTINATION}
set -f
convert_to_das ${WAVEFRONT_OBJ_PATH} ${WAVEFRONT_OBJ_DESTINATION}
set -f
convert_to_das ${GLTF_BLOBBED_PATH} ${GLTF_BLOBBED_DESTINATION}
set -f
convert_to_das ${GLTF_EMBEDDED_PATH} ${GLTF_EMBEDDED_DESTINATION}

