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

project("Russian Doom" LANGUAGES C NONE)

# Versions
set(BUILD_DOOM_VERSION "6.2")
set(BUILD_HERETIC_VERSION "6.1")
set(BUILD_HEXEN_VERSION "6.0")

# Compile modules
Option(RD_COMPILE_DOOM "Compile Doom module" ON)
Option(RD_COMPILE_HERETIC "Compile Heretic module" ON)
Option(RD_COMPILE_HEXEN "Compile Hexen module" ON)
Option(RD_TEST_WITH_GDB "Attach gdb to CTest tests" OFF)

include(CMakeDependentOption)
find_package(Git)
find_package(Python3 COMPONENTS Interpreter)

cmake_dependent_option(RD_BUILD_PORTABLE "Build portable version with Windows like behavior (see COMPILING.MD)" OFF "NOT WIN32" OFF)

# Hash suffix options
cmake_dependent_option(RD_GIT_NO_HASH "For Devs only! Don't run 'revision_check' at all" OFF "GIT_FOUND" ON)
cmake_dependent_option(RD_GIT_NO_HASH_SUFFIX "For Devs only! Never append hash suffix to project version" OFF "NOT NO_GIT_HASH" ON)
cmake_dependent_option(RD_GIT_FORCE_HASH_SUFFIX "For Devs only! Always append hash suffix to project version" OFF "NOT NO_GIT_HASH_SUFFIX" OFF)

# Package related variables
set(PACKAGE_TARNAME "russian-doom")
set(PACKAGE_COPYRIGHT "Copyright \\xA9 2016-2023 Julian Nechaevsky. Copyright \\xA9 2020-2023 Leonid Murin (Dasperal). Licensed under GNU General Public License, version 2")

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

#MSYS MINGW32 to UCRT32 hack
cmake_dependent_option(RD_UCRT32 "Use MSYS MINGW32 to UCRT32 hack" ON "WIN32;ARCH STREQUAL x86;MINGW;CMAKE_C_COMPILER_ID STREQUAL GNU" OFF)

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
    if(RD_UCRT32)
        add_compile_definitions(
            # Fixes utf-8 text in console
            _UCRT
        )
    endif()
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

# [Dasperal] RD_BUILD_VERSION_OVERWRITE variable is used on CI to replace version with git hash
if(RD_COMPILE_DOOM)
    string(REPLACE "." ", " WINDOWS_RC_DOOM_VERSION ${BUILD_DOOM_VERSION})
    if(RD_BUILD_VERSION_OVERWRITE)
        set(BUILD_DOOM_VERSION ${RD_BUILD_VERSION_OVERWRITE})
    endif()
    configure_file(src/doom/resource.rc.in src/doom/resource.rc @ONLY)
    list(APPEND COMPILE_MODULES "doom")
endif()
if(RD_COMPILE_HERETIC)
    string(REPLACE "." ", " WINDOWS_RC_HERETIC_VERSION ${BUILD_HERETIC_VERSION})
    if(RD_BUILD_VERSION_OVERWRITE)
        set(BUILD_HERETIC_VERSION ${RD_BUILD_VERSION_OVERWRITE})
    endif()
    configure_file(src/heretic/resource.rc.in src/heretic/resource.rc @ONLY)
    list(APPEND COMPILE_MODULES "heretic")
endif()
if(RD_COMPILE_HEXEN)
    string(REPLACE "." ", " WINDOWS_RC_HEXEN_VERSION ${BUILD_HEXEN_VERSION})
    if(RD_BUILD_VERSION_OVERWRITE)
        set(BUILD_HEXEN_VERSION ${RD_BUILD_VERSION_OVERWRITE})
    endif()
    configure_file(src/hexen/resource.rc.in src/hexen/resource.rc @ONLY)
    list(APPEND COMPILE_MODULES "hexen")
endif()

enable_testing()

# Update git_info.h
if(RD_GIT_NO_HASH)
    # Use a function to avoid accidentally messing the real variables
    configure_empty_git_info("cmake/git_info.h.in" "git_info.h")
else()
    if(RD_GIT_NO_HASH_SUFFIX)
        set(HASH_SUFFIX "NO")
    elseif(RD_GIT_FORCE_HASH_SUFFIX)
        set(HASH_SUFFIX "FORCE")
    else()
        set(HASH_SUFFIX "AUTO")
    endif()
    add_custom_target(revision_check
        COMMAND "${CMAKE_COMMAND}" -P "\"${PROJECT_SOURCE_DIR}/cmake/UpdateRevision.cmake\"" "\"git_info.h\"" "\"${HASH_SUFFIX}\"" "${GIT_EXECUTABLE}"
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
