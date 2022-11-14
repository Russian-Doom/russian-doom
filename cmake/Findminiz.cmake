# Findminiz.cmake
#
# Copyright (c) 2022, Murin Leonid (Dasperal) <Dasperal1@gmail.com>
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
if(PKG_CONFIG_FOUND)
    pkg_search_module(PC_MINIZ QUIET miniz)
endif()

# Find the include directory
find_path(MINIZ_INCLUDE_DIR "miniz.h"
    HINTS ${PC_MINIZ_INCLUDE_DIRS} "${MINIZ_DIR}" "${MINIZ_DIR}/include" "${MINIZ_DIR}/include/miniz")

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

# Find library
if(CMAKE_SIZEOF_VOID_P STREQUAL 8)
    find_library(MINIZ_LIBRARY "miniz"
        HINTS ${PC_MINIZ_LIBRARY_DIRS} "${MINIZ_DIR}/lib/x64" "${MINIZ_DIR}/lib")
else()
    find_library(MINIZ_LIBRARY "miniz"
        HINTS ${PC_MINIZ_LIBRARY_DIRS} "${MINIZ_DIR}/lib/x86" "${MINIZ_DIR}/lib")
endif()

include(FindPackageHandleStandardArgs)

if(EXISTS "${MINIZ_LIBRARY}")
    # Have library
    find_package_handle_standard_args(miniz
        FOUND_VAR MINIZ_FOUND
        REQUIRED_VARS MINIZ_INCLUDE_DIR MINIZ_LIBRARY
        VERSION_VAR MINIZ_VERSION
    )
else()
    # No library. May be amalgamated sources? Find .h and .c files
    find_file(MINIZ_C_FILE "miniz.c"
        HINTS "${MINIZ_DIR}")
    find_file(MINIZ_H_FILE "miniz.h"
        HINTS "${MINIZ_DIR}")
    find_package_handle_standard_args(miniz
        FOUND_VAR MINIZ_FOUND
        REQUIRED_VARS MINIZ_INCLUDE_DIR MINIZ_C_FILE MINIZ_H_FILE
        VERSION_VAR MINIZ_VERSION
    )
endif()

if(MINIZ_FOUND)
    if(EXISTS "${MINIZ_LIBRARY}")
        # Use imported library
        add_library(miniz::miniz UNKNOWN IMPORTED)
        set_target_properties(miniz::miniz PROPERTIES
            INTERFACE_COMPILE_OPTIONS "${PC_MINIZ_CFLAGS_OTHER}"
            INTERFACE_INCLUDE_DIRECTORIES "${MINIZ_INCLUDE_DIR}"
            INTERFACE_miniz_MAJOR_VERSION "${MINIZ_VERSION_MAJOR}"
            COMPATIBLE_INTERFACE_STRING "miniz_MAJOR_VERSION"
            IMPORTED_LOCATION "${MINIZ_LIBRARY}"
        )
    else()
        # Have only amalgamated sources, so build static library from them
        add_library(miniz STATIC EXCLUDE_FROM_ALL
            ${MINIZ_C_FILE}     ${MINIZ_H_FILE}
        )
        target_compile_definitions(miniz
            PRIVATE $<$<C_COMPILER_ID:GNU>:_GNU_SOURCE>)
        set_target_properties(miniz PROPERTIES
            INTERFACE_COMPILE_OPTIONS "${PC_MINIZ_CFLAGS_OTHER}"
            INTERFACE_INCLUDE_DIRECTORIES "${MINIZ_INCLUDE_DIR}"
            INTERFACE_miniz_MAJOR_VERSION "${MINIZ_VERSION_MAJOR}"
            COMPATIBLE_INTERFACE_STRING "miniz_MAJOR_VERSION"
            C_STANDARD 90
            C_STANDARD_REQUIRED ON
        )
        add_library(miniz::miniz ALIAS miniz)
    endif()
endif()

unset(MINIZ_VERSION_MAJOR)
