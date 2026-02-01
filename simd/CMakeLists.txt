# minimum CMake version required for C++20 support, among other things
cmake_minimum_required(VERSION 3.15)

SET_SOURCE_FILES_PROPERTIES( simd.c PROPERTIES LANGUAGE C)

project(simd)

set(CMAKE_EXE_LINKER_FLAGS "-Wl,--no-as-needed -ldl")
#set(CMAKE_BUILD_TYPE Debug)

# Argtable 3->2 fallback
find_path(ARGTABLE3_INCLUDE_DIR argtable3.h)
find_library(ARGTABLE3_LIBRARY argtable3)

if (ARGTABLE3_INCLUDE_DIR AND ARGTABLE3_LIBRARY)
    add_compile_definitions(HAVE_ARGTABLE3)
    set(ARGTABLE_LIBS ${ARGTABLE3_LIBRARY})
    set(ARGTABLE_INCLUDE_DIR ${ARGTABLE3_INCLUDE_DIR})
else()
    find_path(ARGTABLE2_INCLUDE_DIR argtable2.h)
    find_library(ARGTABLE2_LIBRARY argtable2)

    if (ARGTABLE2_INCLUDE_DIR AND ARGTABLE2_LIBRARY)
        add_compile_definitions(HAVE_ARGTABLE2)
        set(ARGTABLE_LIBS ${ARGTABLE2_LIBRARY})
        set(ARGTABLE_INCLUDE_DIR ${ARGTABLE2_INCLUDE_DIR})
    else()
        message(FATAL_ERROR "Neither argtable3 nor argtable2 found")
    endif()
endif()

add_executable(simd simd.c parameters.c confighelper.c dirhelper.c poke.c ../simmap/mapsimdata.c)
target_link_libraries(simd m uv yder ${ARGTABLE_LIBS} config simapi)

target_include_directories(simd PRIVATE ${ARGTABLE_INCLUDE_DIR})

# User-level installation
install(TARGETS simd RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

# Install systemd user service file to ~/.config/systemd/user/
option(INSTALL_SYSTEMD_SERVICE "Install systemd user service file" ON)
if(INSTALL_SYSTEMD_SERVICE)
    set(SYSTEMD_USER_UNIT_DIR "$ENV{HOME}/.config/systemd/user" CACHE PATH "Systemd user unit directory")
    install(FILES conf/simd.service DESTINATION ${SYSTEMD_USER_UNIT_DIR})
endif()

# Install default config file to ~/.config/simd/
option(INSTALL_DEFAULT_CONFIG "Install default configuration file" ON)
if(INSTALL_DEFAULT_CONFIG)
    set(SIMD_CONFIG_DIR "$ENV{HOME}/.config/simd" CACHE PATH "simd configuration directory")
    install(FILES conf/simd.config DESTINATION ${SIMD_CONFIG_DIR})
endif()

# used for enabling additional compiler options if supported
include(CheckCXXCompilerFlag)

function(enable_cxx_compiler_flag_if_supported flag)
    message(STATUS "[simd] Checking if compiler supports warning flag '${flag}'")
    check_cxx_compiler_flag("${flag}" flag_supported)
    if(flag_supported)
        message(STATUS "[simd] Enabling warning flag '${flag}'")
        target_compile_options(simd INTERFACE "${flag}")
    endif()
    unset(flag_supported CACHE)
endfunction()

# enable a large amount of extra warnings, regardless of build mode
if (MSVC) # MSVC supports different warning options to GCC/Clang
    enable_cxx_compiler_flag_if_supported("/W3")  # set warning level 3
    # if tests are enabled, enable converting all warnings to errors too
    if (ENABLE_TESTS)
        # add_compile_options(/WX)
        enable_cxx_compiler_flag_if_supported("/WX")
    endif()
else() # GCC/Clang warning option
    # NOTE: GCC and Clang support most of the same options, but neither supports all
    # of the others'. By only enabling them if supported, we get graceful failure
    # when trying to enable unsupported flags
    # e.g. at the time of writing, GCC does not support -Wdocumentation
    #
    # enable all warnings about 'questionable constructs'

    if(analyze)
        message("-- Analyzer is on")
        target_compile_options(simd PRIVATE -fanalyzer -Wno-analyzer-possible-null-argument -Wno-analyzer-possible-null-dereference)
    endif()

    enable_cxx_compiler_flag_if_supported("-Wall")
    # issue 'pedantic' warnings for strict ISO compliance
    enable_cxx_compiler_flag_if_supported("-pedantic")
    # enable 'extra' strict warnings
    enable_cxx_compiler_flag_if_supported("-Wextra")
    # enable sign conversion warnings
    enable_cxx_compiler_flag_if_supported("-Wsign-conversion")
    # enable warnings about mistakes in Doxygen documentation
    enable_cxx_compiler_flag_if_supported("-Wdocumentation")
    # if tests are enabled, enable converting all warnings to errors too
    if (ENABLE_TESTS)
        enable_cxx_compiler_flag_if_supported("-Werror")
        # exclude the following kinds of warnings from being converted into errors
        # unknown-pragma is useful to have as a warning but not as an error, if you have
        # pragmas which are for the consumption of one compiler only
        enable_cxx_compiler_flag_if_supported("-Wno-error=unknown-pragmas")
        # unused variable and function warnings are helpful but we don't need them as errors
        enable_cxx_compiler_flag_if_supported("-Wno-error=unused-function")
        enable_cxx_compiler_flag_if_supported("-Wno-error=unused-variable")
        enable_cxx_compiler_flag_if_supported("-Wno-error=unused-parameter")
        enable_cxx_compiler_flag_if_supported("-Wno-error=unused-private-field")
        enable_cxx_compiler_flag_if_supported("-Wno-error=unused-but-set-variable")
    endif()
endif()
