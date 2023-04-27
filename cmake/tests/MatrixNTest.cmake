# libdas: DENG asset management library
# licence: Apache, see LICENCE file
# file: VectorNTest.cmake - MatrixN testing program
# author: Karl-Mihkel Ott

set(MATRIX_N_TEST_TARGET MatrixNTestInteractive)
set(MATRIX_N_TEST_SOURCES src/tests/MatrixNTestInteractive.cpp) 

add_executable(${MATRIX_N_TEST_TARGET} ${MATRIX_N_TEST_SOURCES})
target_link_libraries(${MATRIX_N_TEST_TARGET} PRIVATE ${LIBDAS_SHARED_TARGET})
add_dependencies(${MATRIX_N_TEST_TARGET} ${LIBDAS_SHARED_TARGET} ${LIBDAS_STATIC_TARGET})