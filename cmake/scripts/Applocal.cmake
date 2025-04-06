#!/usr/bin/cmake -P

# The MIT License (MIT)
#
# Copyright (c) 2023 Leonid Murin (Dasperal)
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

# This script recursively searches for runtime dependencies of the given <input_file> and already found DLLs,
# and puts them in the directory occupied by the <input_file>. It accepts a list of <search_path> in form of multiple
# arguments or ';' separated list(s).

# Known bugs: Can't update DLLs. If DLLs already present in output dir, `file(GET_RUNTIME_DEPENDENCIES ...)` will find
# EXACTLY those DLLs ignoring additional search paths.

cmake_minimum_required(VERSION 3.16...4.1)

function(main)
    if(NOT CMAKE_ARGC GREATER_EQUAL 4) # cmake -P Applocal.cmake <input_file> <search_path> ...
        message(NOTICE "Usage: ${CMAKE_ARGV2} <input_file> <search_path> ...")
        return()
    endif()
    set(input_file "${CMAKE_ARGV3}")
    set(_i 4)
    while(_i LESS_EQUAL CMAKE_ARGC)
        list(APPEND search_path ${CMAKE_ARGV${_i}})
        math(EXPR _i "${_i} + 1" OUTPUT_FORMAT DECIMAL)
    endwhile()

    get_filename_component(destination "${input_file}" DIRECTORY)
    get_filename_component(input_ext "${input_file}" EXT)
    if(input_ext MATCHES "\\.[Dd][Ll]{2}")
        set(target_field LIBRARIES "${input_file}")
    else()
        set(target_field EXECUTABLES "${input_file}")
    endif()

    file(GET_RUNTIME_DEPENDENCIES
        RESOLVED_DEPENDENCIES_VAR resolved_dependencies
        ${target_field}
        DIRECTORIES ${search_path}
        PRE_EXCLUDE_REGEXES "api-ms-" "ext-ms-"
        POST_EXCLUDE_REGEXES ".*system32/.*\\.dll"
    )

    file(COPY ${resolved_dependencies} DESTINATION "${destination}")
endfunction()

main()