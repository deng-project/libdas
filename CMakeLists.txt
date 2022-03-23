# libdas: DENG asset management library
# licence: Apache, see LICENCE file
# file: CMakeLists.txt - Main CMake configuration file
# author: Karl-Mihkel Ott

cmake_minimum_required(VERSION 3.10)
project(libdas LANGUAGES CXX)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_GENERATOR_PLATFORM x64)

option(BUILD_TESTS "Build all tests along with main libdas library" OFF)

# Set appropriate compiler flags
if(MSVC)
    add_compile_options(/W4 /std:c++17)

    # Set correct optimisation flags
    if(CMAKE_BUILD_TYPE MATCHES Debug)
        add_compile_options(/Od)
    else()
        add_compile_options(/O2)
    endif()
else()
    add_compile_options(-Wall -Wextra -std=c++17)

    if(CMAKE_BUILD_TYPE MATCHES Debug)
        add_compile_options(-O0)
    else()
        add_compile_options(-O3)
    endif()
endif()

if(CMAKE_BUILD_TYPE MATCHES Debug)
    add_definitions(-D_DEBUG)
endif()

# Include main library build config
include(cmake/libdas.cmake)
include(cmake/dastool.cmake)

# Add test configurations
if(BUILD_TESTS)
    include(cmake/tests/AsciiSTLParser.cmake)
    include(cmake/tests/BinarySTLParser.cmake)
    include(cmake/tests/HuffmanCompression.cmake)
    include(cmake/tests/WavefrontObjCompiler.cmake)
    include(cmake/tests/STLCompiler.cmake)
    include(cmake/tests/Base64Decoder.cmake)
    include(cmake/tests/JSONParser.cmake)
    include(cmake/tests/GLTFParserTest.cmake)
    include(cmake/tests/GLTFCompilerTest.cmake)
    include(cmake/tests/TextureReader.cmake)
    include(cmake/tests/DasReaderCore.cmake)
    include(cmake/tests/SubstringSearchTest.cmake)
    include(cmake/tests/WavefrontObjParser.cmake)
endif()
