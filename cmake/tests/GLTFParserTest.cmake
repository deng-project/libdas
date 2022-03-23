# libdas: DENG asset management library
# licence: Apache, see LICENCE file
# file: GLTFParserTest.cmake - GLTFParser class test build configuration
# author: Karl-Mihkel Ott

set(GLTF_PARSER_TARGET GLTFParserTest)
set(GLTF_PARSER_SOURCES tests/GLTFParserTest.cpp) 

add_executable(${GLTF_PARSER_TARGET} ${GLTF_PARSER_SOURCES})
target_link_libraries(${GLTF_PARSER_TARGET} PRIVATE ${LIBDAS_SHARED_TARGET})
add_dependencies(${GLTF_PARSER_TARGET} ${LIBDAS_SHARED_TARGET} ${LIBDAS_STATIC_TARGET})
