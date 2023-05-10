# FindSDL2_mixer.cmake
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
set(SDL2_MIXER_DIR "${SDL2_MIXER_DIR}" CACHE PATH "Location of SDL2_mixer library directory")

# Use pkg-config to find library locations in *NIX environments.
find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
    pkg_search_module(PC_SDL2_MIXER QUIET SDL2_mixer)
endif()

# Find the include directory.
find_path(SDL2_MIXER_INCLUDE_DIR "SDL_mixer.h"
    PATH_SUFFIXES "include/SDL2" include
    HINTS "${SDL2_MIXER_DIR}" ${PC_SDL2_MIXER_INCLUDE_DIRS}
)

# Find the version.  Taken and modified from CMake's FindSDL.cmake.
if(SDL2_MIXER_INCLUDE_DIR AND EXISTS "${SDL2_MIXER_INCLUDE_DIR}/SDL_mixer.h")
    file(STRINGS "${SDL2_MIXER_INCLUDE_DIR}/SDL_mixer.h" SDL2_MIXER_VERSION_MAJOR_LINE REGEX "^#define[ \t]+SDL_MIXER_MAJOR_VERSION[ \t]+[0-9]+$")
    file(STRINGS "${SDL2_MIXER_INCLUDE_DIR}/SDL_mixer.h" SDL2_MIXER_VERSION_MINOR_LINE REGEX "^#define[ \t]+SDL_MIXER_MINOR_VERSION[ \t]+[0-9]+$")
    file(STRINGS "${SDL2_MIXER_INCLUDE_DIR}/SDL_mixer.h" SDL2_MIXER_VERSION_PATCH_LINE REGEX "^#define[ \t]+SDL_MIXER_PATCHLEVEL[ \t]+[0-9]+$")
    string(REGEX REPLACE "^#define[ \t]+SDL_MIXER_MAJOR_VERSION[ \t]+([0-9]+)$" "\\1" SDL2_MIXER_VERSION_MAJOR "${SDL2_MIXER_VERSION_MAJOR_LINE}")
    string(REGEX REPLACE "^#define[ \t]+SDL_MIXER_MINOR_VERSION[ \t]+([0-9]+)$" "\\1" SDL2_MIXER_VERSION_MINOR "${SDL2_MIXER_VERSION_MINOR_LINE}")
    string(REGEX REPLACE "^#define[ \t]+SDL_MIXER_PATCHLEVEL[ \t]+([0-9]+)$" "\\1" SDL2_MIXER_VERSION_PATCH "${SDL2_MIXER_VERSION_PATCH_LINE}")
    set(SDL2_MIXER_VERSION "${SDL2_MIXER_VERSION_MAJOR}.${SDL2_MIXER_VERSION_MINOR}.${SDL2_MIXER_VERSION_PATCH}")
    unset(SDL2_MIXER_VERSION_MAJOR_LINE)
    unset(SDL2_MIXER_VERSION_MINOR_LINE)
    unset(SDL2_MIXER_VERSION_PATCH_LINE)
    unset(SDL2_MIXER_VERSION_MAJOR)
    unset(SDL2_MIXER_VERSION_MINOR)
    unset(SDL2_MIXER_VERSION_PATCH)
endif()

if(${CMAKE_SIZEOF_VOID_P} STREQUAL 8)
    set(_arch_suffix x64)
else()
    set(_arch_suffix x86)
endif()

# Find DLLs
find_file(SDL2_MIXER_DLL_RELEASE
    NAMES SDL2_mixer.dll
    PATH_SUFFIXES "lib/${_arch_suffix}" bin
    HINTS ${SDL2_MIXER_DIR} "${PC_SDL2_MIXER_PREFIX}"
)
find_file(SDL2_MIXER_DLL_DEBUG
    NAMES SDL2_mixerd.dll
    PATH_SUFFIXES "lib/${_arch_suffix}" "debug/bin" bin
    HINTS ${SDL2_MIXER_DIR} "${PC_SDL2_MIXER_PREFIX}"
)

include(SelectDllConfigurations)
select_dll_configurations(SDL2_MIXER)

if(SDL2_MIXER_DLL)
    set(_sdl2_mixer_shared_release_names "SDL2_mixer.lib" "libSDL2_mixer.dll.a")
    set(_sdl2_mixer_static_release_names "SDL2_mixer-static.lib")
    set(_sdl2_mixer_shared_debug_names "SDL2_mixerd.lib")
    set(_sdl2_mixer_static_debug_names "SDL2_mixer-staticd.lib")
else()
    set(_sdl2_mixer_shared_release_names "")
    set(_sdl2_mixer_static_release_names "SDL2_mixer.lib" "SDL2_mixer-static.lib")
    set(_sdl2_mixer_shared_debug_names "")
    set(_sdl2_mixer_static_debug_names "SDL2_mixerd.lib" "SDL2_mixer-staticd.lib")
endif()

set(_saved_suffixes ${CMAKE_FIND_LIBRARY_SUFFIXES})
# Find the SDL2_mixer dynamic libraries
set(CMAKE_FIND_LIBRARY_SUFFIXES "" ".so" ".dylib" ".dll.a")
find_library(SDL2_MIXER_LIBRARY_RELEASE
    NAMES ${_sdl2_mixer_shared_release_names} SDL2_mixer
    PATH_SUFFIXES "lib/${_arch_suffix}" lib
    HINTS ${SDL2_MIXER_DIR} "${PC_SDL2_MIXER_LIBDIR}"
)
find_library(SDL2_MIXER_LIBRARY_DEBUG
    NAMES ${_sdl2_mixer_shared_debug_names} SDL2_mixerd
    PATH_SUFFIXES "lib/${_arch_suffix}" "debug/lib" lib
    HINTS ${SDL2_MIXER_DIR} "${PC_SDL2_MIXER_LIBDIR}"
)

# Find the SDL2_mixer static libraries
set(CMAKE_FIND_LIBRARY_SUFFIXES "" ".a")
find_library(SDL2_MIXER_STATIC_LIBRARY_RELEASE
    NAMES ${_sdl2_mixer_static_release_names} SDL2_mixer
    PATH_SUFFIXES "lib/${_arch_suffix}" lib
    HINTS ${SDL2_MIXER_DIR} "${PC_SDL2_MIXER_LIBDIR}"
)
find_library(SDL2_MIXER_STATIC_LIBRARY_DEBUG
    NAMES ${_sdl2_mixer_static_debug_names} SDL2_mixerd
    PATH_SUFFIXES "lib/${_arch_suffix}" "debug/lib" lib
    HINTS ${SDL2_MIXER_DIR} "${PC_SDL2_MIXER_LIBDIR}"
)
set(CMAKE_FIND_LIBRARY_SUFFIXES ${_saved_suffixes})

# Find optional DLLs
find_file(SDL2_MIXER_DLL_OPTIONAL_DIR "optional"
    PATH_SUFFIXES "lib/${_arch_suffix}"
    HINTS "${SDL2_MIXER_DIR}"
)
if(SDL2_MIXER_DLL_OPTIONAL_DIR)
    file(GLOB SDL2_MIXER_DLL_OPTIONAL
        "${SDL2_MIXER_DLL_OPTIONAL_DIR}/*.dll")
endif()

unset(_saved_suffixes)
unset(_arch_suffix)

# Select libraries
include(SelectLibraryConfigurations)
select_library_configurations(SDL2_MIXER)
select_library_configurations(SDL2_MIXER_STATIC)

get_flags_from_pkg_config("SHARED" "PC_SDL2_MIXER" "_sdl2_mixer")
get_flags_from_pkg_config("STATIC" "PC_SDL2_MIXER" "_sdl2_mixer_static")

# Link flags for SDL2_mixer static library if PkgConfig not found
if(SDL2_MIXER_STATIC_LIBRARY AND NOT PC_SDL2_MIXER_FOUND)
    if(NOT SDL2_MIXER_LIBRARY)
        set(SDL2_MIXER_STATIC_LINK_LIBRARIES "" CACHE STRING "Additional libraries to link to SDL2_mixer-static.")
        set(SDL2_MIXER_STATIC_LINK_DIRECTORIES "" CACHE PATH "Additional directories to search libraries in for SDL2_mixer-static.")
        set(_sdl2_mixer_static_link_libraries ${SDL2_MIXER_STATIC_LINK_LIBRARIES})
        set(_sdl2_mixer_static_link_directories ${SDL2_MIXER_STATIC_LINK_DIRECTORIES})
        if(NOT _sdl2_mixer_static_link_libraries)
            message(WARNING
                "pkg-config is unavailable and only a static version of SDL2_mixer was found.\n"
                "Link failures are to be expected.\n"
                "Set `SDL2_MIXER_STATIC_LINK_LIBRARIES` to a list of libraries SDL2_mixer depends on.\n"
                "Set `SDL2_MIXER_STATIC_LINK_DIRECTORIES` to a list of directories to search for libraries in."
            )
        endif()
    endif()
endif()

if(SDL2_MIXER_LIBRARY)
    set(_SDL2_MIXER_LIBRARY "${SDL2_MIXER_LIBRARY}")
else()
    set(_SDL2_MIXER_LIBRARY "${SDL2_MIXER_STATIC_LIBRARY}")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SDL2_mixer
    FOUND_VAR SDL2_MIXER_FOUND
    REQUIRED_VARS SDL2_MIXER_INCLUDE_DIR _SDL2_MIXER_LIBRARY
    VERSION_VAR SDL2_MIXER_VERSION
)
unset(_SDL2_MIXER_LIBRARY)

# Cleanup macro
macro(_cleanup)
    unset(_sdl2_mixer_shared_release_names)
    unset(_sdl2_mixer_static_release_names)
    unset(_sdl2_mixer_shared_debug_names)
    unset(_sdl2_mixer_static_debug_names)
    unset(_sdl2_mixer_compile_options)
    unset(_sdl2_mixer_link_libraries)
    unset(_sdl2_mixer_link_directories)
    unset(_sdl2_mixer_link_options)
    unset(_sdl2_mixer_static_compile_options)
    unset(_sdl2_mixer_static_link_libraries)
    unset(_sdl2_mixer_static_link_directories)
    unset(_sdl2_mixer_static_link_options)
    mark_as_advanced(FORCE
        SDL2_MIXER_INCLUDE_DIR
        SDL2_MIXER_DLL_RELEASE
        SDL2_MIXER_DLL_DEBUG
        SDL2_MIXER_DLL_OPTIONAL_DIR
        SDL2_MIXER_LIBRARY_RELEASE
        SDL2_MIXER_LIBRARY_DEBUG
        SDL2_MIXER_STATIC_LIBRARY_RELEASE
        SDL2_MIXER_STATIC_LIBRARY_DEBUG
        )
endmacro()

if(NOT SDL2_MIXER_FOUND)
    _cleanup()
    return()
endif()

# SDL2_mixer imported target
if(SDL2_MIXER_LIBRARY)
    add_library(SDL2_mixer::SDL2_mixer SHARED IMPORTED)
    set_target_properties(SDL2_mixer::SDL2_mixer PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${SDL2_MIXER_INCLUDE_DIR}"
        INTERFACE_COMPILE_OPTIONS "${_sdl2_mixer_compile_options}"
        INTERFACE_LINK_LIBRARIES "${_sdl2_mixer_link_libraries}"
        INTERFACE_LINK_DIRECTORIES "${_sdl2_mixer_link_directories}"
        INTERFACE_LINK_OPTIONS "${_sdl2_mixer_link_options}"
    )
    if(SDL2_MIXER_DLL)
        set_target_properties(SDL2_mixer::SDL2_mixer PROPERTIES
            IMPORTED_LOCATION "${SDL2_MIXER_DLL}"
            IMPORTED_IMPLIB "${SDL2_MIXER_LIBRARY}"
        )
    else()
        set_target_properties(SDL2_mixer::SDL2_mixer PROPERTIES
            IMPORTED_LOCATION "${SDL2_MIXER_LIBRARY}"
        )
    endif()
    if(SDL2_MIXER_LIBRARY_RELEASE)
        set_property(TARGET SDL2_mixer::SDL2_mixer APPEND PROPERTY
            IMPORTED_CONFIGURATIONS RELEASE
        )
        set_target_properties(SDL2_mixer::SDL2_mixer PROPERTIES
            MAP_IMPORTED_CONFIG_MINSIZEREL RELEASE
            MAP_IMPORTED_CONFIG_RELWITHDEBINFO RELEASE
        )
        if(SDL2_MIXER_DLL_RELEASE)
            set_target_properties(SDL2_mixer::SDL2_mixer PROPERTIES
                IMPORTED_LOCATION_RELEASE "${SDL2_MIXER_DLL_RELEASE}"
                IMPORTED_IMPLIB_RELEASE "${SDL2_MIXER_LIBRARY_RELEASE}"
            )
        else()
            set_target_properties(SDL2_mixer::SDL2_mixer PROPERTIES
                IMPORTED_LOCATION_RELEASE "${SDL2_MIXER_LIBRARY_RELEASE}"
            )
        endif()
    endif()
    if(SDL2_MIXER_LIBRARY_DEBUG)
        set_property(TARGET SDL2_mixer::SDL2_mixer APPEND PROPERTY
            IMPORTED_CONFIGURATIONS DEBUG
        )
        if(SDL2_MIXER_DLL_DEBUG)
            set_target_properties(SDL2_mixer::SDL2_mixer PROPERTIES
                IMPORTED_LOCATION_DEBUG "${SDL2_MIXER_DLL_DEBUG}"
                IMPORTED_IMPLIB_DEBUG "${SDL2_MIXER_LIBRARY_DEBUG}"
            )
        else()
            set_target_properties(SDL2_mixer::SDL2_mixer PROPERTIES
                IMPORTED_LOCATION_DEBUG "${SDL2_MIXER_LIBRARY_DEBUG}"
            )
        endif()
    endif()
    # Optional Dlls
    if(SDL2_MIXER_DLL_OPTIONAL)
        # OPTIONAL_DLLS - list of optional dlls to install with this target
        set_target_properties(SDL2_mixer::SDL2_mixer PROPERTIES
             OPTIONAL_DLLS "${SDL2_MIXER_DLL_OPTIONAL}"
        )
    endif()
endif()

# SDL2_mixer-static imported target
if(SDL2_MIXER_STATIC_LIBRARY)
    add_library(SDL2_mixer::SDL2_mixer-static STATIC IMPORTED)
    set_target_properties(SDL2_mixer::SDL2_mixer-static PROPERTIES
        IMPORTED_LOCATION "${SDL2_MIXER_STATIC_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${SDL2_MIXER_INCLUDE_DIR}"
        INTERFACE_COMPILE_OPTIONS "${_sdl2_mixer_static_compile_options}"
        INTERFACE_LINK_LIBRARIES "${_sdl2_mixer_static_link_libraries}"
        INTERFACE_LINK_DIRECTORIES "${_sdl2_mixer_static_link_directories}"
        INTERFACE_LINK_OPTIONS "${_sdl2_mixer_static_link_options}"
    )
    if(SDL2_MIXER_STATIC_LIBRARY_RELEASE)
        set_property(TARGET SDL2_mixer::SDL2_mixer-static APPEND PROPERTY
            IMPORTED_CONFIGURATIONS RELEASE
        )
        set_target_properties(SDL2_mixer::SDL2_mixer-static PROPERTIES
            IMPORTED_LOCATION_RELEASE "${SDL2_MIXER_STATIC_LIBRARY_RELEASE}"
            MAP_IMPORTED_CONFIG_MINSIZEREL RELEASE
            MAP_IMPORTED_CONFIG_RELWITHDEBINFO RELEASE
        )
    endif()
    if(SDL2_MIXER_STATIC_LIBRARY_DEBUG)
        set_property(TARGET SDL2_mixer::SDL2_mixer-static APPEND PROPERTY
            IMPORTED_CONFIGURATIONS DEBUG
        )
        set_target_properties(SDL2_mixer::SDL2_mixer-static PROPERTIES
            IMPORTED_LOCATION_DEBUG "${SDL2_MIXER_STATIC_LIBRARY_DEBUG}"
        )
    endif()
    if(NOT TARGET SDL2_mixer::SDL2_mixer)
        add_library(SDL2_mixer::SDL2_mixer ALIAS SDL2_mixer::SDL2_mixer-static)
    endif()
endif()

_cleanup()
