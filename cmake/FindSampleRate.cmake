# FindSampleRate.cmake
#
# Copyright (c) 2018, Alex Mayfield <alexmax2742@gmail.com>
# All rights reserved.
# 
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#     * Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above copyright
#       notice, this list of conditions and the following disclaimer in the
#       documentation and/or other materials provided with the distribution.
#     * Neither the name of the <organization> nor the
#       names of its contributors may be used to endorse or promote products
#       derived from this software without specific prior written permission.
# 
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
# DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
# ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
# Currently works with the following generators:
# - Unix Makefiles (Linux, MSYS2)
# - Ninja (Linux, MSYS2)
# - Visual Studio

# Cache variable that allows you to point CMake at a directory containing
# an extracted development library.
set(SAMPLERATE_DIR "${SAMPLERATE_DIR}" CACHE PATH "Location of Samplerate library directory")

# Use pkg-config to find library locations in *NIX environments.
find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
    pkg_search_module(PC_SAMPLERATE QUIET samplerate)
endif()

# Find the include directory.
find_path(SAMPLERATE_INCLUDE_DIR "samplerate.h"
    PATH_SUFFIXES include
    HINTS "${SAMPLERATE_DIR}" ${PC_SAMPLERATE_INCLUDE_DIRS})

# Find the version.  I don't know if there is a correct way to find this on
# Windows - the config.h in the tarball is wrong for 0.1.19.
if(PC_SAMPLERATE_VERSION)
    set(SAMPLERATE_VERSION "${PC_SAMPLERATE_VERSION}")
endif()

# Find DLLs
find_file(SAMPLERATE_DLL_RELEASE
    NAMES samplerate.dll samplerate-0.dll libsamplerate.dll libsamplerate-0.dll
    PATH_SUFFIXES bin
    HINTS ${SAMPLERATE_DIR} "${PC_SAMPLERATE_PREFIX}"
)
find_file(SAMPLERATE_DLL_DEBUG
    NAMES samplerate.dll samplerate-0.dll libsamplerate.dll libsamplerate-0.dll
    PATH_SUFFIXES "debug/bin" bin
    HINTS ${SAMPLERATE_DIR} "${PC_SAMPLERATE_PREFIX}"
)

include(SelectDllConfigurations)
select_dll_configurations(SAMPLERATE)

if(SAMPLERATE_DLL)
    set(_samplerate_shared_release_names "samplerate.lib" "libsamplerate.dll.a")
    set(_samplerate_static_release_names "samplerate-static.lib")
    set(_samplerate_shared_debug_names "samplerate.lib")
    set(_samplerate_static_debug_names "samplerate-staticd.lib")
else()
    set(_samplerate_shared_release_names "")
    set(_samplerate_static_release_names "samplerate.lib" "samplerate-static.lib")
    set(_samplerate_shared_debug_names "")
    set(_samplerate_static_debug_names "samplerate.lib" "samplerate-staticd.lib")
endif()

set(_saved_suffixes ${CMAKE_FIND_LIBRARY_SUFFIXES})
# Find the samplerate dynamic libraries
set(CMAKE_FIND_LIBRARY_SUFFIXES "" ".so" ".dylib" ".dll.a")
find_library(SAMPLERATE_LIBRARY_RELEASE
    NAMES ${_samplerate_shared_release_names} samplerate
    PATH_SUFFIXES lib
    HINTS ${SAMPLERATE_DIR} "${PC_SAMPLERATE_LIBRARY_DIRS}"
)
find_library(SAMPLERATE_LIBRARY_DEBUG
    NAMES ${_samplerate_shared_debug_names} samplerate
    PATH_SUFFIXES "debug/lib" lib
    HINTS ${SAMPLERATE_DIR} "${PC_SAMPLERATE_LIBRARY_DIRS}"
)

# Find the samplerate static libraries
set(CMAKE_FIND_LIBRARY_SUFFIXES "" ".a")
find_library(SAMPLERATE_STATIC_LIBRARY_RELEASE
    NAMES ${_samplerate_static_release_names} samplerate
    PATH_SUFFIXES lib
    HINTS ${SAMPLERATE_DIR} "${PC_SAMPLERATE_LIBRARY_DIRS}"
)
find_library(SAMPLERATE_STATIC_LIBRARY_DEBUG
    NAMES ${_samplerate_static_debug_names} samplerate
    PATH_SUFFIXES "debug/lib" lib
    HINTS ${SAMPLERATE_DIR} "${PC_SAMPLERATE_LIBRARY_DIRS}"
)
set(CMAKE_FIND_LIBRARY_SUFFIXES ${_saved_suffixes})

unset(_saved_suffixes)

# Select libraries
include(SelectLibraryConfigurations)
select_library_configurations(SAMPLERATE)
select_library_configurations(SAMPLERATE_STATIC)

get_flags_from_pkg_config("SHARED" "PC_SAMPLERATE" "_samplerate")
get_flags_from_pkg_config("STATIC" "PC_SAMPLERATE" "_samplerate_static")

# Link flags for samplerate static library if PkgConfig not found
if(SAMPLERATE_STATIC_LIBRARY AND NOT PC_SAMPLERATE_FOUND)
    if(NOT SAMPLERATE_LIBRARY)
        set(SAMPLERATE_STATIC_LINK_LIBRARIES "" CACHE STRING "Additional libraries to link to samplerate-static.")
        set(SAMPLERATE_STATIC_LINK_DIRECTORIES "" CACHE PATH "Additional directories to search libraries in for samplerate-static.")
        set(_samplerate_static_link_libraries ${SAMPLERATE_STATIC_LINK_LIBRARIES})
        set(_samplerate_static_link_directories ${SAMPLERATE_STATIC_LINK_DIRECTORIES})
        if(NOT _samplerate_static_link_libraries)
            message(WARNING
                "pkg-config is unavailable and only a static version of samplerate was found.\n"
                "Link failures are to be expected.\n"
                "Set `SAMPLERATE_STATIC_LINK_LIBRARIES` to a list of libraries samplerate depends on.\n"
                "Set `SAMPLERATE_STATIC_LINK_DIRECTORIES` to a list of directories to search for libraries in."
            )
        endif()
    endif()
endif()

if(SAMPLERATE_LIBRARY)
    set(_SAMPLERATE_LIBRARY "${SAMPLERATE_LIBRARY}")
else()
    set(_SAMPLERATE_LIBRARY "${SAMPLERATE_STATIC_LIBRARY}")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SampleRate
    FOUND_VAR SAMPLERATE_FOUND
    REQUIRED_VARS SAMPLERATE_INCLUDE_DIR _SAMPLERATE_LIBRARY
    VERSION_VAR SAMPLERATE_VERSION
)
unset(_SDL2_LIBRARY)

# Cleanup macro
macro(_cleanup)
    unset(_samplerate_shared_release_names)
    unset(_samplerate_static_release_names)
    unset(_samplerate_shared_debug_names)
    unset(_samplerate_static_debug_names)
    unset(_samplerate_compile_options)
    unset(_samplerate_link_libraries)
    unset(_samplerate_link_directories)
    unset(_samplerate_link_options)
    unset(_samplerate_static_compile_options)
    unset(_samplerate_static_link_libraries)
    unset(_samplerate_static_link_directories)
    unset(_samplerate_static_link_options)
    mark_as_advanced(FORCE
        SAMPLERATE_INCLUDE_DIR
        SAMPLERATE_DLL_RELEASE
        SAMPLERATE_DLL_DEBUG
        SAMPLERATE_LIBRARY_RELEASE
        SAMPLERATE_LIBRARY_DEBUG
        SAMPLERATE_STATIC_LIBRARY_RELEASE
        SAMPLERATE_STATIC_LIBRARY_DEBUG
        )
endmacro()

if(NOT SAMPLERATE_FOUND)
    _cleanup()
    return()
endif()

# samplerate imported target
if(SAMPLERATE_LIBRARY)
    add_library(SampleRate::samplerate SHARED IMPORTED)
    set_target_properties(SampleRate::samplerate PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${SAMPLERATE_INCLUDE_DIR}"
        INTERFACE_COMPILE_OPTIONS "${_samplerate_compile_options}"
        INTERFACE_LINK_LIBRARIES "${_samplerate_link_libraries}"
        INTERFACE_LINK_DIRECTORIES "${_samplerate_link_directories}"
        INTERFACE_LINK_OPTIONS "${_samplerate_link_options}"
    )
    if(SAMPLERATE_DLL)
        set_target_properties(SampleRate::samplerate PROPERTIES
            IMPORTED_LOCATION "${SAMPLERATE_DLL}"
            IMPORTED_IMPLIB "${SAMPLERATE_LIBRARY}"
        )
    else()
        set_target_properties(SampleRate::samplerate PROPERTIES
            IMPORTED_LOCATION "${SAMPLERATE_LIBRARY}"
        )
    endif()
    if(SAMPLERATE_LIBRARY_RELEASE)
        set_property(TARGET SampleRate::samplerate APPEND PROPERTY
            IMPORTED_CONFIGURATIONS RELEASE
        )
        set_target_properties(SampleRate::samplerate PROPERTIES
            MAP_IMPORTED_CONFIG_MINSIZEREL RELEASE
            MAP_IMPORTED_CONFIG_RELWITHDEBINFO RELEASE
        )
        if(SAMPLERATE_DLL_RELEASE)
            set_target_properties(SampleRate::samplerate PROPERTIES
                IMPORTED_LOCATION_RELEASE "${SAMPLERATE_DLL_RELEASE}"
                IMPORTED_IMPLIB_RELEASE "${SAMPLERATE_LIBRARY_RELEASE}"
            )
        else()
            set_target_properties(SampleRate::samplerate PROPERTIES
                IMPORTED_LOCATION_RELEASE "${SAMPLERATE_LIBRARY_RELEASE}"
            )
        endif()
    endif()
    if(SAMPLERATE_LIBRARY_DEBUG)
        set_property(TARGET SampleRate::samplerate APPEND PROPERTY
            IMPORTED_CONFIGURATIONS DEBUG
        )
        if(SAMPLERATE_DLL_DEBUG)
            set_target_properties(SampleRate::samplerate PROPERTIES
                IMPORTED_LOCATION_DEBUG "${SAMPLERATE_DLL_DEBUG}"
                IMPORTED_IMPLIB_DEBUG "${SAMPLERATE_LIBRARY_DEBUG}"
            )
        else()
            set_target_properties(SampleRate::samplerate PROPERTIES
                IMPORTED_LOCATION_DEBUG "${SAMPLERATE_LIBRARY_DEBUG}"
            )
        endif()
    endif()
endif()

# samplerate-static imported target
if(SAMPLERATE_STATIC_LIBRARY)
    add_library(SampleRate::samplerate-static STATIC IMPORTED)
    set_target_properties(SampleRate::samplerate-static PROPERTIES
        IMPORTED_LOCATION "${SAMPLERATE_STATIC_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${SAMPLERATE_INCLUDE_DIR}"
        INTERFACE_COMPILE_OPTIONS "${_samplerate_static_compile_options}"
        INTERFACE_LINK_LIBRARIES "${_samplerate_static_link_libraries}"
        INTERFACE_LINK_DIRECTORIES "${_samplerate_static_link_directories}"
        INTERFACE_LINK_OPTIONS "${_samplerate_static_link_options}"
    )
    if(SAMPLERATE_STATIC_LIBRARY_RELEASE)
        set_property(TARGET SampleRate::samplerate-static APPEND PROPERTY
            IMPORTED_CONFIGURATIONS RELEASE
        )
        set_target_properties(SampleRate::samplerate-static PROPERTIES
            IMPORTED_LOCATION_RELEASE "${SAMPLERATE_STATIC_LIBRARY_RELEASE}"
            MAP_IMPORTED_CONFIG_MINSIZEREL RELEASE
            MAP_IMPORTED_CONFIG_RELWITHDEBINFO RELEASE
        )
    endif()
    if(SAMPLERATE_STATIC_LIBRARY_DEBUG)
        set_property(TARGET SampleRate::samplerate-static APPEND PROPERTY
            IMPORTED_CONFIGURATIONS DEBUG
        )
        set_target_properties(SampleRate::samplerate-static PROPERTIES
            IMPORTED_LOCATION_DEBUG "${SAMPLERATE_STATIC_LIBRARY_DEBUG}"
        )
    endif()
    if(NOT TARGET SampleRate::samplerate)
        add_library(SampleRate::samplerate ALIAS SampleRate::samplerate-static)
    endif()
endif()

_cleanup()
