#!/bin/sh

echo "Compiling GLTF files to DAS ..."
./compile_gltf.sh
echo "Compiling STL files to DAS ..."
./compile_stl.sh
echo "Compiling Wavefront OBJ files to DAS ..."
./compile_wobj.sh
echo "List testing generated DAS files ..."
./list_test.sh
echo "Validating generated DAS files ..."
./validate_test.sh
