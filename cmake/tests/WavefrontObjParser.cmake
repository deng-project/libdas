# libdas: DENG asset management library
# licence: Apache, see LICENCE file
# file: WavefrontObjParser.cmake - WavefrontObjParser class test build configuration
# author: Karl-Mihkel Ott

set(WAVEFRONT_OBJ_PARSER_TARGET WavefrontObjParserTest)
set(WAVEFRONT_OBJ_PARSER_SOURCES tests/WavefrontObjCompilerTest.cpp) 

add_executable(${WAVEFRONT_OBJ_PARSER_TARGET} ${WAVEFRONT_OBJ_PARSER_SOURCES})
target_link_libraries(${WAVEFRONT_OBJ_PARSER_TARGET} PRIVATE ${LIBDAS_SHARED_TARGET})
add_dependencies(${WAVEFRONT_OBJ_PARSER_TARGET} ${LIBDAS_SHARED_TARGET} ${LIBDAS_STATIC_TARGET})
