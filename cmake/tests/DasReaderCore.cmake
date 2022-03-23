# libdas: DENG asset management library
# licence: Apache, see LICENCE file
# file: DasReaderCore.cmake - DasReaderCore class test build configuration
# author: Karl-Mihkel Ott

set(DAS_READER_CORE_TARGET DasReaderCoreTest)
set(DAS_READER_CORE_SOURCES tests/DasReaderCoreTest.cpp) 

add_executable(${DAS_READER_CORE_TARGET} ${DAS_READER_CORE_SOURCES})
target_link_libraries(${DAS_READER_CORE_TARGET} PRIVATE ${LIBDAS_SHARED_TARGET})
add_dependencies(${DAS_READER_CORE_TARGET} ${LIBDAS_SHARED_TARGET} ${LIBDAS_STATIC_TARGET})
