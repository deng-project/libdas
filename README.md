# libdas

## What is it?

Libdas is a small C++ library for creating and loading binary das (DENG asset format) files. It attempts to solve the current issue
in DENG development, which is the lack of dynamical rendering, by offering appropriate vertex data structures and algorithms for manipulating
in-game entities (also known as assets) during the rendering process.  

## Current in progress feature list (not all features are implemented yet)

* DAM CLI utility for compiling well known 3D model formats, such as obj, fbx, stl and gltf into das binary format
* Texture loading from jpeg, png, tga, bmp, psd and ppm formats
* Face triangulation algorithms
* Mesh manifold testing

## Building and testing

Libdas and its tests can be built using GNU make. Make targets are following:  
* `make` -- build libdas and dam  
* `make ldtest` -- build a small program to test das binary loading capability (no graphical output)  
* `make asmtest` -- build a small program to test das binary compilation and source format parsing capabilities  
* `make mesh_query` -- build a small program to test vertex data querying capabilities (currently only creates triangles from vertices 
and displays each triangle, with its neighbours)  

## License

Libdas is licenced under Apache license (see [LICENSE](LICENSE) for more information)  
