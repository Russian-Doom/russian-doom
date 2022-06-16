cmake_minimum_required(VERSION 3.15...3.23)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

project("International Doom" LANGUAGES C NONE)

# Versions
set(BUILD_DOOM_VERSION "5.4")
set(BUILD_HERETIC_VERSION "5.3")
set(BUILD_HEXEN_VERSION "5.2")
set(BUILD_STRIFE_VERSION "0.0")

# Compile modules
Option(COMPILE_DOOM "Compile Doom module" ON)
Option(COMPILE_HERETIC "Compile Heretic module" ON)
Option(COMPILE_HEXEN "Compile Hexen module" ON)
# [Dasperal] Disable Strife by default. It won't compile until `JNechaevsky/inter-doom/pull/268` is implemented for Strife
Option(COMPILE_STRIFE "Compile Strife module" OFF)
# [Dasperal] Disable Setup by default. Usage of Setup resets many of the settings available in the game menu, so we don't distribute it
Option(COMPILE_SETUP "Compile Setup module" OFF)

include(CMakeDependentOption)

# Hash suffix options
option(NO_GIT_HASH_SUFFIX "For Devs only! Never append hash suffix to project version" OFF)
cmake_dependent_option(FORCE_GIT_HASH_SUFFIX "For Devs only! Always append hash suffix to project version" OFF "NOT NO_GIT_HASH_SUFFIX" OFF)

# [Dasperal] BUILD_HAS_SDL_MIXER_PATCH should be ON only if used SDL_Mixer runtime library is new enough and has https://github.com/libsdl-org/SDL_mixer/commit/1c0092787398097360f7da745c7644fd32697f3b patch
cmake_dependent_option(BUILD_HAS_SDL_MIXER_PATCH "Use features available in new enough SDL_mixer runtime library (see COMPILING.MD)" OFF "NOT WIN32" ON)

# Package related variables
set(PACKAGE_TARNAME "inter-doom")
set(PACKAGE_COPYRIGHT "Copyright \\xA9 2016-2022 Julian Nechaevsky. Licensed under GNU General Public License, version 2")

# Check architecture
if(WIN32)
    if("${CMAKE_C_COMPILER}" MATCHES "^(.*mingw64.*)|(.*clang64.*)")
        set(ARCH x64)
    elseif("${CMAKE_C_COMPILER}" MATCHES "^(.*mingw32.*)|(.*clang32.*)")
        set(ARCH x86)
    endif()
endif()

# Any settings that should apply to all targets in this directory and all
# subdirectories should go here.  Use judiciously.
set(RD_C_STANDARD 99)

add_compile_options("-Wall" "-Wredundant-decls" "-Wno-unknown-pragmas" "-Wno-unused-result")
string(REPLACE "-O3" "-O2" CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE}")
string(REPLACE "-O3" "-O2" CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO}")

find_package(SDL2 2.0.4 REQUIRED)
find_package(SDL2_mixer 2.0.2 REQUIRED)
find_package(SDL2_net 2.0.0 REQUIRED)
find_package(samplerate)
find_package(PNG)
find_package(m)

include(CheckSymbolExists)
include(CheckIncludeFile)
check_symbol_exists(strcasecmp "strings.h" HAVE_DECL_STRCASECMP)
check_symbol_exists(strncasecmp "strings.h" HAVE_DECL_STRNCASECMP)
check_symbol_exists(ioperm "sys/io.h" HAVE_IOPERM)
check_symbol_exists(mmap "sys/mman.h" HAVE_MMAP)
check_symbol_exists(i386_iopl "i386/pio.h" HAVE_LIBI386)
check_symbol_exists(amd64_iopl "amd64/pio.h" HAVE_LIBAMD64)

check_include_file("dev/isa/spkrio.h" HAVE_DEV_ISA_SPKRIO_H)
check_include_file("dev/speaker/speaker.h" HAVE_DEV_SPEAKER_SPEAKER_H)
check_include_file("linux/kd.h" HAVE_LINUX_KD_H)

string(REPLACE " Doom" "" PACKAGE_PREFIX "${PROJECT_NAME}")
string(REPLACE "doom" "" PROGRAM_PREFIX "${PACKAGE_TARNAME}")

if(NOT WIN32)
    Option(BUILD_PORTABLE "Build portable version with Windows like behavior (see COMPILING.MD)" OFF)
endif()

# [Dasperal] BUILD_VERSION_OVERWRITE variable is used on CI to replace version with git hash

if(COMPILE_DOOM)
    string(REPLACE "." ", " WINDOWS_RC_DOOM_VERSION ${BUILD_DOOM_VERSION})
    if(BUILD_VERSION_OVERWRITE)
        set(BUILD_DOOM_VERSION ${BUILD_VERSION_OVERWRITE})
    endif()
    configure_file(src/doom/resource.rc.in src/doom/resource.rc @ONLY)
    list(APPEND COMPILE_MODULES "doom")
    set(CPACK_PACKAGE_NAME "${PROGRAM_PREFIX}doom")
    set(CPACK_PACKAGE_VERSION ${BUILD_DOOM_VERSION})
    set(GIT_INFO_TAG_PATTERN "[0-9]*")
endif()
if(COMPILE_HERETIC)
    string(REPLACE "." ", " WINDOWS_RC_HERETIC_VERSION ${BUILD_HERETIC_VERSION})
    if(BUILD_VERSION_OVERWRITE)
        set(BUILD_HERETIC_VERSION ${BUILD_VERSION_OVERWRITE})
    endif()
    configure_file(src/heretic/resource.rc.in src/heretic/resource.rc @ONLY)
    list(APPEND COMPILE_MODULES "heretic")
    set(CPACK_PACKAGE_NAME "${PROGRAM_PREFIX}heretic")
    set(CPACK_PACKAGE_VERSION ${BUILD_HERETIC_VERSION})
    set(GIT_INFO_TAG_PATTERN "heretic-[0-9]*")
endif()
if(COMPILE_HEXEN)
    string(REPLACE "." ", " WINDOWS_RC_HEXEN_VERSION ${BUILD_HEXEN_VERSION})
    if(BUILD_VERSION_OVERWRITE)
        set(BUILD_HEXEN_VERSION ${BUILD_VERSION_OVERWRITE})
    endif()
    configure_file(src/hexen/resource.rc.in src/hexen/resource.rc @ONLY)
    list(APPEND COMPILE_MODULES "hexen")
    set(CPACK_PACKAGE_NAME "${PROGRAM_PREFIX}hexen")
    set(CPACK_PACKAGE_VERSION ${BUILD_HEXEN_VERSION})
    set(GIT_INFO_TAG_PATTERN "hexen-[0-9]*")
endif()
if(COMPILE_STRIFE)
    string(REPLACE "." ", " WINDOWS_RC_STRIFE_VERSION ${BUILD_STRIFE_VERSION})
    if(BUILD_VERSION_OVERWRITE)
        set(BUILD_STRIFE_VERSION ${BUILD_VERSION_OVERWRITE})
    endif()
    configure_file(src/strife/resource.rc.in src/strife/resource.rc @ONLY)
    list(APPEND COMPILE_MODULES "strife")
    set(CPACK_PACKAGE_NAME "${PROGRAM_PREFIX}strife")
    set(CPACK_PACKAGE_VERSION ${BUILD_STRIFE_VERSION})
    set(GIT_INFO_TAG_PATTERN "strife-[0-9]*")
endif()

if(COMPILE_SETUP)
    if(COMPILE_DOOM)
        configure_file(src/setup/doom-resource.rc.in src/setup/doom-resource.rc @ONLY)
    endif()
    if(COMPILE_HERETIC)
        configure_file(src/setup/heretic-resource.rc.in src/setup/heretic-resource.rc @ONLY)
    endif()
    if(COMPILE_HEXEN)
        configure_file(src/setup/hexen-resource.rc.in src/setup/hexen-resource.rc @ONLY)
    endif()
    if(COMPILE_STRIFE)
        configure_file(src/setup/strife-resource.rc.in src/setup/strife-resource.rc @ONLY)
    endif()
    configure_file(src/setup/setup-manifest.xml.in src/setup/setup-manifest.xml @ONLY)
endif()

include(RDUtils)
enable_testing()

# Update git_info.h
if(NO_GIT_HASH_SUFFIX)
    unset(GIT_INFO_TAG_PATTERN)
endif()
if(FORCE_GIT_HASH_SUFFIX)
    # Check against an already existing outdated tag so that
    # 'revision_check' always finds new commits and emits a hash suffix
    set(GIT_INFO_TAG_PATTERN "5.3")
endif()
add_custom_target(revision_check
    COMMAND "${CMAKE_COMMAND}" -P "\"${CMAKE_SOURCE_DIR}/cmake/UpdateRevision.cmake\"" "\"git_info.h\"" "\"${GIT_INFO_TAG_PATTERN}\""
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

foreach(SUBDIR textscreen opl pcsound src)
    add_subdirectory("${SUBDIR}")
endforeach()

# CPack
if(WIN32)
    set(CPACK_STRIP_FILES ON)
    set(CPACK_PACKAGING_INSTALL_PREFIX "/")
    set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY OFF)
    set(CPACK_SYSTEM_NAME "windows-${ARCH}")
    include(Cpack)
endif()
