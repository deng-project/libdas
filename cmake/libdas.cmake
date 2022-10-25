# libdas: DENG asset management library
# licence: Apache, see LICENCE file
# file: libdas.cmake - libdas cmake configuration file
# author: Karl-Mihkel Ott

set(LIBDAS_SHARED_TARGET das-shared)
set(LIBDAS_STATIC_TARGET das-static)
set(LIBDAS_SOURCES
    src/Algorithm.cpp
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
    include/das/Algorithm.h
    include/das/Api.h
    include/das/Base64Decoder.h
    include/das/BufferImageTypeResolver.h
    include/das/DasParser.h
    include/das/DasReaderCore.h
    include/das/DasStructures.h
    include/das/DasValidator.h
    include/das/DasWriterCore.h
    include/das/Debug.h
    include/das/ErrorHandlers.h
    include/das/GLTFCompiler.h
    include/das/GLTFParser.h
    include/das/GLTFStructures.h
    include/das/Hash.h
    include/das/HuffmanCompression.h
    include/das/JSONParser.h
    include/das/LibdasAssert.h
    include/das/Libdas.h
    include/das/stb_image.h
    include/das/STLCompiler.h
    include/das/STLParser.h
    include/das/STLStructures.h
    include/das/TextureReader.h
    include/das/URIResolver.h
	include/das/Version.h
    include/das/WavefrontObjCompiler.h
    include/das/WavefrontObjParser.h
    include/das/WavefrontObjStructures.h
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

    target_include_directories(${LIBDAS_STATIC_TARGET} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
    if(LIBDAS_BUILD_DEPENDENCIES)
        target_include_directories(${LIBDAS_STATIC_TARGET} 
            PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/deps/mar/include
            PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/deps/trs/include)
    endif()
    target_link_libraries(${LIBDAS_STATIC_TARGET} PUBLIC mar)
	target_compile_definitions(${LIBDAS_STATIC_TARGET} PUBLIC LIBDAS_STATIC)
endif()


# Shared library configuration
if(LIBDAS_BUILD_SHARED_LIB)
	add_library(${LIBDAS_SHARED_TARGET} SHARED 
		${LIBDAS_HEADERS} 
		${LIBDAS_SOURCES}
	)

	target_include_directories(${LIBDAS_SHARED_TARGET} PUBLIC include)
    if(LIBDAS_BUILD_DEPENDENCIES)
        target_include_directories(${LIBDAS_SHARED_TARGET} 
            PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/deps/mar/include
            PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/deps/trs/include)
    endif()
	
	target_link_libraries(${LIBDAS_SHARED_TARGET} PUBLIC mar)
	target_compile_definitions(${LIBDAS_SHARED_TARGET} PRIVATE LIBDAS_EXPORT_LIBRARY)
endif()
