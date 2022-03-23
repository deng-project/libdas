# libdas: DENG asset management library
# licence: Apache, see LICENCE file
# file: WavefrontObjCompiler.cmake - WavefrontObjCompiler class test build configuration
# author: Karl-Mihkel Ott

set(WAVEFRONT_OBJ_COMPILER_TARGET WavefrontObjCompilerTest)
set(WAVEFRONT_OBJ_COMPILER_SOURCES tests/WavefrontObjCompilerTest.cpp) 

add_executable(${WAVEFRONT_OBJ_COMPILER_TARGET} ${WAVEFRONT_OBJ_COMPILER_SOURCES})
target_link_libraries(${WAVEFRONT_OBJ_COMPILER_TARGET} PRIVATE ${LIBDAS_SHARED_TARGET})
add_dependencies(${WAVEFRONT_OBJ_COMPILER_TARGET} ${LIBDAS_SHARED_TARGET} ${LIBDAS_STATIC_TARGET})
