# libdas: DENG asset management library
# licence: Apache, see LICENCE file
# file: SubstringSearchTest.cmake - KMP algorithm test application config
# author: Karl-Mihkel Ott

set(KMP_TARGET KMP)
set(KMP_SOURCES tests/KMP.cpp) 

add_executable(${KMP_TARGET} ${KMP_SOURCES})
target_link_libraries(${KMP_TARGET} PRIVATE ${LIBDAS_SHARED_TARGET})
add_dependencies(${KMP_TARGET} ${LIBDAS_SHARED_TARGET} ${LIBDAS_STATIC_TARGET})
