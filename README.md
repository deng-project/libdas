# Libdas

## What is it?

Libdas is a small C++ library for creating and loading binary das (DENG asset format) files. It attempts to solve the current issue
in DENG development, which is the lack of dynamical rendering, by offering appropriate vertex data structures and algorithms for manipulating
in-game entities (also known as assets) during the rendering process.


## Building

In order to build libdas library you will need [premake5](https://github.com/premake/premake-core).  

### Generate build files
If GNU toolchain is used you can generate Makefiles using following command:  
`premake5 gmake2`

On Windows with Visual Studio 2019 use following command:  
`premake5 vs2019`

When using GNU toolchain you can specify the build configuration using either `config=debug_linux` or `config=release_linux` respectively. Similarly
on Windows with mingw32 the configuration variable values can be following `config=debug_win32` or `config=release_win32`.

Additionally manual tests can be built using `--test` switch on premake5 command. See `premake5 --help` for more information.


## Generating documentation

The source code is commented using doxygen notation. In order to generate Libdas reference documentation use following command:  
`doxygen doxygen.conf`  
