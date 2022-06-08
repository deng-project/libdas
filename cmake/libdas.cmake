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
    src/DasValidator.cpp
    src/DasWriterCore.cpp
    src/ErrorHandlers.cpp
    src/GLTFCompiler.cpp
    src/GLTFParser.cpp
    src/Hash.cpp
    src/JSONParser.cpp
    src/STLCompiler.cpp
    src/STLParser.cpp
    src/STLStructures.cpp
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
    include/DasValidator.h
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
    include/stb_image.h
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

if(CMAKE_BUILD_TYPE MATCHES Debug)
    list(APPEND LIBDAS_SOURCES src/Debug.cpp)
endif()

# Static library configuration
if(LIBDAS_BUILD_STATIC_LIB)
	add_library(${LIBDAS_STATIC_TARGET} STATIC 
		${LIBDAS_HEADERS} 
		${LIBDAS_SOURCES}
	)

	target_include_directories(${LIBDAS_STATIC_TARGET} PUBLIC include)
	target_compile_definitions(${LIBDAS_STATIC_TARGET} PUBLIC LIBDAS_STATIC)
endif()


# Shared library configuration
if(LIBDAS_BUILD_SHARED_LIB)
	add_library(${LIBDAS_SHARED_TARGET} SHARED 
		${LIBDAS_HEADERS} 
		${LIBDAS_SOURCES}
	)

	target_include_directories(${LIBDAS_SHARED_TARGET} PUBLIC include)
	target_compile_definitions(${LIBDAS_SHARED_TARGET} PRIVATE LIBDAS_EXPORT_LIBRARY)
endif()
