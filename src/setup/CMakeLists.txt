if(COMPILE_SETUP)
    add_library(setup STATIC
                compatibility.c     compatibility.h
                display.c           display.h
                mainmenu.c
                mode.c              mode.h
                mouse.c             mouse.h
                multiplayer.c       multiplayer.h
                sound.c             sound.h
                execute.c           execute.h
    )

    target_link_libraries(setup PRIVATE
        textscreen
        SDL2::SDL2
        SDL2::mixer
        SetupCommon
    )

    # [Dasperal] Disable clang warnings for windows-1251 strings
    if (CMAKE_C_COMPILER_ID STREQUAL "Clang")
        target_compile_options(setup PRIVATE -Wno-invalid-source-encoding)
    endif()
endif()
