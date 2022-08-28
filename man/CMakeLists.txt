if(UNIX AND Python3_Interpreter_FOUND)
    function(add_docgen OUT)
        cmake_parse_arguments(PARSE_ARGV 1 "MAN"
            "" # List of Options
            "LANG;DEFINES;CONFIG_FILE;MANPAGE_TEMPLATE;MARKDOWN_TEMPLATE;PLAINTEXT_TEMPLATE;COMPLETION_TEMPLATE" # List of Values
            "GAMES;SOURCE" # List of Lists
        )

        if(MAN_LANG)
            list(APPEND DOCGEN_ARGS -l ${MAN_LANG})
        endif()
        if(MAN_GAMES)
            list(APPEND DOCGEN_ARGS -g ${MAN_GAMES})
        endif()
        if(MAN_DEFINES)
            list(APPEND DOCGEN_ARGS -d \"${MAN_DEFINES}\")
        endif()
        if(MAN_CONFIG_FILE)
            list(APPEND DOCGEN_ARGS -c \"${MAN_CONFIG_FILE}\")
        endif()
        if(MAN_MANPAGE_TEMPLATE)
            list(APPEND DOCGEN_ARGS -m \"${MAN_MANPAGE_TEMPLATE}\")
            list(APPEND DEPENDS "${MAN_MANPAGE_TEMPLATE}")
        endif()
        if(MAN_MARKDOWN_TEMPLATE)
            list(APPEND DOCGEN_ARGS -M \"${MAN_MARKDOWN_TEMPLATE}\")
            list(APPEND DEPENDS "${MAN_MARKDOWN_TEMPLATE}")
        endif()
        if(MAN_PLAINTEXT_TEMPLATE)
            list(APPEND DOCGEN_ARGS -p \"${MAN_PLAINTEXT_TEMPLATE}\")
            list(APPEND DEPENDS "${MAN_PLAINTEXT_TEMPLATE}")
        endif()
        if(MAN_COMPLETION_TEMPLATE)
            list(APPEND DOCGEN_ARGS -b \"${MAN_COMPLETION_TEMPLATE}\")
            list(APPEND DEPENDS "${MAN_COMPLETION_TEMPLATE}")
        endif()

        list(APPEND DOCGEN_ARGS -o "\"${OUT}\"" ${MAN_SOURCE})

        add_custom_command(OUTPUT "${OUT}"
            COMMAND Python3::Interpreter "\"${PROJECT_SOURCE_DIR}/man/docgen.py\"" ${DOCGEN_ARGS}
            MAIN_DEPENDENCY "${SOURCE}"
            DEPENDS ${DEPENDS} "${PROJECT_SOURCE_DIR}/man/docgen.py"
            WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
            COMMENT "Generating ${OUT}"
        )
    endfunction()

    function(generate_man MODULE)
        foreach(Lang . ru)
            configure_file("${Lang}/${MODULE}.template.in" "${Lang}/${MODULE}.template" @ONLY)

            set(Out "${CMAKE_CURRENT_BINARY_DIR}/${Lang}/${PROGRAM_PREFIX}${MODULE}.6")
            add_docgen("${Out}"
                GAMES ${MODULE}
                MANPAGE_TEMPLATE "${CMAKE_CURRENT_BINARY_DIR}/${Lang}/${MODULE}.template"
                LANG ${Lang}
                SOURCE "\"${PROJECT_SOURCE_DIR}/src/m_argv.c\""
            )
            list(APPEND Out_list ${Out})
        endforeach()

        add_custom_target("man-${MODULE}"
            DEPENDS ${Out_list}
        )
        set_target_properties("man-${MODULE}" PROPERTIES
            OUT_FILES "${Out_list}"
        )
    endfunction()

    foreach(MODULE ${COMPILE_MODULES})
        generate_man(${MODULE})

        install(FILES $<TARGET_PROPERTY:man-${MODULE},OUT_FILES>
            DESTINATION "share/man/man6"
            COMPONENT ${MODULE}
        )
    endforeach()

    add_subdirectory(bash-completion)
endif()
