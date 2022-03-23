# libdas: DENG asset management library
# licence: Apache, see LICENCE file
# file: TextureReader.cmake - TextureReader class test build configuration
# author: Karl-Mihkel Ott

set(TEXTURE_READER_TARGET TextureReaderTest)
set(TEXTURE_READER_SOURCES tests/TextureReaderTest.cpp) 

add_executable(${TEXTURE_READER_TARGET} ${TEXTURE_READER_SOURCES})
target_link_libraries(${TEXTURE_READER_TARGET} PRIVATE ${LIBDAS_SHARED_TARGET})
add_dependencies(${TEXTURE_READER_TARGET} ${LIBDAS_SHARED_TARGET} ${LIBDAS_STATIC_TARGET})
