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
    set_target_properties(setup PROPERTIES
        C_STANDARD "${RD_C_STANDARD}"
        C_STANDARD_REQUIRED ON
    )
    target_compile_definitions(setup PRIVATE
        BUILD_DOOM_VERSION="${BUILD_DOOM_VERSION}"
        BUILD_HERETIC_VERSION="${BUILD_HERETIC_VERSION}"
        BUILD_HEXEN_VERSION="${BUILD_HEXEN_VERSION}"
        BUILD_STRIFE_VERSION="${BUILD_STRIFE_VERSION}"
        "$<$<BOOL:${PNG_FOUND}>:HAVE_LIBPNG>"
        PACKAGE_PREFIX="${PACKAGE_PREFIX}"
        PROGRAM_PREFIX="${PROGRAM_PREFIX}"
    )
    target_link_libraries(setup PRIVATE
        textscreen
        SDL2::SDL2
        SDL2::mixer
        SetupCommon
    )
    add_dependencies(setup revision_check)

    # [Dasperal] Disable clang warnings for windows-1251 strings
    if (CMAKE_C_COMPILER_ID STREQUAL "Clang")
        target_compile_options(setup PRIVATE -Wno-invalid-source-encoding)
    endif()
endif()
