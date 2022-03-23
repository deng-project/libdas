# libdas: DENG asset management library
# licence: Apache, see LICENCE file
# file: BinarySTLParser.cmake - BinarySTLParser class test build configuration
# author: Karl-Mihkel Ott

set(BINARY_STL_PARSER_TARGET BinarySTLParserTest)
set(BINARY_STL_PARSER_SOURCES tests/BinarySTLParseTest.cpp) 

add_executable(${BINARY_STL_PARSER_TARGET} ${BINARY_STL_PARSER_SOURCES})
target_link_libraries(${BINARY_STL_PARSER_TARGET} PRIVATE ${LIBDAS_SHARED_TARGET})
add_dependencies(${BINARY_STL_PARSER_TARGET} ${LIBDAS_SHARED_TARGET} ${LIBDAS_STATIC_TARGET})
