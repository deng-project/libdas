# libdas: DENG asset management library
# licence: Apache, see LICENCE file
# file: GLTFCompilerTest.cmake - GLTFCompiler class test build configuration
# author: Karl-Mihkel Ott

set(GLTF_COMPILER_TARGET GLTFCompilerTest)
set(GLTF_COMPILER_SOURCES tests/GLTFCompilerTest.cpp) 

add_executable(${GLTF_COMPILER_TARGET} ${GLTF_COMPILER_SOURCES})
target_link_libraries(${GLTF_COMPILER_TARGET} PRIVATE ${LIBDAS_SHARED_TARGET})
add_dependencies(${GLTF_COMPILER_TARGET} ${LIBDAS_SHARED_TARGET} ${LIBDAS_STATIC_TARGET})
