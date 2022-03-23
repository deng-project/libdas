# libdas: DENG asset management library
# licence: Apache, see LICENCE file
# file: Base64Decoder.cmake - AsciiSTLParser class test build configuration
# author: Karl-Mihkel Ott

set(BASE64_DECODER_TARGET Base64DecoderTest)
set(BASE64_DECODER_SOURCES tests/Base64DecoderTest.cpp) 

add_executable(${BASE64_DECODER_TARGET} ${BASE64_DECODER_SOURCES})
target_link_libraries(${BASE64_DECODER_TARGET} PRIVATE ${LIBDAS_SHARED_TARGET})
add_dependencies(${BASE64_DECODER_TARGET} ${LIBDAS_SHARED_TARGET} ${LIBDAS_STATIC_TARGET})
