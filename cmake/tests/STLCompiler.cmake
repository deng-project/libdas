# libdas: DENG asset management library
# licence: Apache, see LICENCE file
# file: STLCompiler.cmake - STLCompiler class test build configuration
# author: Karl-Mihkel Ott

set(STL_COMPILER_TARGET STLCompilerTest)
set(STL_COMPILER_SOURCES tests/STLCompilerTest.cpp) 

add_executable(${STL_COMPILER_TARGET} ${STL_COMPILER_SOURCES})
target_link_libraries(${STL_COMPILER_TARGET} PRIVATE ${LIBDAS_SHARED_TARGET})
add_dependencies(${STL_COMPILER_TARGET} ${LIBDAS_SHARED_TARGET} ${LIBDAS_STATIC_TARGET})
