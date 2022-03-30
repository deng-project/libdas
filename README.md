# Libdas

## What is it?

Libdas is a small C++ library for creating and loading binary das (DENG asset format) files. It attempts to solve the current issue
in DENG development, which is the lack of dynamical rendering, by offering appropriate vertex data structures and algorithms for manipulating
in-game entities (also known as assets) during the rendering process.


## Building

### Generate build files

The project is set up to use CMake for generating build files.  
`mkdir build`  
`cd build`  
`cmake ..`  
`make`  


## Generating documentation

The source code is commented using doxygen notation. In order to generate Libdas reference documentation use following command:  
`doxygen doxygen.conf`  
