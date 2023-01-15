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

# Populate variables "Hash", "Hash_suffix", "Timestamp",
# "GIT_TAG_DOOM", "GIT_TAG_HERETIC", "GIT_TAG_HEXEN" and "GIT_TAG_STRIFE" with relevant information
# from source repository. If anything goes wrong return something in "Error."
function(query_repo_info)
    execute_process(
        COMMAND "${Git_executable}" tag --points-at HEAD
        RESULT_VARIABLE Error
        OUTPUT_VARIABLE Tags
        ERROR_QUIET
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    execute_process(
        COMMAND "${Git_executable}" log -1 "--format=%ai;%H"
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

    string(REGEX MATCH "^[0-9]+(\\.[0-9]+)+" GIT_TAG_DOOM "${Tags}")
    string(REGEX MATCH "heretic-[0-9]+(\\.[0-9]+)+" GIT_TAG_HERETIC "${Tags}")
    string(REGEX MATCH "hexen-[0-9]+(\\.[0-9]+)+" GIT_TAG_HEXEN "${Tags}")
    string(REGEX MATCH "strife-[0-9]+(\\.[0-9]+)+" GIT_TAG_STRIFE "${Tags}")

    ret_var(Hash)
    ret_var(Hash_suffix)
    ret_var(Timestamp)

    ret_var(GIT_TAG_DOOM)
    ret_var(GIT_TAG_HERETIC)
    ret_var(GIT_TAG_HEXEN)
    ret_var(GIT_TAG_STRIFE)
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
    if(NOT CMAKE_ARGC EQUAL 6) # cmake -P UpdateRevision.cmake <OutputFile> <Suffix:FORCE|NO|AUTO> <path to git>
        message(NOTICE "Usage: ${CMAKE_ARGV2} <path to git_info.h> <Suffix:FORCE|NO|AUTO> <path to git>")
        return()
    endif()
    set(OutputFile "${CMAKE_ARGV3}")
    set(Git_executable "${CMAKE_ARGV5}")

    get_filename_component(ScriptDir "${CMAKE_SCRIPT_MODE_FILE}" DIRECTORY)

    query_repo_info()

    if(NOT Hash)
        message(NOTICE "Failed to get commit info: ${Error}")
        set(Hash "<unknown>")
        set(Hash_suffix "")
        set(Timestamp "<unknown>")
    endif()
    if(CMAKE_ARGV4 STREQUAL "FORCE")
        unset(GIT_TAG_DOOM)
        unset(GIT_TAG_HERETIC)
        unset(GIT_TAG_HEXEN)
        unset(GIT_TAG_STRIFE)
    elseif(CMAKE_ARGV4 STREQUAL "NO")
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