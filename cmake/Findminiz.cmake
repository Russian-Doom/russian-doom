# Findminiz.cmake
#
# Copyright (c) 2022-2023, Murin Leonid (Dasperal) <Dasperal1@gmail.com>
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


# Cache variable that allows you to point CMake at a directory containing
# an extracted development library.
set(MINIZ_DIR "${MINIZ_DIR}" CACHE PATH "Location of Miniz library directory")

# Use pkg-config to find library locations in *NIX environments
find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
    pkg_search_module(PC_MINIZ QUIET miniz)
endif()

# Find the include directory
find_path(MINIZ_INCLUDE_DIR "miniz.h"
    PATH_SUFFIXES "include/miniz" include
    HINTS "${MINIZ_DIR}" ${PC_MINIZ_INCLUDE_DIRS}
)

# Find the version
if(MINIZ_INCLUDE_DIR AND EXISTS "${MINIZ_INCLUDE_DIR}/miniz.h")
    file(STRINGS "${MINIZ_INCLUDE_DIR}/miniz.h" MINIZ_VERSION_MAJOR_LINE REGEX "^#define[ \t]+MZ_VER_MAJOR[ \t]+[0-9]+$")
    file(STRINGS "${MINIZ_INCLUDE_DIR}/miniz.h" MINIZ_VERSION_MINOR_LINE REGEX "^#define[ \t]+MZ_VER_MINOR[ \t]+[0-9]+$")
    file(STRINGS "${MINIZ_INCLUDE_DIR}/miniz.h" MINIZ_VERSION_PATCH_LINE REGEX "^#define[ \t]+MZ_VER_REVISION[ \t]+[0-9]+$")
    string(REGEX REPLACE "^#define[ \t]+MZ_VER_MAJOR[ \t]+([0-9]+)$" "\\1" MINIZ_VERSION_MAJOR "${MINIZ_VERSION_MAJOR_LINE}")
    string(REGEX REPLACE "^#define[ \t]+MZ_VER_MINOR[ \t]+([0-9]+)$" "\\1" MINIZ_VERSION_MINOR "${MINIZ_VERSION_MINOR_LINE}")
    string(REGEX REPLACE "^#define[ \t]+MZ_VER_REVISION[ \t]+([0-9]+)$" "\\1" MINIZ_VERSION_PATCH "${MINIZ_VERSION_PATCH_LINE}")
    math(EXPR MINIZ_VERSION_MAJOR "${MINIZ_VERSION_MAJOR} - 8" OUTPUT_FORMAT DECIMAL)
    set(MINIZ_VERSION "${MINIZ_VERSION_MAJOR}.${MINIZ_VERSION_MINOR}.${MINIZ_VERSION_PATCH}")
    unset(MINIZ_VERSION_MAJOR_LINE)
    unset(MINIZ_VERSION_MINOR_LINE)
    unset(MINIZ_VERSION_PATCH_LINE)
    unset(MINIZ_VERSION_MINOR)
    unset(MINIZ_VERSION_PATCH)
endif()

# Find DLLs
find_file(MINIZ_DLL_RELEASE
    NAMES miniz.dll
    PATH_SUFFIXES bin
    HINTS ${MINIZ_DIR} "${PC_MINIZ_PREFIX}"
)
find_file(MINIZ_DLL_DEBUG
    NAMES miniz.dll
    PATH_SUFFIXES "debug/bin" bin
    HINTS ${MINIZ_DIR} "${PC_MINIZ_PREFIX}"
)

include(SelectDllConfigurations)
select_dll_configurations(MINIZ)

if(MINIZ_DLL)
    set(_miniz_shared_release_names "miniz.lib" "libminiz.dll.a")
    set(_miniz_static_release_names "miniz-static.lib")
    set(_miniz_shared_debug_names "miniz.lib")
    set(_miniz_static_debug_names "miniz-static.lib")
else()
    set(_miniz_shared_release_names "")
    set(_miniz_static_release_names "miniz.lib" "miniz-static.lib")
    set(_miniz_shared_debug_names "")
    set(_miniz_static_debug_names "miniz.lib" "miniz-static.lib")
endif()

set(_saved_suffixes ${CMAKE_FIND_LIBRARY_SUFFIXES})
# Find the samplerate dynamic libraries
set(CMAKE_FIND_LIBRARY_SUFFIXES "" ".so" ".dylib" ".dll.a")
find_library(MINIZ_LIBRARY_RELEASE
    NAMES ${_miniz_shared_release_names} miniz
    PATH_SUFFIXES lib
    HINTS ${MINIZ_DIR} "${PC_MINIZ_LIBRARY_DIRS}"
    )
find_library(MINIZ_LIBRARY_DEBUG
    NAMES ${_miniz_shared_debug_names} miniz
    PATH_SUFFIXES "debug/lib" lib
    HINTS ${MINIZ_DIR} "${PC_MINIZ_LIBRARY_DIRS}"
    )

# Find the samplerate static libraries
set(CMAKE_FIND_LIBRARY_SUFFIXES "" ".a")
find_library(MINIZ_STATIC_LIBRARY_RELEASE
    NAMES ${_miniz_static_release_names} miniz
    PATH_SUFFIXES lib
    HINTS ${MINIZ_DIR} "${PC_MINIZ_LIBRARY_DIRS}"
    )
find_library(MINIZ_STATIC_LIBRARY_DEBUG
    NAMES ${_miniz_static_debug_names} miniz
    PATH_SUFFIXES "debug/lib" lib
    HINTS ${MINIZ_DIR} "${PC_MINIZ_LIBRARY_DIRS}"
    )
set(CMAKE_FIND_LIBRARY_SUFFIXES ${_saved_suffixes})

unset(_saved_suffixes)

# Select libraries
include(SelectLibraryConfigurations)
select_library_configurations(MINIZ)
select_library_configurations(MINIZ_STATIC)

get_flags_from_pkg_config("SHARED" "PC_MINIZ" "_miniz")
get_flags_from_pkg_config("STATIC" "PC_MINIZ" "_miniz_static")

# Link flags for samplerate static library if PkgConfig not found
if(MINIZ_STATIC_LIBRARY AND NOT PC_MINIZ_FOUND)
    if(NOT MINIZ_LIBRARY)
        set(MINIZ_STATIC_LINK_LIBRARIES "" CACHE STRING "Additional libraries to link to miniz-static.")
        set(MINIZ_STATIC_LINK_DIRECTORIES "" CACHE PATH "Additional directories to search libraries in for miniz-static.")
        set(_miniz_static_link_libraries ${MINIZ_STATIC_LINK_LIBRARIES})
        set(_miniz_static_link_directories ${MINIZ_STATIC_LINK_DIRECTORIES})
        if(NOT _miniz_static_link_libraries)
            message(WARNING
                "pkg-config is unavailable and only a static version of miniz was found.\n"
                "Link failures are to be expected.\n"
                "Set `MINIZ_STATIC_LINK_LIBRARIES` to a list of libraries miniz depends on.\n"
                "Set `MINIZ_STATIC_LINK_DIRECTORIES` to a list of directories to search for libraries in."
            )
        endif()
    endif()
endif()

if(MINIZ_LIBRARY)
    set(_MINIZ_LIBRARY "${MINIZ_LIBRARY}")
else()
    set(_MINIZ_LIBRARY "${MINIZ_STATIC_LIBRARY}")
endif()

# No libraries, may be amalgamated sources? Find .c file
if(NOT _MINIZ_LIBRARY)
    find_file(MINIZ_C_FILE "miniz.c"
        HINTS "${MINIZ_DIR}"
    )
    set(_MINIZ_LIBRARY "${MINIZ_C_FILE}")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(miniz
    FOUND_VAR MINIZ_FOUND
    REQUIRED_VARS MINIZ_INCLUDE_DIR _MINIZ_LIBRARY
    VERSION_VAR MINIZ_VERSION
)
unset(_MINIZ_LIBRARY)

# Cleanup macro
macro(_cleanup)
    unset(MINIZ_VERSION_MAJOR)
    unset(_miniz_shared_release_names)
    unset(_miniz_static_release_names)
    unset(_miniz_shared_debug_names)
    unset(_miniz_static_debug_names)
    unset(_miniz_compile_options)
    unset(_miniz_link_libraries)
    unset(_miniz_link_directories)
    unset(_miniz_link_options)
    unset(_miniz_static_compile_options)
    unset(_miniz_static_link_libraries)
    unset(_miniz_static_link_directories)
    unset(_miniz_static_link_options)
    mark_as_advanced(FORCE
        MINIZ_INCLUDE_DIR
        MINIZ_DLL_RELEASE
        MINIZ_DLL_DEBUG
        MINIZ_LIBRARY_RELEASE
        MINIZ_LIBRARY_DEBUG
        MINIZ_STATIC_LIBRARY_RELEASE
        MINIZ_STATIC_LIBRARY_DEBUG
        MINIZ_C_FILE
        )
endmacro()

if(NOT MINIZ_FOUND)
    _cleanup()
    return()
endif()

# miniz imported target
if(MINIZ_LIBRARY)
    add_library(miniz::miniz SHARED IMPORTED)
    set_target_properties(miniz::miniz PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${MINIZ_INCLUDE_DIR}"
        INTERFACE_COMPILE_OPTIONS "${PC_MINIZ_CFLAGS_OTHER}"
        INTERFACE_COMPILE_OPTIONS "${_miniz_compile_options}"
        INTERFACE_LINK_LIBRARIES "${_miniz_link_libraries}"
        INTERFACE_LINK_DIRECTORIES "${_miniz_link_directories}"
        INTERFACE_LINK_OPTIONS "${_miniz_link_options}"
        INTERFACE_miniz_MAJOR_VERSION "${MINIZ_VERSION_MAJOR}"
        COMPATIBLE_INTERFACE_STRING "miniz_MAJOR_VERSION"
    )
    if(MINIZ_DLL)
        set_target_properties(miniz::miniz PROPERTIES
            IMPORTED_LOCATION "${MINIZ_DLL}"
            IMPORTED_IMPLIB "${MINIZ_LIBRARY}"
        )
    else()
        set_target_properties(miniz::miniz PROPERTIES
            IMPORTED_LOCATION "${MINIZ_LIBRARY}"
        )
    endif()
    if(MINIZ_LIBRARY_RELEASE)
        set_property(TARGET miniz::miniz APPEND PROPERTY
            IMPORTED_CONFIGURATIONS RELEASE
        )
        set_target_properties(miniz::miniz PROPERTIES
            MAP_IMPORTED_CONFIG_MINSIZEREL RELEASE
            MAP_IMPORTED_CONFIG_RELWITHDEBINFO RELEASE
        )
        if(MINIZ_DLL_RELEASE)
            set_target_properties(miniz::miniz PROPERTIES
                IMPORTED_LOCATION_RELEASE "${MINIZ_DLL_RELEASE}"
                IMPORTED_IMPLIB_RELEASE "${MINIZ_LIBRARY_RELEASE}"
            )
        else()
            set_target_properties(miniz::miniz PROPERTIES
                IMPORTED_LOCATION_RELEASE "${MINIZ_LIBRARY_RELEASE}"
            )
        endif()
    endif()
    if(MINIZ_LIBRARY_DEBUG)
        set_property(TARGET miniz::miniz APPEND PROPERTY
            IMPORTED_CONFIGURATIONS DEBUG
        )
        if(MINIZ_DLL_DEBUG)
            set_target_properties(miniz::miniz PROPERTIES
                IMPORTED_LOCATION_DEBUG "${MINIZ_DLL_DEBUG}"
                IMPORTED_IMPLIB_DEBUG "${MINIZ_LIBRARY_DEBUG}"
            )
        else()
            set_target_properties(miniz::miniz PROPERTIES
                IMPORTED_LOCATION_DEBUG "${MINIZ_LIBRARY_DEBUG}"
            )
        endif()
    endif()
endif()

# miniz-static imported target
if(MINIZ_STATIC_LIBRARY)
    add_library(miniz::miniz-static STATIC IMPORTED)
    set_target_properties(miniz::miniz-static PROPERTIES
        IMPORTED_LOCATION "${MINIZ_STATIC_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${SAMPLERATE_INCLUDE_DIR}"
        INTERFACE_COMPILE_OPTIONS "${_miniz_static_compile_options}"
        INTERFACE_LINK_LIBRARIES "${_miniz_static_link_libraries}"
        INTERFACE_LINK_DIRECTORIES "${_miniz_static_link_directories}"
        INTERFACE_LINK_OPTIONS "${_miniz_static_link_options}"
        INTERFACE_miniz_MAJOR_VERSION "${MINIZ_VERSION_MAJOR}"
        COMPATIBLE_INTERFACE_STRING "miniz_MAJOR_VERSION"
    )
    if(MINIZ_STATIC_LIBRARY_RELEASE)
        set_property(TARGET miniz::miniz-static APPEND PROPERTY
            IMPORTED_CONFIGURATIONS RELEASE
        )
        set_target_properties(miniz::miniz-static PROPERTIES
            IMPORTED_LOCATION_RELEASE "${MINIZ_STATIC_LIBRARY_RELEASE}"
            MAP_IMPORTED_CONFIG_MINSIZEREL RELEASE
            MAP_IMPORTED_CONFIG_RELWITHDEBINFO RELEASE
        )
    endif()
    if(MINIZ_STATIC_LIBRARY_DEBUG)
        set_property(TARGET miniz::miniz-static APPEND PROPERTY
            IMPORTED_CONFIGURATIONS DEBUG
        )
        set_target_properties(miniz::miniz-static PROPERTIES
            IMPORTED_LOCATION_DEBUG "${MINIZ_STATIC_LIBRARY_DEBUG}"
        )
    endif()
    if(NOT TARGET miniz::miniz)
        add_library(miniz::miniz ALIAS miniz::miniz-static)
    endif()
endif()

# Have only amalgamated sources, so build static library from them
if(MINIZ_C_FILE) # Implicit "AND NOT MINIZ_LIBRARY AND NOT MINIZ_STATIC_LIBRARY"
    add_library(miniz STATIC EXCLUDE_FROM_ALL
        ${MINIZ_C_FILE}     "${MINIZ_INCLUDE_DIR}/miniz.h"
    )
    target_compile_definitions(miniz
        PRIVATE $<$<C_COMPILER_ID:GNU>:_GNU_SOURCE>
    )
    set_target_properties(miniz PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${MINIZ_INCLUDE_DIR}"
        INTERFACE_COMPILE_OPTIONS "${_miniz_static_compile_options}"
        INTERFACE_LINK_LIBRARIES "${_miniz_static_link_libraries}"
        INTERFACE_LINK_DIRECTORIES "${_miniz_static_link_directories}"
        INTERFACE_LINK_OPTIONS "${_miniz_static_link_options}"
        INTERFACE_miniz_MAJOR_VERSION "${MINIZ_VERSION_MAJOR}"
        COMPATIBLE_INTERFACE_STRING "miniz_MAJOR_VERSION"
        C_STANDARD 90
        C_STANDARD_REQUIRED ON
    )
    add_library(miniz::miniz ALIAS miniz)
endif()

_cleanup()
