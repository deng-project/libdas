#!/bin/sh

ASCII_STL_DESTINATION=./Models/stla/*.das
BINARY_STL_DESTINATION=./Models/stlb/*.das
WAVEFRONT_OBJ_DESTINATION=./Models/obj/*.das
GLTF_BLOBBED_DESTINATION=./Models/glTF/Blobbed/*.das
GLTF_EMBEDDED_DESTINATION=./Models/glTF/Embedded/*.das
dastool_Binary=../build/dastool

listall() {
    set +f
    for f in $1; do
        echo "Listing $f"
        valgrind $dastool_Binary list "$f" 1>/dev/null
    done
}

set -f
listall ${ASCII_STL_DESTINATION}
set -f
listall ${ASCII_BINARY_DESTINATION}
set -f
listall ${WAVEFRONT_OBJ_DESTINATION}
set -f
listall ${GLTF_BLOBBED_DESTINATION}
set -f
listall ${GLTF_EMBEDDED_DESTINATION}
