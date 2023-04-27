# libdas: DENG asset management library
# licence: Apache, see LICENCE file
# file: VectorNTest.cmake - VectorN testing program
# author: Karl-Mihkel Ott

set(VECTOR_N_TEST_TARGET VectorNTestInteractive)
set(VECTOR_N_TEST_SOURCES src/tests/VectorNTestInteractive.cpp) 

add_executable(${VECTOR_N_TEST_TARGET} ${VECTOR_N_TEST_SOURCES})
target_link_libraries(${VECTOR_N_TEST_TARGET} PRIVATE ${LIBDAS_SHARED_TARGET})
add_dependencies(${VECTOR_N_TEST_TARGET} ${LIBDAS_SHARED_TARGET} ${LIBDAS_STATIC_TARGET})