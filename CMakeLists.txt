cmake_minimum_required(VERSION 3.15)
project(simapi VERSION 1.0.1 DESCRIPTION "Telemetry Mapping Library for Racing Sims")
include(GNUInstallDirs)
add_library(simapi SHARED simapi/simmapper.c simapi/getpid.c simapi/mapping/acmapper.c simapi/mapping/pcars2mapper.c simapi/mapping/rf2mapper.c simapi/mapping/scs2mapper.c simapi/mapping/outgaugemapper.c simapi/mapping/dirt2mapper.c simapi/mapping/f12018mapper.c simapi/mapping/wreckfest2mapper.c simapi/mapping/rbrmapper.c simapi/mapping/forzamapper.c simapi/mapping/r3emapper.c)

set(SIMAPI_PUBLIC_HEADERS
    "simapi/simmapper.h"
    "simapi/simapi.h"
    "simapi/simdata.h")

set_target_properties(simapi PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    PUBLIC_HEADER "${SIMAPI_PUBLIC_HEADERS}")

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBPROCPS libprocps)
pkg_check_modules(LIBPROC2 libproc2)

if (LIBPROCPS_FOUND)
    #add_compile_definitions(USE_OLD_PID_VAL=0)
elseif (LIBPROC2_FOUND)
    # Some distro packagings -- the freedesktop Flatpak SDK among them -- ship
    # a libproc2.pc whose Version field is the literal string UNKNOWN. Compared
    # against that, VERSION_GREATER_EQUAL is always false, so this fell through
    # to the pre-4.0.5 four-argument PIDS_VAL API and failed to compile against
    # a current libproc2 whose header has long since moved to three. Take the
    # old-API branch only when the version really parses and really is old.
    if (LIBPROC2_VERSION MATCHES "^[0-9]" AND LIBPROC2_VERSION VERSION_LESS "4.0.5")
        add_compile_definitions(USE_OLD_PID_VAL)
    endif()
else()
    add_compile_definitions(USE_OLD_PID_VAL)
endif()

target_link_libraries(simapi proc2)

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
    add_executable(view_telemetry tests/view_telemetry.c)
    target_link_libraries(view_telemetry simapi m)
endif()


