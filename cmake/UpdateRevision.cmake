#!/usr/bin/cmake -P

# UpdateRevision.cmake
#
# Public domain. This program uses git commands command to get
# various bits of repository status for a particular directory
# and writes it into a header file so that it can be used for a
# project's versioning.

# Boilerplate to return a variable from a function.
macro(ret_var VAR)
    set(${VAR} "${${VAR}}" PARENT_SCOPE)
endmacro()

# Populate variables "Hash", "Hash_suffix", "Tag" and "Timestamp" with relevant information
# from source repository.  If anything goes wrong return something in "Error."
function(query_repo_info TagPattern)
    execute_process(
        COMMAND git describe --tags --dirty=-m --abbrev=7 --no-match --match ${TagPattern}
        RESULT_VARIABLE Error
        OUTPUT_VARIABLE Tag
        ERROR_QUIET
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    if(NOT "${Error}" STREQUAL "0")
        set(Tag "<unknown>")
    endif()
    execute_process(
        COMMAND git log -1 "--format=%ai;%H"
        RESULT_VARIABLE Error
        OUTPUT_VARIABLE CommitInfo
        ERROR_QUIET
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    if(NOT "${Error}" STREQUAL "0")
        ret_var(Error)
        return()
    endif()
    list(GET CommitInfo 0 Timestamp)
    list(GET CommitInfo 1 Hash)
    string(SUBSTRING " - ${Hash}" 0 10 Hash_suffix)

    ret_var(Tag)
    ret_var(Hash)
    ret_var(Hash_suffix)
    ret_var(Timestamp)
endfunction()

# Although configure_file doesn't overwrite the file if the contents are the
# same we can't easily observe that to change the status message.  This
# function parses the existing file (if it exists) and puts the hash in
# variable "OldHash"
function(get_existing_hash File)
    if(EXISTS "${File}")
        file(STRINGS "${File}" OldHash LIMIT_COUNT 1)
        if(OldHash)
            string(SUBSTRING "${OldHash}" 3 -1 OldHash)
            ret_var(OldHash)
        endif()
    endif()
endfunction()

function(main)
    if(NOT CMAKE_ARGC EQUAL 5) # cmake -P UpdateRevision.cmake <OutputFile> <TagPattern>
        message(NOTICE "Usage: ${CMAKE_ARGV2} <path to git_info.h> <TagPattern>")
        return()
    endif()
    set(OutputFile "${CMAKE_ARGV3}")

    get_filename_component(ScriptDir "${CMAKE_SCRIPT_MODE_FILE}" DIRECTORY)

    if(CMAKE_ARGV4 STREQUAL "")
        query_repo_info("*")
    else()
        query_repo_info("${CMAKE_ARGV4}")
    endif()

    if(NOT Hash)
        message(NOTICE "Failed to get commit info: ${Error}")
        set(Tag "<unknown>")
        set(Hash "<unknown>")
        set(Hash_suffix "")
        set(Timestamp "<unknown>")
    endif()
    if((CMAKE_ARGV4 STREQUAL "") OR (NOT Tag MATCHES "-g"))
        set(Hash_suffix "")
    endif()

    get_existing_hash("${OutputFile}")
    if("${Hash}${Hash_suffix}" STREQUAL OldHash)
        return()
    endif()

    configure_file("${ScriptDir}/git_info.h.in" "${OutputFile}" @ONLY)

    string(SUBSTRING "${Hash}" 0 7 Hash)
    message(STATUS "Configuring ${OutputFile} - updated to commit ${Hash}")
endfunction()

main()