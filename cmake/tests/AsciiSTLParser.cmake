# libdas: DENG asset management library
# licence: Apache, see LICENCE file
# file: AsciiSTLParser.cmake - AsciiSTLParser class test build configuration
# author: Karl-Mihkel Ott

set(ASCII_STL_PARSER_TARGET AsciiSTLParserTest)
set(ASCII_STL_PARSER_SOURCES tests/AsciiSTLParseTest.cpp) 

add_executable(${ASCII_STL_PARSER_TARGET} ${ASCII_STL_PARSER_SOURCES})
target_link_libraries(${ASCII_STL_PARSER_TARGET} PRIVATE ${LIBDAS_SHARED_TARGET})
add_dependencies(${ASCII_STL_PARSER_TARGET} ${LIBDAS_SHARED_TARGET} ${LIBDAS_STATIC_TARGET})
