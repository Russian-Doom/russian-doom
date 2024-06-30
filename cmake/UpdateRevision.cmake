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

# Populate variables "Hash", "Timestamp", "Version_suffix" with relevant information
# from source repository. If anything goes wrong return something in "Error."
function(query_repo_info Tag ProjectDir)
    execute_process(
        COMMAND "${Git_executable}" log -1 "--format=%ai;%H"
        WORKING_DIRECTORY "${ProjectDir}"
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

    ret_var(Hash)
    ret_var(Timestamp)

    execute_process(
        COMMAND "${Git_executable}" rev-list "${Tag}.." --count
        WORKING_DIRECTORY "${ProjectDir}"
        RESULT_VARIABLE Error
        OUTPUT_VARIABLE Commits_from_release
        ERROR_QUIET
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    if(NOT "${Error}" STREQUAL "0")
        return()
    endif()

    if("${Commits_from_release}" STREQUAL "0")
        return()
    endif()

    execute_process(
        COMMAND "${Git_executable}" rev-parse --abbrev-ref HEAD
        WORKING_DIRECTORY "${ProjectDir}"
        RESULT_VARIABLE Error
        OUTPUT_VARIABLE Branch
        ERROR_QUIET
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    if(NOT "${Error}" STREQUAL "0")
        return()
    endif()

    if("${Branch}" STREQUAL "makepkg")
        execute_process(
            COMMAND "${Git_executable}" log -n 1 "--pretty='%(decorate:separator=;,prefix=,suffix=)'" HEAD
            WORKING_DIRECTORY "${ProjectDir}"
            RESULT_VARIABLE Error
            OUTPUT_VARIABLE Branch
            ERROR_QUIET
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )

        if(NOT "${Error}" STREQUAL "0")
            return()
        endif()

        list(GET Branch 1 Branch)
        string(REPLACE "origin/" "" Branch "${Branch}")
    endif()

    if("${Branch}" STREQUAL "master")
        set(Version_suffix "~${Commits_from_release}")
        set(Display_version_suffix "-${Commits_from_release}")
        set(On_master_branch 1)
        ret_var(Version_suffix)
        ret_var(Display_version_suffix)
        ret_var(On_master_branch)
        return()
    endif()

    execute_process(
        COMMAND "${Git_executable}" rev-list origin/master.. --count
        WORKING_DIRECTORY "${ProjectDir}"
        RESULT_VARIABLE Error
        OUTPUT_VARIABLE Commits_in_branch
        ERROR_QUIET
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    if(NOT "${Error}" STREQUAL "0")
        return()
    endif()

    math(EXPR Commits_from_master "${Commits_from_release} - ${Commits_in_branch}" OUTPUT_FORMAT DECIMAL)
    string(SUBSTRING "${Branch}" 0 1 Branch_short)

    set(Version_suffix "~${Commits_from_master}+${Branch_short}${Commits_in_branch}")
    set(Display_version_suffix "-${Commits_from_master}-${Branch_short}${Commits_in_branch}")
    ret_var(Version_suffix)
    ret_var(Display_version_suffix)
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

function(get_project_version CmakeFile)
    file(STRINGS "${CmakeFile}" Project_statement REGEX "project\(.*\)")
    string(REGEX REPLACE "[()\" ]" ";" Project_statement "${Project_statement}")
    if(Project_statement)
        cmake_parse_arguments("PROJECT"
            ""        # List of Options
            "VERSION" # List of Values
            ""        # List of Lists
            ${Project_statement}
        )
        ret_var(PROJECT_VERSION)
    endif()
endfunction()

function(main)
    if(CMAKE_ARGC LESS 4) # cmake -P UpdateRevision.cmake <path to git> [<OutputFile>]
        message(NOTICE "Usage: ${CMAKE_ARGV2} <path to git> [<path to git_info.h>]")
        return()
    endif()
    set(Git_executable "${CMAKE_ARGV3}")

    get_filename_component(ScriptDir "${CMAKE_SCRIPT_MODE_FILE}" DIRECTORY)
    get_filename_component(ProjectDir "${ScriptDir}" DIRECTORY)
    get_project_version("${ProjectDir}/CMakeLists.txt")

    query_repo_info("${PROJECT_VERSION}" "${ProjectDir}")

    if(NOT Hash)
        message(NOTICE "Failed to get commit info: ${Error}")
        set(Hash "<unknown>")
        set(Timestamp "<unknown>")
        set(Version_suffix "")
        set(Display_version_suffix "")
    else()
        string(SUBSTRING "${Hash}" 0 7 Hash_suffix)

        if(NOT Version_suffix)
            set(Version_suffix "")
            set(Display_version_suffix "")
        else()
            String(APPEND Display_version_suffix " ${Hash_suffix}")
        endif()

        string(SUBSTRING "${Timestamp}" 0 10 Date)
        String(APPEND Display_version_suffix " (${Date})")
    endif()

    if(CMAKE_ARGV4)
        set(OutputFile "${CMAKE_ARGV4}")
        get_existing_hash("${OutputFile}")
        if("${Hash}${Version_suffix}" STREQUAL OldHash)
            return()
        endif()

        configure_file("${ScriptDir}/git_info.h.in" "${OutputFile}" @ONLY)

        message(STATUS "Configuring ${OutputFile} - updated to commit ${Hash_suffix}")
    else()
        if(On_master_branch)
            message(NOTICE "${PROJECT_VERSION}${Version_suffix}")
        else()
            message(NOTICE "${PROJECT_VERSION}${Version_suffix}-${Hash_suffix}")
        endif()
    endif()
endfunction()

main()