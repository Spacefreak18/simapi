cmake_minimum_required(VERSION 3.15)
project(simapi VERSION 1.0.1 DESCRIPTION "Telemetry Mapping Library for Racing Sims")
include(GNUInstallDirs)
add_library(simapi SHARED simapi/simmapper.c simapi/getpid.c simapi/mapping/acmapper.c simapi/mapping/pcars2mapper.c simapi/mapping/rf2mapper.c simapi/mapping/scs2mapper.c simapi/mapping/outgaugemapper.c simapi/mapping/dirt2mapper.c simapi/mapping/f12018mapper.c simapi/mapping/wreckfest2mapper.c simapi/mapping/rbrmapper.c)

set(SIMAPI_PUBLIC_HEADERS
    "simapi/simmapper.h"
    "simapi/simapi.h"
    "simapi/simdata.h")

set_target_properties(simapi PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    PUBLIC_HEADER "${SIMAPI_PUBLIC_HEADERS}")

configure_file(simapi.pc.in simapi.pc @ONLY)
target_include_directories(simapi PUBLIC include simapi)
install(TARGETS simapi
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(FILES ${CMAKE_BINARY_DIR}/simapi.pc
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/pkgconfig)

option(BUILD_SIMD "Build the simd tool" ON)
if(BUILD_SIMD)
    add_subdirectory(simd)
endif()

add_executable(view_telemetry tests/view_telemetry.c)
target_link_libraries(view_telemetry simapi m)
