function(add_copy_target)
    set(ARGS NAME DESTINATION)
    set(LIST_ARGS SOURCE)
    cmake_parse_arguments(PARSE_ARGV 0 "ARG"
        ""
        "${ARGS}"
        "${LIST_ARGS}"
    )

    foreach(SOURCE_FILE_PATH ${ARG_SOURCE})
        get_filename_component(DEST_FILE_NAME "${SOURCE_FILE_PATH}" NAME)
        add_custom_command(OUTPUT "${ARG_DESTINATION}/${DEST_FILE_NAME}"
            MAIN_DEPENDENCY "${SOURCE_FILE_PATH}"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different "${SOURCE_FILE_PATH}" "${ARG_DESTINATION}/${DEST_FILE_NAME}"
            COMMENT "Copying ${CMAKE_CURRENT_BINARY_DIR}/${ARG_DESTINATION}/${DEST_FILE_NAME}"
        )
        list(APPEND DESTINATION_FILES "${CMAKE_CURRENT_BINARY_DIR}/${ARG_DESTINATION}/${DEST_FILE_NAME}")
    endforeach()

    add_custom_target(${ARG_NAME} DEPENDS ${DESTINATION_FILES})
    set_target_properties(${ARG_NAME} PROPERTIES
        OUT_FILES "${DESTINATION_FILES}"
    )
endfunction()

include(CheckCCompilerFlag)

# Don't support generator expressions
function(add_compile_options_checked)
    foreach(flag ${ARGV})
        # Turn flag into suitable internal cache variable.
        string(REGEX REPLACE "-(.*)" "CFLAG_\\1" flag_found ${flag})
        string(REPLACE "=" "_" flag_found "${flag_found}")

        check_c_compiler_flag(${flag} ${flag_found})
        if(${flag_found})
            list(APPEND flag_list ${flag})
        endif()
    endforeach()
    add_compile_options(${flag_list})
endfunction()

function(configure_empty_git_info Template Output)
    set(Tag "<unknown>")
    set(Hash "<unknown>")
    set(Hash_suffix "")
    set(Timestamp "<unknown>")
    configure_file("${Template}" "${Output}" @ONLY)
endfunction()
