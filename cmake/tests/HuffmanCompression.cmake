# libdas: DENG asset management library
# licence: Apache, see LICENCE file
# file: AsciiSTLParser.cmake - AsciiSTLParser class test build configuration
# author: Karl-Mihkel Ott

set(HUFFMAN_COMPRESSION_TEST_TARGET HuffmanCompressionTest)
set(HUFFMAN_COMPRESSION_TEST_SOURCES tests/HuffmanCompressionTest.cpp) 

add_executable(${HUFFMAN_COMPRESSION_TEST_TARGET} ${HUFFMAN_COMPRESSION_TEST_SOURCES})
target_link_libraries(${HUFFMAN_COMPRESSION_TEST_TARGET} PRIVATE ${LIBDAS_SHARED_TARGET})
add_dependencies(${HUFFMAN_COMPRESSION_TEST_TARGET} ${LIBDAS_SHARED_TARGET} ${LIBDAS_STATIC_TARGET})
