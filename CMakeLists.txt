cmake_minimum_required(VERSION 3.15...3.26)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# CMP0077 supress some arcane CMake warning when using vcpkg.
set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
set(VCPKG_OVERLAY_TRIPLETS ${CMAKE_CURRENT_SOURCE_DIR}/cmake/triplets)
# Automatically copy dependencies into the output directory for executables when using vcpkg.
set(VCPKG_APPLOCAL_DEPS ON)
# Automatically installs dependencies when using vcpkg.
set(X_VCPKG_APPLOCAL_DEPS_INSTALL ON)
Option(VCPKG_MANIFEST_MODE "Operate in vcpkg manifest mode instead of classic mode" ON)

# Project Name and Version
project("Russian Doom" VERSION 6.3 LANGUAGES C NONE)
set(PACKAGE_ID "io.github.russian_doom.russian_doom")

Option(RD_TEST_WITH_GDB "Attach gdb to CTest tests" OFF)

include(CMakeDependentOption)
find_package(Python3 COMPONENTS Interpreter)

# Package related variables
set(PACKAGE_TARNAME "russian-doom")
set(PACKAGE_COPYRIGHT "Copyright \\xA9 2016-2023 Julian Nechaevsky. Copyright \\xA9 2020-2025 Leonid Murin (Dasperal). Licensed under GNU General Public License, version 2")

# Check target architecture
if(MSVC)
    # Shortcut for MSVC since it has explicit option for architecture
    if(CMAKE_GENERATOR_PLATFORM STREQUAL Win32)
        set(ARCH x86)
    elseif(CMAKE_GENERATOR_PLATFORM STREQUAL x64)
        set(ARCH x64)
    elseif(CMAKE_GENERATOR_PLATFORM STREQUAL ARM)
        set(ARCH AArch32)
    elseif(CMAKE_GENERATOR_PLATFORM STREQUAL ARM64)
        set(ARCH AArch64)
    endif()
endif()
if(NOT ARCH)
    # Get target architecture from the compiler
    include(TargetArch)
    target_architecture(ARCH)
    # Normalize
    if(ARCH STREQUAL i386)
        set(ARCH x86)
    elseif(ARCH STREQUAL x86_64
        OR ARCH STREQUAL ia64)
        set(ARCH x64)
    elseif(ARCH STREQUAL arm)
        set(ARCH AArch32)
    elseif(ARCH STREQUAL arm64)
        set(ARCH AArch64)
    elseif(ARCH STREQUAL unknown)
        unset(ARCH)
    endif()
endif()

# Enable by default for MinGW (MSYS) environments
# Disabled for vcpkg, because it copies and installs DLLs for dependencies by itself
cmake_dependent_option(RD_USE_SELECTED_DLL_SET
    "Use 'Selected DLL set' instead of DLLs provided by build environment" "${MINGW}" "NOT VCPKG_TOOLCHAIN" OFF
)

# Any settings that should apply to all targets in this directory and all subdirectories should go here
set(RD_C_STANDARD 99)

# Enable LTO if available.
include(CheckIPOSupported)
check_ipo_supported(RESULT HAVE_LTO)
cmake_dependent_option(RD_ENABLE_LTO "Use link-time optimisation" ON "HAVE_LTO" OFF)

include(Utils)
include(VersionFromGit)

# Parameters we want to check for on all compilers
#
# Note that we want to check for these, even on MSVC, because some compilers
# that pretend to be MSVC can take both GCC and MSVC-style parameters at the
# same time, like clang-cl.exe

add_compile_options_checked(
    # Extra errors
    -Werror=implicit-function-declaration
    -Werror=incompatible-pointer-types
    -Werror=int-conversion

    # Extra warnings
    -Wformat=2
    -Wredundant-decls
    -Wrestrict

    # Disable warnings
    -Wno-unknown-pragmas
    -Wno-unused-result
    -Wno-format-nonliteral
    -Wno-discarded-qualifiers
    -Wno-incompatible-pointer-types-discards-qualifiers
)

if(MSVC)
    add_common_compile_definitions(
        # Silence the usual warnings for POSIX and standard C functions
        _CRT_NONSTDC_NO_DEPRECATE
        _CRT_SECURE_NO_WARNINGS
        _CRT_SECURE_NO_DEPRECATE
    )
    add_compile_options_checked(
        # Default warning setting for MSVC
        /W3

        # Extra errors for cl.exe
        /we4013 # Implicit function declaration
        /we4133 # Incompatible types
        /we4477 # Format string mismatch

        # Disable warnings for cl.exe
        /wd4244 # Disable "possible loss of data" warning
        /wd4068 # Disable "unknown pragma" warning

        # Disable warnings for clang-cl.exe - prevents warning spam in SDL headers
        -Wno-pragma-pack
    )
    string(REPLACE "/O3" "/O2" CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE}")
    string(REPLACE "/O3" "/O2" CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO}")
else()
    # We only want -Wall on GCC compilers, since /Wall on MSVC is noisy
    add_compile_options_checked(-Wall)
    string(REPLACE "-O3" "-O2" CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE}")
    string(REPLACE "-O3" "-O2" CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO}")
endif()

Option(CMAKE_FIND_PACKAGE_PREFER_CONFIG "Prefer dependency search in config mode over find module mode" OFF)

if(VCPKG_TOOLCHAIN)
    set(CMAKE_FIND_PACKAGE_PREFER_CONFIG TRUE)
endif()

find_package(SDL2 2.0.18 REQUIRED)
find_package(SDL2_mixer 2.0.2 REQUIRED)
find_package(SDL2_net 2.0.0 REQUIRED)
find_package(SampleRate)
find_package(miniz 2.0.0)
find_package(m)

# Use system of user-provided library if it exists
if(NOT miniz_FOUND)
    message(STATUS "Using vendored miniz library")
    add_subdirectory("lib/thirdparty/miniz")
endif()

# Construct path where to search
if(WIN32 AND (Python3_Interpreter_FOUND OR CMAKE_VERSION VERSION_GREATER_EQUAL 3.16)
AND NOT VCPKG_TOOLCHAIN
AND NOT RD_USE_SELECTED_DLL_SET)
    applocal_search_path(APPLOCAL_SEARCH_PATH TARGETS "SDL2::SDL2" "SDL2_mixer::SDL2_mixer" "SDL2_net::SDL2_net" "SampleRate::samplerate" "miniz::miniz")
endif()

include(CheckSymbolExists)
include(CheckIncludeFile)
check_symbol_exists(strcasecmp "strings.h" HAVE_DECL_STRCASECMP)
check_symbol_exists(strncasecmp "strings.h" HAVE_DECL_STRNCASECMP)
check_symbol_exists(sscanf_s "stdio.h" HAVE_DECL_SSCANF_S)
check_symbol_exists(ioperm "sys/io.h" HAVE_IOPERM)
check_symbol_exists(mmap "sys/mman.h" HAVE_MMAP)
check_symbol_exists(i386_iopl "i386/pio.h" HAVE_LIBI386)
check_symbol_exists(amd64_iopl "amd64/pio.h" HAVE_LIBAMD64)

check_include_file("dirent.h" HAVE_DIRENT_H)
check_include_file("dev/isa/spkrio.h" HAVE_DEV_ISA_SPKRIO_H)
check_include_file("dev/speaker/speaker.h" HAVE_DEV_SPEAKER_SPEAKER_H)
check_include_file("linux/kd.h" HAVE_LINUX_KD_H)

string(REPLACE " Doom" "" PACKAGE_PREFIX "${PROJECT_NAME}")
string(REPLACE "doom" "" PROGRAM_PREFIX "${PACKAGE_TARNAME}")
string(REPLACE "." ", " WINDOWS_RC_VERSION ${PROJECT_VERSION})

configure_file(src/doom/resource.rc.in src/doom/resource.rc @ONLY)
configure_file(src/heretic/resource.rc.in src/heretic/resource.rc @ONLY)
configure_file(src/hexen/resource.rc.in src/hexen/resource.rc @ONLY)

set(COMPILE_MODULES "doom" "heretic" "hexen")

enable_testing()

include(GNUInstallDirs)

foreach(SUBDIR lib data src)
    add_subdirectory("${SUBDIR}")
endforeach()

# CPack
if(WIN32)
    set(CPACK_PACKAGE_NAME ${PACKAGE_TARNAME})
    set(CPACK_PACKAGE_CHECKSUM SHA256)
    set(CPACK_PROJECT_CONFIG_FILE "CPackProjectConfig.cmake")

    set(CPACK_GENERATOR ZIP)
    set(CPACK_SYSTEM_NAME "windows-${ARCH}")

    # Archive Generator
    set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY OFF)
    set(CPACK_ARCHIVE_COMPONENT_INSTALL OFF)
    set(CPACK_ARCHIVE_THREADS 0) # For Cmake 3.18+

    include(CPack)
endif()
