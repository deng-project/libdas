# libdas: DENG asset management library
# licence: Apache, see LICENCE file
# file: libdas.cmake - libdas cmake configuration file
# author: Karl-Mihkel Ott

set(LIBDAS_SHARED_TARGET das-shared)
set(LIBDAS_STATIC_TARGET das-static)
set(LIBDAS_SOURCES
    src/Algorithm.cpp
    src/AsciiLineReader.cpp
    src/AsciiStreamReader.cpp
    src/Base64Decoder.cpp
    src/BufferImageTypeResolver.cpp
    src/DasParser.cpp
    src/DasReaderCore.cpp
    src/DasStructures.cpp
    src/DasWriterCore.cpp
    src/Debug.cpp
    src/ErrorHandlers.cpp
    src/GLTFCompiler.cpp
    src/GLTFParser.cpp
    src/Hash.cpp
    src/HuffmanCompression.cpp
    src/interpolation
    src/JSONParser.cpp
    src/STLCompiler.cpp
    src/STLParser.cpp
    src/STLStructures.cpp
    src/tests
    src/TextureDump.cpp
    src/TextureReader.cpp
    src/URIResolver.cpp
    src/WavefrontObjCompiler.cpp
    src/WavefrontObjParser.cpp
    src/WavefrontObjStructures.cpp
)

set(LIBDAS_HEADERS
    include/Algorithm.h
    include/Api.h
    include/AsciiLineReader.h
    include/AsciiStreamReader.h
    include/Base64Decoder.h
    include/BufferImageTypeResolver.h
    include/DasParser.h
    include/DasReaderCore.h
    include/DasStructures.h
    include/DasWriterCore.h
    include/Debug.h
    include/ErrorHandlers.h
    include/GLTFCompiler.h
    include/GLTFParser.h
    include/GLTFStructures.h
    include/Hash.h
    include/HuffmanCompression.h
    include/Iterators.h
    include/JSONParser.h
    include/LibdasAssert.h
    include/Libdas.h
    include/Matrix.h
    include/Points.h
    include/Quaternion.h
    include/SIMD.h
    include/stb_image.h
    include/STDUtils.h
    include/STLCompiler.h
    include/STLParser.h
    include/STLStructures.h
    include/TextureReader.h
    include/URIResolver.h
    include/Vector.h
    include/WavefrontObjCompiler.h
    include/WavefrontObjParser.h
    include/WavefrontObjStructures.h
)

add_library(${LIBDAS_STATIC_TARGET} STATIC 
    ${LIBDAS_HEADERS} 
    ${LIBDAS_SOURCES}
)

add_library(${LIBDAS_SHARED_TARGET} SHARED 
    ${LIBDAS_HEADERS} 
    ${LIBDAS_SOURCES}
)

target_include_directories(${LIBDAS_STATIC_TARGET} PUBLIC include)
target_include_directories(${LIBDAS_SHARED_TARGET} PUBLIC include)
