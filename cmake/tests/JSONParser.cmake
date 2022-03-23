# libdas: DENG asset management library
# licence: Apache, see LICENCE file
# file: JSONParser.cmake - JSONParser class test build configuration
# author: Karl-Mihkel Ott

set(JSON_PARSER_TARGET JSONParserTest)
set(JSON_PARSER_SOURCES tests/JSONParserTest.cpp) 

add_executable(${JSON_PARSER_TARGET} ${JSON_PARSER_SOURCES})
target_link_libraries(${JSON_PARSER_TARGET} PRIVATE ${LIBDAS_SHARED_TARGET})
add_dependencies(${JSON_PARSER_TARGET} ${LIBDAS_SHARED_TARGET} ${LIBDAS_STATIC_TARGET})
