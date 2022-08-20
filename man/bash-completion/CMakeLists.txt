function(add_completion MODULE)
    set(Out "${CMAKE_CURRENT_BINARY_DIR}/${PROGRAM_PREFIX}${MODULE}")
    add_docgen("${Out}"
        GAMES ${MODULE}
        DEFINES "$<$<BOOL:WIN32>:_WIN32>,$<$<BOOL:APPLE>:_APPLE_>"
        COMPLETION_TEMPLATE "${CMAKE_CURRENT_BINARY_DIR}/${MODULE}.template"
        SOURCE "\"${PROJECT_SOURCE_DIR}/src/m_argv.c\""
    )
    add_custom_target("completion-${MODULE}"
        DEPENDS ${Out}
    )
    set_target_properties("completion-${MODULE}" PROPERTIES
        OUT_FILES "${Out}"
    )
endfunction()

foreach(MODULE ${COMPILE_MODULES})
    configure_file("${MODULE}.template.in" "${MODULE}.template" @ONLY)

    add_completion(${MODULE})

    install(FILES $<TARGET_PROPERTY:completion-${MODULE},OUT_FILES>
        DESTINATION "share/bash-completion/completions"
        COMPONENT ${MODULE}
    )
endforeach()
