#!/bin/sh

ASCII_STL_DESTINATION=./Models/stla/*.das
BINARY_STL_DESTINATION=./Models/stlb/*.das
WAVEFRONT_OBJ_DESTINATION=./Models/obj/*.das
GLTF_BLOBBED_DESTINATION=./Models/glTF/Blobbed/*.das
GLTF_EMBEDDED_DESTINATION=./Models/glTF/Embedded/*.das
dastool_Binary=../build/dastool

# For each das file generated from blobbed GLTF
validate() {
    set +f
    for f in $1; do
        echo "Validating $f"
        $dastool_Binary validate "$f"
    done
}

set -f
validate ${ASCII_STL_DESTINATION}
set -f
validate ${BINARY_STL_DESTINATION}
set -f
validate ${WAVEFRONT_OBJ_STL_DESTINATION}
set -f
validate ${GLTF_BLOBBED_DESTINATION}
set -f
validate ${GLTF_EMBEDDED_DESTINATION}
