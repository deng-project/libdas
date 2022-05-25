# DENG: dynamic engine - powerful 3D game engine
# licence: Apache, see LICENCE file
# file: dastool.cmake - dastool application build config
# author: Karl-Mihkel Ott

set(DASTOOL_TARGET dastool)
set(DASTOOL_SOURCES src/DASTool.cpp include/DASTool.h)

add_executable(${DASTOOL_TARGET} ${DASTOOL_SOURCES})

# Linking
if(LIBDAS_BUILD_SHARED_LIB)
	target_link_libraries(${DASTOOL_TARGET} PRIVATE ${LIBDAS_SHARED_TARGET})
	add_dependencies(${DASTOOL_TARGET} ${LIBDAS_SHARED_TARGET})
elseif(LIBDAS_BUILD_STATIC_LIB)
	target_link_libraries(${DASTOOL_TARGET} PRIVATE ${LIBDAS_STATIC_TARGET})
	add_dependencies(${DASTOOL_TARGET} ${LIBDAS_STATIC_TARGET})
else()
	message(FATAL_ERROR "Cannot build dastool without building libdas")
endif()

install(TARGETS ${DASTOOL_TARGET})
