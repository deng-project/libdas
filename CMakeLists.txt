# libdas: DENG asset management library
# licence: Apache, see LICENCE file
# file: CMakeLists.txt - Main CMake configuration file
# author: Karl-Mihkel Ott

cmake_minimum_required(VERSION 3.10)
project(libdas LANGUAGES CXX)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(LIBDAS_BUILD_TESTS "Build all tests along with main libdas libraries" OFF)
option(LIBDAS_BUILD_STATIC_LIB "Build static libdas library" ON)
option(LIBDAS_BUILD_SHARED_LIB "Build shared libdas library" ON)
option(LIBDAS_BUILD_DASTOOL "Build dastool application" ON)
option(LIBDAS_BUILD_DEPENDENCIES "Fetch and build libdas dependencies" ON)

# Set appropriate compiler flags
if(MSVC)
    add_compile_options(/W3 /std:c++17 /wd4251)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)

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
    add_definitions(-D__DEBUG)
endif()

if(LIBDAS_BUILD_DEPENDENCIES)
    include(cmake/FetchDependencies.cmake)
endif()

# Include main library build config
include(cmake/libdas.cmake)

if(LIBDAS_BUILD_DASTOOL)
	include(cmake/dastool.cmake)
endif()

# Add test configurations
if(LIBDAS_BUILD_TESTS)
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
