cmake_minimum_required(VERSION 3.15...3.24)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

project("International Doom" LANGUAGES C NONE)

# Versions
set(BUILD_DOOM_VERSION "6.1")
set(BUILD_HERETIC_VERSION "6.0")
set(BUILD_HEXEN_VERSION "5.2")
set(BUILD_STRIFE_VERSION "0.0")

# Compile modules
Option(COMPILE_DOOM "Compile Doom module" ON)
Option(COMPILE_HERETIC "Compile Heretic module" ON)
Option(COMPILE_HEXEN "Compile Hexen module" ON)
# [Dasperal] Disable Strife by default. It won't compile until `JNechaevsky/inter-doom/pull/268` is implemented for Strife
Option(COMPILE_STRIFE "Compile Strife module" OFF)

include(CMakeDependentOption)
find_package(Git)
find_package(Python3 COMPONENTS Interpreter)

cmake_dependent_option(BUILD_PORTABLE "Build portable version with Windows like behavior (see COMPILING.MD)" OFF "NOT WIN32" OFF)

# Hash suffix options
cmake_dependent_option(NO_GIT_HASH "For Devs only! Don't run 'revision_check' at all" OFF "GIT_FOUND" ON)
cmake_dependent_option(NO_GIT_HASH_SUFFIX "For Devs only! Never append hash suffix to project version" OFF "NOT NO_GIT_HASH" ON)
cmake_dependent_option(FORCE_GIT_HASH_SUFFIX "For Devs only! Always append hash suffix to project version" OFF "NOT NO_GIT_HASH_SUFFIX" OFF)

# Package related variables
set(PACKAGE_TARNAME "inter-doom")
set(PACKAGE_COPYRIGHT "Copyright \\xA9 2016-2022 Julian Nechaevsky. Licensed under GNU General Public License, version 2")

# Check architecture
if(WIN32)
    if("${CMAKE_C_COMPILER}" MATCHES "^(.*mingw64.*)|(.*clang64.*)" OR
    (MSVC AND "${CMAKE_GENERATOR_PLATFORM}" STREQUAL "x64"))
        set(ARCH x64)
    elseif("${CMAKE_C_COMPILER}" MATCHES "^(.*mingw32.*)|(.*clang32.*)" OR
    (MSVC AND "${CMAKE_GENERATOR_PLATFORM}" STREQUAL "Win32"))
        set(ARCH x86)
    endif()
endif()

# Any settings that should apply to all targets in this directory and all subdirectories should go here
set(RD_C_STANDARD 99)

include(Utils)

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
    add_compile_definitions(
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

find_package(SDL2 2.0.18 REQUIRED)
find_package(SDL2_mixer 2.0.2 REQUIRED)
find_package(SDL2_net 2.0.0 REQUIRED)
find_package(samplerate)
find_package(miniz 2.0.0)
find_package(m)

# Use system of user-provided library if it exists
if(NOT MINIZ_FOUND)
    message(STATUS "Using vendored miniz library")
    add_subdirectory("lib/thirdparty/miniz")
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

# [Dasperal] BUILD_VERSION_OVERWRITE variable is used on CI to replace version with git hash
if(COMPILE_DOOM)
    string(REPLACE "." ", " WINDOWS_RC_DOOM_VERSION ${BUILD_DOOM_VERSION})
    if(BUILD_VERSION_OVERWRITE)
        set(BUILD_DOOM_VERSION ${BUILD_VERSION_OVERWRITE})
    endif()
    configure_file(src/doom/resource.rc.in src/doom/resource.rc @ONLY)
    list(APPEND COMPILE_MODULES "doom")
    set(GIT_INFO_TAG_PATTERN "[0-9]*")
endif()
if(COMPILE_HERETIC)
    string(REPLACE "." ", " WINDOWS_RC_HERETIC_VERSION ${BUILD_HERETIC_VERSION})
    if(BUILD_VERSION_OVERWRITE)
        set(BUILD_HERETIC_VERSION ${BUILD_VERSION_OVERWRITE})
    endif()
    configure_file(src/heretic/resource.rc.in src/heretic/resource.rc @ONLY)
    list(APPEND COMPILE_MODULES "heretic")
    set(GIT_INFO_TAG_PATTERN "heretic-[0-9]*")
endif()
if(COMPILE_HEXEN)
    string(REPLACE "." ", " WINDOWS_RC_HEXEN_VERSION ${BUILD_HEXEN_VERSION})
    if(BUILD_VERSION_OVERWRITE)
        set(BUILD_HEXEN_VERSION ${BUILD_VERSION_OVERWRITE})
    endif()
    configure_file(src/hexen/resource.rc.in src/hexen/resource.rc @ONLY)
    list(APPEND COMPILE_MODULES "hexen")
    set(GIT_INFO_TAG_PATTERN "hexen-[0-9]*")
endif()
if(COMPILE_STRIFE)
    string(REPLACE "." ", " WINDOWS_RC_STRIFE_VERSION ${BUILD_STRIFE_VERSION})
    if(BUILD_VERSION_OVERWRITE)
        set(BUILD_STRIFE_VERSION ${BUILD_VERSION_OVERWRITE})
    endif()
    configure_file(src/strife/resource.rc.in src/strife/resource.rc @ONLY)
    list(APPEND COMPILE_MODULES "strife")
    set(GIT_INFO_TAG_PATTERN "strife-[0-9]*")
endif()

enable_testing()

# Update git_info.h
if(NO_GIT_HASH)
    # Use a function to avoid accidentally messing the real variables
    configure_empty_git_info("cmake/git_info.h.in" "git_info.h")
else()
    if(NO_GIT_HASH_SUFFIX)
        unset(GIT_INFO_TAG_PATTERN)
    endif()
    if(FORCE_GIT_HASH_SUFFIX)
        # Check against an already existing outdated tag so that
        # 'revision_check' always finds new commits and emits a hash suffix
        set(GIT_INFO_TAG_PATTERN "5.3")
    endif()
    add_custom_target(revision_check
        COMMAND "${CMAKE_COMMAND}" -P "\"${PROJECT_SOURCE_DIR}/cmake/UpdateRevision.cmake\"" "\"git_info.h\"" "\"${GIT_INFO_TAG_PATTERN}\"" "${GIT_EXECUTABLE}"
        WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
    )
endif()

include(GNUInstallDirs)

foreach(SUBDIR lib man src)
    add_subdirectory("${SUBDIR}")
endforeach()

# CPack
if(WIN32)
    set(CPACK_GENERATOR ZIP)
    set(CPACK_SYSTEM_NAME "windows-${ARCH}")

    # Common CPack settings
    set(CPACK_PACKAGE_NAME "${PROGRAM_PREFIX}$COMPONENT$") # Will be configured by CPackProjectConfig.cmake
    set(CPACK_PACKAGE_VERSION "$VERSION$") # Will be configured by CPackProjectConfig.cmake
    set(CPACK_PACKAGE_CHECKSUM SHA256)

    get_cmake_property(CPACK_COMPONENTS_ALL COMPONENTS)
    list(REMOVE_ITEM CPACK_COMPONENTS_ALL "Unspecified")

    configure_file(cmake/CPackProjectConfig.cmake.in CPackProjectConfig.cmake @ONLY)
    set(CPACK_PROJECT_CONFIG_FILE CPackProjectConfig.cmake)
    include(Cpack)
endif()
