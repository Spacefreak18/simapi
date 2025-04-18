cmake_minimum_required(VERSION 3.9)
project(simapi VERSION 1.0.1 DESCRIPTION "sim mapping library")
include(GNUInstallDirs)
add_library(simapi SHARED simapi/simmapper.c simapi/getpid.c)

set(SIMAPI_PUBLIC_HEADERS
    "simapi/simmapper.h"
    "simapi/simapi.h"
    "simapi/simdata.h")

set_target_properties(simapi PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    PUBLIC_HEADER "${SIMAPI_PUBLIC_HEADERS}")

configure_file(simapi.pc.in simapi.pc @ONLY)
target_include_directories(simapi PRIVATE include simapi)
install(TARGETS simapi
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(FILES ${CMAKE_BINARY_DIR}/simapi.pc
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/pkgconfig)
