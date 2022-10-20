foreach(SUBDIR base doom heretic hexen strife setup)
    add_subdirectory("${SUBDIR}")
endforeach()

# Common library
add_library(Common STATIC EXCLUDE_FROM_ALL
                        doomtype.h
    d_event.c           d_event.h
                        d_name.h
                        d_ticcmd.h
                        jn.h
    i_system.c          i_system.h
    i_timer.c           i_timer.h
    m_argv.c            m_argv.h
    m_misc.c            m_misc.h
    m_fixed.c           m_fixed.h
                        rd_io.h
    win_compat.c
    z_zone.c            z_zone.h
)
target_include_directories(Common PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}"
)
set_target_properties(Common PROPERTIES
    C_STANDARD "${RD_C_STANDARD}"
    C_STANDARD_REQUIRED ON
)
target_compile_definitions(Common PRIVATE
    "$<$<BOOL:${BUILD_PORTABLE}>:BUILD_PORTABLE>"
    PACKAGE_TARNAME="${PACKAGE_TARNAME}"
    PROGRAM_PREFIX="${PROGRAM_PREFIX}"
    PUBLIC
    "$<IF:$<BOOL:${HAVE_DECL_STRCASECMP}>,HAVE_DECL_STRCASECMP=1,HAVE_DECL_STRCASECMP=0>"
    "$<IF:$<BOOL:${HAVE_DECL_STRNCASECMP}>,HAVE_DECL_STRNCASECMP=1,HAVE_DECL_STRNCASECMP=0>"
)
target_link_libraries(Common PUBLIC
    "$<$<BOOL:${SDL2_MAIN_LIBRARY}>:SDL2::SDL2main>"
    SDL2::SDL2
)

if(WIN32 AND MSVC)
    target_sources(Common PRIVATE
        win_opendir.c       win_opendir.h
    )
endif()

# Game Library
add_library(Game STATIC EXCLUDE_FROM_ALL
    aes_prng.c          aes_prng.h
                        doomkeys.h
                        doomfeatures.h
    d_iwad.c            d_iwad.h
    d_loop.c            d_loop.h
    d_mode.c            d_mode.h
    deh_str.c           deh_str.h
    gusconf.c           gusconf.h
    i_cdmus.c           i_cdmus.h
                        icon.h
    i_controller.c      i_controller.h
    i_endoom.c          i_endoom.h
    i_input.c           i_input.h
    i_glob.c            i_glob.h
                        i_swap.h
    i_pcsound.c
    i_sdlsound.c
    i_sdlmusic.c
    i_oplmusic.c
    i_sound.c           i_sound.h
    i_video.c           i_video.h
    i_videohr.c         i_videohr.h
    i_winmusic.c        i_winmusic.h
    midifile.c          midifile.h
    mus2mid.c           mus2mid.h
    m_bbox.c            m_bbox.h
    m_cheat.c           m_cheat.h
    m_config.c          m_config.h
    net_client.c        net_client.h
    net_common.c        net_common.h
    net_dedicated.c     net_dedicated.h
                        net_defs.h
    net_gui.c           net_gui.h
    net_io.c            net_io.h
    net_loop.c          net_loop.h
    net_packet.c        net_packet.h
    net_query.c         net_query.h
    net_sdl.c           net_sdl.h
    net_server.c        net_server.h
    net_structrw.c      net_structrw.h
    rd_keybinds.c       rd_keybinds.h
    rd_menu.c           rd_menu.h
                        rd_menu_control.h
    rd_migration.c      rd_migration.h
    rd_text.c           rd_text.h
    sha1.c              sha1.h
    memio.c             memio.h
    tables.c            tables.h
    v_diskicon.c        v_diskicon.h
    v_video.c           v_video.h
                        v_patch.h
    v_trans.c           v_trans.h
    w_checksum.c        w_checksum.h
    w_main.c            w_main.h
    w_wad.c             w_wad.h
    w_file.c            w_file.h
    w_file_stdc.c
    w_file_posix.c
    w_file_win32.c
    w_merge.c           w_merge.h
)
target_include_directories(Game PUBLIC
    "${PROJECT_BINARY_DIR}"
)
set_target_properties(Game PROPERTIES
    C_STANDARD "${RD_C_STANDARD}"
    C_STANDARD_REQUIRED ON
)
target_compile_definitions(Game PRIVATE
    "$<$<BOOL:${SAMPLERATE_FOUND}>:HAVE_LIBSAMPLERATE>"
    "$<$<BOOL:${PNG_FOUND}>:HAVE_LIBPNG>"
    "$<$<BOOL:${HAVE_MMAP}>:HAVE_MMAP>"
    "$<$<BOOL:${BUILD_PORTABLE}>:BUILD_PORTABLE>"
    "$<$<BOOL:${HAVE_DIRENT_H}>:HAVE_DIRENT_H>"
    "$<IF:$<BOOL:${HAVE_DECL_SSCANF_S}>,HAVE_DECL_SSCANF_S=1,HAVE_DECL_SSCANF_S=0>"
    PACKAGE_TARNAME="${PACKAGE_TARNAME}"
    PROGRAM_PREFIX="${PROGRAM_PREFIX}"
)
target_link_libraries(Game PUBLIC
    Common
    SDL2::mixer
    SDL2::net
    textscreen
    pcsound
    opl
    "$<$<BOOL:${SAMPLERATE_FOUND}>:samplerate::samplerate>"
    "$<$<BOOL:${PNG_FOUND}>:PNG::PNG>"
    "$<$<BOOL:${WIN32}>:winmm>"
)

# SetupCommon libraty
if(COMPILE_SETUP)
    add_library(SetupCommon STATIC EXCLUDE_FROM_ALL
        deh_str.c           deh_str.h
        d_mode.c            d_mode.h
        d_iwad.c            d_iwad.h
                            icon.h
        m_config.c          m_config.h
        net_io.c            net_io.h
        net_packet.c        net_packet.h
        net_sdl.c           net_sdl.h
        net_query.c         net_query.h
        net_structrw.c      net_structrw.h
                            rd_migration.h
        z_native.c
    )
    target_include_directories(SetupCommon PUBLIC
        "${PROJECT_BINARY_DIR}"
    )
    set_target_properties(SetupCommon PROPERTIES
        C_STANDARD "${RD_C_STANDARD}"
        C_STANDARD_REQUIRED ON
    )
    target_compile_definitions(SetupCommon PRIVATE
        "___RD_TARGET_SETUP___"
        "$<$<BOOL:${BUILD_PORTABLE}>:BUILD_PORTABLE>"
        PACKAGE_TARNAME="${PACKAGE_TARNAME}"
    )
    target_link_libraries(SetupCommon PUBLIC
        Common
        SDL2::net
        textscreen
    )
endif()

# Create executables
foreach(MODULE ${COMPILE_MODULES})
    if(WIN32)
        add_library("${PROGRAM_PREFIX}${MODULE}" SHARED EXCLUDE_FROM_ALL
            i_main.c
            "${PROJECT_SOURCE_DIR}/data/${MODULE}_icon.c"
        )
        set_target_properties("${PROGRAM_PREFIX}${MODULE}" PROPERTIES
            PREFIX ""
        )
        add_dependencies("${PROGRAM_PREFIX}${MODULE}" win-libs)

        # .exe
        add_executable("${PROGRAM_PREFIX}${MODULE}-exe" WIN32
            win_launcher.c
            "${CMAKE_CURRENT_BINARY_DIR}/${MODULE}/resource.rc"
        )
        target_link_options("${PROGRAM_PREFIX}${MODULE}-exe" PRIVATE
            "$<$<BOOL:${MSVC}>:/MANIFEST:NO>"
            "$<IF:$<BOOL:${MSVC}>,/SUBSYSTEM:WINDOWS,-mwindows>"
        )
        set_target_properties("${PROGRAM_PREFIX}${MODULE}-exe" PROPERTIES
            C_STANDARD "${RD_C_STANDARD}"
            C_STANDARD_REQUIRED ON
            RUNTIME_OUTPUT_NAME "${PROGRAM_PREFIX}${MODULE}"
        )
        target_link_libraries("${PROGRAM_PREFIX}${MODULE}-exe" PRIVATE
            "$<$<BOOL:${SDL2_MAIN_LIBRARY}>:SDL2::SDL2main>"
            SDL2::SDL2
            "${PROGRAM_PREFIX}${MODULE}"
        )
        add_custom_command(TARGET "${PROGRAM_PREFIX}${MODULE}-exe" POST_BUILD
            MAIN_DEPENDENCY "$<TARGET_FILE:${PROGRAM_PREFIX}${MODULE}-exe>"
            COMMAND $<$<OR:$<CONFIG:RELEASE>,$<CONFIG:MINSIZEREL>>:${CMAKE_STRIP}> $<$<OR:$<CONFIG:RELEASE>,$<CONFIG:MINSIZEREL>>:"$<TARGET_FILE:${PROGRAM_PREFIX}${MODULE}-exe>">
        )

        # .com
        add_executable("${PROGRAM_PREFIX}${MODULE}-com" WIN32
            win_launcher.c
        )
        target_link_options("${PROGRAM_PREFIX}${MODULE}-com" PRIVATE
            "$<$<BOOL:${MSVC}>:/MANIFEST:NO>"
            "$<IF:$<BOOL:${MSVC}>,/SUBSYSTEM:CONSOLE,-mconsole>"
        )
        set_target_properties("${PROGRAM_PREFIX}${MODULE}-com" PROPERTIES
            C_STANDARD "${RD_C_STANDARD}"
            C_STANDARD_REQUIRED ON
            RUNTIME_OUTPUT_NAME "${PROGRAM_PREFIX}${MODULE}"
            SUFFIX ".com"
        )
        target_compile_definitions("${PROGRAM_PREFIX}${MODULE}-com" PRIVATE
            HAVE_CONSOLE
        )
        target_link_libraries("${PROGRAM_PREFIX}${MODULE}-com" PRIVATE
            "$<$<BOOL:${SDL2_MAIN_LIBRARY}>:SDL2::SDL2main>"
            SDL2::SDL2
            "${PROGRAM_PREFIX}${MODULE}"
        )
        add_custom_command(TARGET "${PROGRAM_PREFIX}${MODULE}-com" POST_BUILD
            MAIN_DEPENDENCY "$<TARGET_FILE:${PROGRAM_PREFIX}${MODULE}-com>"
            COMMAND $<$<OR:$<CONFIG:RELEASE>,$<CONFIG:MINSIZEREL>>:${CMAKE_STRIP}> $<$<OR:$<CONFIG:RELEASE>,$<CONFIG:MINSIZEREL>>:"$<TARGET_FILE:${PROGRAM_PREFIX}${MODULE}-com>">
        )
    else()
        add_executable("${PROGRAM_PREFIX}${MODULE}"
            i_main.c
            "${PROJECT_SOURCE_DIR}/data/${MODULE}_icon.c"
        )
    endif()

    # Dehacked sources
    if(NOT "${MODULE}" STREQUAL "hexen")
        target_sources("${PROGRAM_PREFIX}${MODULE}" PRIVATE
                                deh_defs.h
            deh_io.c            deh_io.h
            deh_main.c          deh_main.h
            deh_mapping.c       deh_mapping.h
            deh_text.c
        )
    endif()

    target_compile_definitions("${PROGRAM_PREFIX}${MODULE}" PRIVATE
        "$<$<BOOL:${BUILD_PORTABLE}>:BUILD_PORTABLE>"
    )
    target_link_libraries("${PROGRAM_PREFIX}${MODULE}" PRIVATE
        ${MODULE}
        Game
    )
    if(NOT NO_GIT_HASH)
        add_dependencies("${PROGRAM_PREFIX}${MODULE}" revision_check)
    endif()

    add_dependencies("${PROGRAM_PREFIX}${MODULE}" "${MODULE}-wads" gus-patches)
    if(UNIX AND Python3_Interpreter_FOUND)
        add_dependencies("${PROGRAM_PREFIX}${MODULE}" "man-${MODULE}-en" "completion-${MODULE}")
    endif()

    # Properties
    set_target_properties("${PROGRAM_PREFIX}${MODULE}" PROPERTIES
        C_STANDARD "${RD_C_STANDARD}"
        C_STANDARD_REQUIRED ON
    )

    add_custom_command(TARGET "${PROGRAM_PREFIX}${MODULE}" POST_BUILD
        MAIN_DEPENDENCY "$<TARGET_FILE:${PROGRAM_PREFIX}${MODULE}>"
        COMMAND $<$<OR:$<CONFIG:RELEASE>,$<CONFIG:MINSIZEREL>>:${CMAKE_STRIP}> $<$<OR:$<CONFIG:RELEASE>,$<CONFIG:MINSIZEREL>>:"$<TARGET_FILE:${PROGRAM_PREFIX}${MODULE}>">
    )

    if(COMPILE_SETUP)
        add_executable("${PROGRAM_PREFIX}${MODULE}-setup" WIN32
            i_main.c
            "${PROJECT_SOURCE_DIR}/data/setup_icon.c"
        )
        set_target_properties("${PROGRAM_PREFIX}${MODULE}-setup" PROPERTIES
            C_STANDARD "${RD_C_STANDARD}"
            C_STANDARD_REQUIRED ON
        )

        if(WIN32)
            add_dependencies("${PROGRAM_PREFIX}${MODULE}-setup" win-libs)
            # Add resources for windows
            target_sources("${PROGRAM_PREFIX}${MODULE}-setup" PRIVATE
                win_launcher.c
                "${CMAKE_CURRENT_BINARY_DIR}/setup/${MODULE}-resource.rc"
            )
            # Link options
            target_link_options("${PROGRAM_PREFIX}${MODULE}-setup" PRIVATE
                "$<$<BOOL:${MSVC}>:/MANIFEST:NO>"
                "$<IF:$<BOOL:${MSVC}>,/SUBSYSTEM:WINDOWS,-mwindows>"
            )
        endif()

        target_compile_definitions("${PROGRAM_PREFIX}${MODULE}-setup" PRIVATE
            "$<$<BOOL:${BUILD_PORTABLE}>:BUILD_PORTABLE>"
            "___RD_TARGET_SETUP___"
        )
        target_link_libraries("${PROGRAM_PREFIX}${MODULE}-setup" PRIVATE
            setup
            SetupCommon
        )
        if(NOT NO_GIT_HASH)
            add_dependencies("${PROGRAM_PREFIX}${MODULE}-setup" revision_check)
        endif()

        add_custom_command(TARGET "${PROGRAM_PREFIX}${MODULE}-setup" POST_BUILD
            MAIN_DEPENDENCY "$<TARGET_FILE:${PROGRAM_PREFIX}${MODULE}-setup>"
            COMMAND $<$<OR:$<CONFIG:RELEASE>,$<CONFIG:MINSIZEREL>>:${CMAKE_STRIP}> $<$<OR:$<CONFIG:RELEASE>,$<CONFIG:MINSIZEREL>>:"$<TARGET_FILE:${PROGRAM_PREFIX}${MODULE}-setup>">
        )
    endif()

    # Tests
    add_test(NAME "${PROGRAM_PREFIX}${MODULE}"
        COMMAND gdb -ex run -ex quit --args $<TARGET_FILE:${PROGRAM_PREFIX}${MODULE}$<$<BOOL:${WIN32}>:-exe>> -timedemo demo1 -nogui -nosound
        WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/test_data"
    )
    set_tests_properties("${PROGRAM_PREFIX}${MODULE}" PROPERTIES
        PASS_REGULAR_EXPRESSION "Насчитано [0-9]+ gametics в [0-9]+ realtics;Timed [0-9]+ gametics in [0-9]+ realtics"
        FAIL_REGULAR_EXPRESSION "SEGV"
        TIMEOUT 150
    )
endforeach()

# Install rules
list(REMOVE_ITEM COMPILE_MODULES "strife") # strife is not production ready

foreach(MODULE ${COMPILE_MODULES})
    set(INSTALL_TARGETS "${PROGRAM_PREFIX}${MODULE}")
    if(WIN32)
        list(APPEND INSTALL_TARGETS "${PROGRAM_PREFIX}${MODULE}-exe" "${PROGRAM_PREFIX}${MODULE}-com")
    endif()
    if(COMPILE_SETUP)
        list(APPEND INSTALL_TARGETS "${PROGRAM_PREFIX}${MODULE}-setup")
    endif()
    install(TARGETS ${INSTALL_TARGETS}
            DESTINATION $<IF:$<OR:$<BOOL:${WIN32}>,$<BOOL:${BUILD_PORTABLE}>>,.,${CMAKE_INSTALL_BINDIR}>
            PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_WRITE GROUP_EXECUTE WORLD_READ WORLD_WRITE WORLD_EXECUTE
            COMPONENT "${MODULE}"
    )
endforeach()

install(FILES "${PROJECT_SOURCE_DIR}/LICENSE.txt"
    RENAME "GPL.txt"
    DESTINATION $<IF:$<OR:$<BOOL:${WIN32}>,$<BOOL:${BUILD_PORTABLE}>>,.,${CMAKE_INSTALL_DATAROOTDIR}/doc/${PACKAGE_TARNAME}>
)
# Same install rule but for component based install
foreach(MODULE ${COMPILE_MODULES})
    install(FILES "${PROJECT_SOURCE_DIR}/LICENSE.txt"
        RENAME "GPL.txt"
        DESTINATION $<IF:$<OR:$<BOOL:${WIN32}>,$<BOOL:${BUILD_PORTABLE}>>,.,${CMAKE_INSTALL_DATAROOTDIR}/doc/${PACKAGE_TARNAME}>
        COMPONENT "${MODULE}"
        EXCLUDE_FROM_ALL
    )
endforeach()

# Source groups
source_group("Opl" REGULAR_EXPRESSION "^${PROJECT_SOURCE_DIR}/opl/.+")
source_group("Pcsound" REGULAR_EXPRESSION "^${PROJECT_SOURCE_DIR}/pcsound/.+")
source_group("Textscreen" REGULAR_EXPRESSION "^${PROJECT_SOURCE_DIR}/textscreen/.+")
source_group("Common sources" REGULAR_EXPRESSION "^${CMAKE_CURRENT_SOURCE_DIR}/.+")
source_group("Doom" REGULAR_EXPRESSION "^${CMAKE_CURRENT_SOURCE_DIR}/doom/.+")
source_group("Heretic" REGULAR_EXPRESSION "^${CMAKE_CURRENT_SOURCE_DIR}/heretic/.+")
source_group("Hexen" REGULAR_EXPRESSION "^${CMAKE_CURRENT_SOURCE_DIR}/hexen/.+")
source_group("Strife" REGULAR_EXPRESSION "^${CMAKE_CURRENT_SOURCE_DIR}/strife/.+")
source_group("Setup" REGULAR_EXPRESSION "^${CMAKE_CURRENT_SOURCE_DIR}/setup/.+")
