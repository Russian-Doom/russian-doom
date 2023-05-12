# FindSDL2.cmake
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
set(SDL2_DIR "${SDL2_DIR}" CACHE PATH "Location of SDL2 library directory")

# Use pkg-config to find library locations in *NIX environments.
find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
    pkg_search_module(PC_SDL2 QUIET sdl2)
endif()

# Find the include directory.
find_path(SDL2_INCLUDE_DIR "SDL.h"
    PATH_SUFFIXES "include/SDL2" include
    HINTS "${SDL2_DIR}" ${PC_SDL2_INCLUDE_DIRS}
)

# Find the version.  Taken and modified from CMake's FindSDL.cmake.
if(SDL2_INCLUDE_DIR AND EXISTS "${SDL2_INCLUDE_DIR}/SDL_version.h")
    file(STRINGS "${SDL2_INCLUDE_DIR}/SDL_version.h" SDL2_VERSION_MAJOR_LINE REGEX "^#define[ \t]+SDL_MAJOR_VERSION[ \t]+[0-9]+$")
    file(STRINGS "${SDL2_INCLUDE_DIR}/SDL_version.h" SDL2_VERSION_MINOR_LINE REGEX "^#define[ \t]+SDL_MINOR_VERSION[ \t]+[0-9]+$")
    file(STRINGS "${SDL2_INCLUDE_DIR}/SDL_version.h" SDL2_VERSION_PATCH_LINE REGEX "^#define[ \t]+SDL_PATCHLEVEL[ \t]+[0-9]+$")
    string(REGEX REPLACE "^#define[ \t]+SDL_MAJOR_VERSION[ \t]+([0-9]+)$" "\\1" SDL2_VERSION_MAJOR "${SDL2_VERSION_MAJOR_LINE}")
    string(REGEX REPLACE "^#define[ \t]+SDL_MINOR_VERSION[ \t]+([0-9]+)$" "\\1" SDL2_VERSION_MINOR "${SDL2_VERSION_MINOR_LINE}")
    string(REGEX REPLACE "^#define[ \t]+SDL_PATCHLEVEL[ \t]+([0-9]+)$" "\\1" SDL2_VERSION_PATCH "${SDL2_VERSION_PATCH_LINE}")
    set(SDL2_VERSION "${SDL2_VERSION_MAJOR}.${SDL2_VERSION_MINOR}.${SDL2_VERSION_PATCH}")
    unset(SDL2_VERSION_MAJOR_LINE)
    unset(SDL2_VERSION_MINOR_LINE)
    unset(SDL2_VERSION_PATCH_LINE)
    unset(SDL2_VERSION_MAJOR)
    unset(SDL2_VERSION_MINOR)
    unset(SDL2_VERSION_PATCH)
endif()

if(${CMAKE_SIZEOF_VOID_P} STREQUAL 8)
    set(_arch_suffix x64)
else()
    set(_arch_suffix x86)
endif()

# Find DLLs
find_file(SDL2_DLL_RELEASE
    NAMES SDL2.dll
    PATH_SUFFIXES "lib/${_arch_suffix}" bin
    HINTS ${SDL2_DIR} "${PC_SDL2_PREFIX}"
)
find_file(SDL2_DLL_DEBUG
    NAMES SDL2d.dll
    PATH_SUFFIXES "lib/${_arch_suffix}" "debug/bin" bin
    HINTS ${SDL2_DIR} "${PC_SDL2_PREFIX}"
)

include(SelectDllConfigurations)
select_dll_configurations(SDL2)

if(SDL2_DLL)
    set(_sdl2_shared_release_names "SDL2.lib" "libSDL2.dll.a")
    set(_sdl2_static_release_names "SDL2-static.lib")
    set(_sdl2_shared_debug_names "SDL2d.lib")
    set(_sdl2_static_debug_names "SDL2-staticd.lib")
else()
    set(_sdl2_shared_release_names "")
    set(_sdl2_static_release_names "SDL2.lib" "SDL2-static.lib")
    set(_sdl2_shared_debug_names "")
    set(_sdl2_static_debug_names "SDL2d.lib" "SDL2-staticd.lib")
endif()

set(_saved_suffixes ${CMAKE_FIND_LIBRARY_SUFFIXES})
# Find the SDL2 dynamic libraries
set(CMAKE_FIND_LIBRARY_SUFFIXES "" ".so" ".dylib" ".dll.a")
find_library(SDL2_LIBRARY_RELEASE
    NAMES ${_sdl2_shared_release_names} SDL2
    PATH_SUFFIXES "lib/${_arch_suffix}" lib
    HINTS ${SDL2_DIR} "${PC_SDL2_LIBDIR}"
)
find_library(SDL2_LIBRARY_DEBUG
    NAMES ${_sdl2_shared_debug_names} SDL2d
    PATH_SUFFIXES "lib/${_arch_suffix}" "debug/lib" lib
    HINTS ${SDL2_DIR} "${PC_SDL2_LIBDIR}"
)

# Find the SDL2 static libraries
set(CMAKE_FIND_LIBRARY_SUFFIXES "" ".a")
find_library(SDL2_STATIC_LIBRARY_RELEASE
    NAMES ${_sdl2_static_release_names} SDL2
    PATH_SUFFIXES "lib/${_arch_suffix}" lib
    HINTS ${SDL2_DIR} "${PC_SDL2_LIBDIR}"
)
find_library(SDL2_STATIC_LIBRARY_DEBUG
    NAMES ${_sdl2_static_debug_names} SDL2d
    PATH_SUFFIXES "lib/${_arch_suffix}" "debug/lib" lib
    HINTS ${SDL2_DIR} "${PC_SDL2_LIBDIR}"
)
set(CMAKE_FIND_LIBRARY_SUFFIXES ${_saved_suffixes})

# Find the SDL2main library
find_library(SDL2_MAIN_LIBRARY_RELEASE
    NAMES SDL2main
    PATH_SUFFIXES "lib/${_arch_suffix}" lib manual-link
    HINTS ${SDL2_DIR} "${PC_SDL2_LIBDIR}"
)
find_library(SDL2_MAIN_LIBRARY_DEBUG
    NAMES SDL2maind
    PATH_SUFFIXES "lib/${_arch_suffix}" "debug/lib" lib manual-link
    HINTS ${SDL2_DIR} "${PC_SDL2_LIBDIR}"
)

unset(_saved_suffixes)
unset(_arch_suffix)

# Select libraries
include(SelectLibraryConfigurations)
select_library_configurations(SDL2)
select_library_configurations(SDL2_STATIC)
select_library_configurations(SDL2_MAIN)

get_flags_from_pkg_config("SHARED" "PC_SDL2" "_sdl2")
get_flags_from_pkg_config("STATIC" "PC_SDL2" "_sdl2_static")

# Link flags for SDL2 static library if PkgConfig not found
if(SDL2_STATIC_LIBRARY AND NOT PC_SDL2_FOUND)
    if(WIN32)
        set(_sdl2_static_link_libraries user32 gdi32 winmm imm32 ole32 oleaut32 version uuid advapi32 setupapi shell32)
    elseif(NOT SDL2_LIBRARY)
        set(SDL2_STATIC_LINK_LIBRARIES "" CACHE STRING "Additional libraries to link to SDL2-static.")
        set(SDL2_STATIC_LINK_DIRECTORIES "" CACHE PATH "Additional directories to search libraries in for SDL2-static.")
        set(_sdl2_static_link_libraries ${SDL2_STATIC_LINK_LIBRARIES})
        set(_sdl2_static_link_directories ${SDL2_STATIC_LINK_DIRECTORIES})
        if(NOT _sdl2_static_link_libraries)
            message(WARNING
                "pkg-config is unavailable and only a static version of SDL2 was found.\n"
                "Link failures are to be expected.\n"
                "Set `SDL2_STATIC_LINK_LIBRARIES` to a list of libraries SDL2 depends on.\n"
                "Set `SDL2_STATIC_LINK_DIRECTORIES` to a list of directories to search for libraries in."
            )
        endif()
    endif()
endif()

if(SDL2_LIBRARY)
    set(_SDL2_LIBRARY "${SDL2_LIBRARY}")
else()
    set(_SDL2_LIBRARY "${SDL2_STATIC_LIBRARY}")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SDL2
    FOUND_VAR SDL2_FOUND
    REQUIRED_VARS SDL2_INCLUDE_DIR _SDL2_LIBRARY
    VERSION_VAR SDL2_VERSION
)
unset(_SDL2_LIBRARY)

# Cleanup macro
macro(_cleanup)
    unset(_sdl2_shared_release_names)
    unset(_sdl2_static_release_names)
    unset(_sdl2_shared_debug_names)
    unset(_sdl2_static_debug_names)
    unset(_sdl2_compile_options)
    unset(_sdl2_link_libraries)
    unset(_sdl2_link_directories)
    unset(_sdl2_link_options)
    unset(_sdl2_static_compile_options)
    unset(_sdl2_static_link_libraries)
    unset(_sdl2_static_link_directories)
    unset(_sdl2_static_link_options)
    mark_as_advanced(FORCE
        SDL2_INCLUDE_DIR
        SDL2_DLL_RELEASE
        SDL2_DLL_DEBUG
        SDL2_LIBRARY_RELEASE
        SDL2_LIBRARY_DEBUG
        SDL2_STATIC_LIBRARY_RELEASE
        SDL2_STATIC_LIBRARY_DEBUG
        SDL2_MAIN_LIBRARY_RELEASE
        SDL2_MAIN_LIBRARY_DEBUG
        )
endmacro()

if(NOT SDL2_FOUND)
    _cleanup()
    return()
endif()

# SDL2 imported target
if(SDL2_LIBRARY)
    add_library(SDL2::SDL2 SHARED IMPORTED)
    set_target_properties(SDL2::SDL2 PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${SDL2_INCLUDE_DIR}"
        INTERFACE_COMPILE_OPTIONS "${_sdl2_compile_options}"
        INTERFACE_LINK_LIBRARIES "${_sdl2_link_libraries}"
        INTERFACE_LINK_DIRECTORIES "${_sdl2_link_directories}"
        INTERFACE_LINK_OPTIONS "${_sdl2_link_options}"
    )
    if(SDL2_DLL)
        set_target_properties(SDL2::SDL2 PROPERTIES
            IMPORTED_LOCATION "${SDL2_DLL}"
            IMPORTED_IMPLIB "${SDL2_LIBRARY}"
        )
    else()
        set_target_properties(SDL2::SDL2 PROPERTIES
            IMPORTED_LOCATION "${SDL2_LIBRARY}"
        )
    endif()
    if(SDL2_LIBRARY_RELEASE)
        set_property(TARGET SDL2::SDL2 APPEND PROPERTY
            IMPORTED_CONFIGURATIONS RELEASE
        )
        set_target_properties(SDL2::SDL2 PROPERTIES
            MAP_IMPORTED_CONFIG_MINSIZEREL RELEASE
            MAP_IMPORTED_CONFIG_RELWITHDEBINFO RELEASE
        )
        if(SDL2_DLL_RELEASE)
            set_target_properties(SDL2::SDL2 PROPERTIES
                IMPORTED_LOCATION_RELEASE "${SDL2_DLL_RELEASE}"
                IMPORTED_IMPLIB_RELEASE "${SDL2_LIBRARY_RELEASE}"
            )
        else()
            set_target_properties(SDL2::SDL2 PROPERTIES
                IMPORTED_LOCATION_RELEASE "${SDL2_LIBRARY_RELEASE}"
            )
        endif()
    endif()
    if(SDL2_LIBRARY_DEBUG)
        set_property(TARGET SDL2::SDL2 APPEND PROPERTY
            IMPORTED_CONFIGURATIONS DEBUG
        )
        if(SDL2_DLL_DEBUG)
            set_target_properties(SDL2::SDL2 PROPERTIES
                IMPORTED_LOCATION_DEBUG "${SDL2_DLL_DEBUG}"
                IMPORTED_IMPLIB_DEBUG "${SDL2_LIBRARY_DEBUG}"
            )
        else()
            set_target_properties(SDL2::SDL2 PROPERTIES
                IMPORTED_LOCATION_DEBUG "${SDL2_LIBRARY_DEBUG}"
            )
        endif()
    endif()
endif()

# SDL2-static imported target
if(SDL2_STATIC_LIBRARY)
    add_library(SDL2::SDL2-static STATIC IMPORTED)
    set_target_properties(SDL2::SDL2-static PROPERTIES
        IMPORTED_LOCATION "${SDL2_STATIC_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${SDL2_INCLUDE_DIR}"
        INTERFACE_COMPILE_OPTIONS "${_sdl2_static_compile_options}"
        INTERFACE_LINK_LIBRARIES "${_sdl2_static_link_libraries}"
        INTERFACE_LINK_DIRECTORIES "${_sdl2_static_link_directories}"
        INTERFACE_LINK_OPTIONS "${_sdl2_static_link_options}"
    )
    if(SDL2_STATIC_LIBRARY_RELEASE)
        set_property(TARGET SDL2::SDL2-static APPEND PROPERTY
            IMPORTED_CONFIGURATIONS RELEASE
        )
        set_target_properties(SDL2::SDL2-static PROPERTIES
            IMPORTED_LOCATION_RELEASE "${SDL2_STATIC_LIBRARY_RELEASE}"
            MAP_IMPORTED_CONFIG_MINSIZEREL RELEASE
            MAP_IMPORTED_CONFIG_RELWITHDEBINFO RELEASE
        )
    endif()
    if(SDL2_STATIC_LIBRARY_DEBUG)
        set_property(TARGET SDL2::SDL2-static APPEND PROPERTY
            IMPORTED_CONFIGURATIONS DEBUG
        )
        set_target_properties(SDL2::SDL2-static PROPERTIES
            IMPORTED_LOCATION_DEBUG "${SDL2_STATIC_LIBRARY_DEBUG}"
        )
    endif()
    if(NOT TARGET SDL2::SDL2)
        add_library(SDL2::SDL2 ALIAS SDL2::SDL2-static)
    endif()
endif()

# SDL2main imported target
if(SDL2_MAIN_LIBRARY)
    if(MINGW AND NOT SDL2_DIR)
        # Gross hack to get mingw32 first in the linker order.
        add_library(SDL2::_SDL2main_detail STATIC IMPORTED)
        set_target_properties(SDL2::_SDL2main_detail PROPERTIES
            IMPORTED_LOCATION "${SDL2_MAIN_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${SDL2_INCLUDE_DIR}"
        )
        set(_sdl2main_target "SDL2::_SDL2main_detail")
        # Ensure that SDL2main comes before SDL2 in the linker order.  CMake
        # isn't smart enough to keep proper ordering for indirect dependencies
        # so we have to spell it out here.
        target_link_libraries(SDL2::_SDL2main_detail INTERFACE SDL2::SDL2)

        add_library(SDL2::SDL2main INTERFACE IMPORTED)
        set_target_properties(SDL2::SDL2main PROPERTIES
            IMPORTED_LIBNAME mingw32
        )
        target_link_libraries(SDL2::SDL2main INTERFACE SDL2::_SDL2main_detail)
    else()
        add_library(SDL2::SDL2main STATIC IMPORTED)
        set_target_properties(SDL2::SDL2main PROPERTIES
            IMPORTED_LOCATION "${SDL2_MAIN_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${SDL2_INCLUDE_DIR}"
        )
        set(_sdl2main_target "SDL2::SDL2main")
    endif()
    if(SDL2_MAIN_LIBRARY_RELEASE)
        set_property(TARGET ${_sdl2main_target} APPEND PROPERTY
            IMPORTED_CONFIGURATIONS RELEASE
        )
        set_target_properties(${_sdl2main_target} PROPERTIES
            IMPORTED_LOCATION_RELEASE "${SDL2_MAIN_LIBRARY_RELEASE}"
            MAP_IMPORTED_CONFIG_MINSIZEREL RELEASE
            MAP_IMPORTED_CONFIG_RELWITHDEBINFO RELEASE
        )
    endif()
    if(SDL2_MAIN_LIBRARY_DEBUG)
        set_property(TARGET ${_sdl2main_target} APPEND PROPERTY
            IMPORTED_CONFIGURATIONS DEBUG
        )
        set_target_properties(${_sdl2main_target} PROPERTIES
            IMPORTED_LOCATION_DEBUG "${SDL2_MAIN_LIBRARY_DEBUG}"
        )
    endif()
    unset(_sdl2main_target)
endif()

_cleanup()
