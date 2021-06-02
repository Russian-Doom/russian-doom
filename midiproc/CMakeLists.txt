if(WIN32)
    add_executable("${PROGRAM_PREFIX}midiproc" WIN32
            buffer.c    buffer.h
            main.c
                        proto.h
    )
    target_include_directories("${PROGRAM_PREFIX}midiproc" PRIVATE
        "${PROJECT_BINARY_DIR}"
        "${PROJECT_SOURCE_DIR}/src"
    )
    if(PC_SDL2_FOUND AND NOT (SDL2_MAIN_LIBRARY STREQUAL "SDL2_MAIN_LIBRARY-NOTFOUND"))
        target_link_libraries("${PROGRAM_PREFIX}midiproc"
            SDL2::SDL2main
        )
    endif()
    target_link_libraries("${PROGRAM_PREFIX}midiproc"
        SDL2::mixer
    )
endif()
