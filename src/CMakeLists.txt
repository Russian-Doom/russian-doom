foreach(SUBDIR doom heretic hexen strife setup)
    add_subdirectory("${SUBDIR}")
endforeach()

# Create executables
foreach(TARGET doom heretic hexen strife setup)
    add_executable("${PROGRAM_PREFIX}${TARGET}" WIN32
                             crispy.h
                             jn.h
        i_main.c
        i_system.c           i_system.h
        m_argv.c             m_argv.h
        m_misc.c             m_misc.h
    )
endforeach()

# Add resources for windows
if(WIN32)
    foreach(TARGET doom heretic hexen strife)
        target_sources("${PROGRAM_PREFIX}${TARGET}" PRIVATE
            "${CMAKE_CURRENT_BINARY_DIR}/resource.rc"
        )
    endforeach()

    target_sources("${PROGRAM_PREFIX}setup" PRIVATE
        "${CMAKE_CURRENT_BINARY_DIR}/setup-res.rc"
    )
endif()

# Add game sources
foreach(TARGET doom heretic hexen strife)
    target_sources("${PROGRAM_PREFIX}${TARGET}" PRIVATE
        aes_prng.c          aes_prng.h
        d_event.c           d_event.h
                            doomkeys.h
                            doomfeatures.h
                            doomtype.h
        d_iwad.c            d_iwad.h
        d_loop.c            d_loop.h
        d_mode.c            d_mode.h
                            d_ticcmd.h
        deh_str.c           deh_str.h
        gusconf.c           gusconf.h
        i_cdmus.c           i_cdmus.h
        i_endoom.c          i_endoom.h
        i_input.c           i_input.h
        i_joystick.c        i_joystick.h
                            i_swap.h
        i_midipipe.c        i_midipipe.h
        i_pcsound.c
        i_sdlsound.c
        i_sdlmusic.c
        i_oplmusic.c
        i_sound.c           i_sound.h
        i_timer.c           i_timer.h
        i_video.c           i_video.h
        i_videohr.c         i_videohr.h
        midifile.c          midifile.h
        mus2mid.c           mus2mid.h
        m_bbox.c            m_bbox.h
        m_cheat.c           m_cheat.h
        m_config.c          m_config.h
        m_controls.c        m_controls.h
        m_fixed.c           m_fixed.h
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
        z_zone.c            z_zone.h
    )
endforeach()

# Add dehacked sources
foreach(TARGET doom heretic strife)
    target_sources("${PROGRAM_PREFIX}${TARGET}" PRIVATE
                            deh_defs.h
        deh_io.c            deh_io.h
        deh_main.c          deh_main.h
        deh_mapping.c       deh_mapping.h
        deh_text.c
    )
endforeach()

# Add sources for russian-setup:
target_sources("${PROGRAM_PREFIX}setup" PRIVATE
        deh_str.c           deh_str.h
        d_mode.c            d_mode.h
        d_iwad.c            d_iwad.h
        i_timer.c           i_timer.h
        m_config.c          m_config.h
        m_controls.c        m_controls.h
        net_io.c            net_io.h
        net_packet.c        net_packet.h
        net_sdl.c           net_sdl.h
        net_query.c         net_query.h
        net_structrw.c      net_structrw.h
        z_native.c          z_zone.h
)

# Add include directories and link libraries
foreach(TARGET doom heretic hexen strife)
    target_include_directories("${PROGRAM_PREFIX}${TARGET}" PRIVATE
            "${PROJECT_BINARY_DIR}"
    )
    target_link_libraries("${PROGRAM_PREFIX}${TARGET}"
        ${TARGET}
    )
    if(PC_SDL2_FOUND AND NOT (SDL2_MAIN_LIBRARY STREQUAL "SDL2_MAIN_LIBRARY-NOTFOUND"))
        target_link_libraries("${PROGRAM_PREFIX}${TARGET}"
            SDL2::SDL2main
        )
    endif()
    target_link_libraries("${PROGRAM_PREFIX}${TARGET}"
        SDL2::SDL2
        SDL2::mixer
        SDL2::net
        textscreen
        pcsound
        opl
    )

    if(SAMPLERATE_FOUND)
        target_link_libraries("${PROGRAM_PREFIX}${TARGET}"
            samplerate::samplerate
        )
    endif()
    if(PNG_FOUND)
        target_link_libraries("${PROGRAM_PREFIX}${TARGET}"
            PNG::PNG
        )
    endif()
endforeach()
target_include_directories("${PROGRAM_PREFIX}setup" PRIVATE
        "${PROJECT_BINARY_DIR}"
)
if(PC_SDL2_FOUND AND NOT (SDL2_MAIN_LIBRARY STREQUAL "SDL2_MAIN_LIBRARY-NOTFOUND"))
    target_link_libraries("${PROGRAM_PREFIX}setup"
        SDL2::SDL2main
    )
endif()
target_link_libraries("${PROGRAM_PREFIX}setup"
    SDL2::SDL2
    SDL2::mixer
    SDL2::net
    setup
    textscreen
)

# MSVC link flags
if(MSVC)
    foreach(TARGET doom heretic hexen strife setup)
        set_target_properties("${PROGRAM_PREFIX}${TARGET}" PROPERTIES
            LINK_FLAGS "/MANIFEST:NO")
    endforeach()
endif()

# Generation of game-specific setup executables
foreach(TARGET doom heretic hexen strife)
    add_custom_command(TARGET ${PROGRAM_PREFIX}setup POST_BUILD
            MAIN_DEPENDENCY "$<TARGET_FILE:${PROGRAM_PREFIX}setup>"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:${PROGRAM_PREFIX}setup>"
            "${PROGRAM_PREFIX}${TARGET}-setup$<TARGET_FILE_SUFFIX:${PROGRAM_PREFIX}setup>")
    list(APPEND GAME_SETUP_FILES "${PROGRAM_PREFIX}${TARGET}-setup$<TARGET_FILE_SUFFIX:${PROGRAM_PREFIX}setup>")
endforeach()
set_target_properties(${PROGRAM_PREFIX}setup PROPERTIES
    ADDITIONAL_CLEAN_FILES "${GAME_SETUP_FILES}"
)

# Copying gus patches
file(GLOB GUS_PATCHES_SRC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/base/gus_patches/*")

foreach(SRC_FILE_PATH ${GUS_PATCHES_SRC_FILES})
    get_filename_component(DEST_FILE_NAME "${SRC_FILE_PATH}" NAME)
    add_custom_command(OUTPUT "base/gus_patches/${DEST_FILE_NAME}"
            MAIN_DEPENDENCY "${SRC_FILE_PATH}"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different "${SRC_FILE_PATH}"
            "base/gus_patches/${DEST_FILE_NAME}"
            COMMENT "Copying ${CMAKE_CURRENT_BINARY_DIR}/base/gus_patches/${DEST_FILE_NAME}")
    list(APPEND GUS_PATCHES_DEST_FILES "${CMAKE_CURRENT_BINARY_DIR}/base/gus_patches/${DEST_FILE_NAME}")
endforeach()
add_custom_target(gus-patches DEPENDS ${GUS_PATCHES_DEST_FILES})

foreach(TARGET doom heretic hexen strife setup)
    add_dependencies("${PROGRAM_PREFIX}${TARGET}" gus-patches)
endforeach()

# Copying game support wads
foreach(TARGET doom heretic hexen strife)

    file(GLOB WADS "${CMAKE_CURRENT_SOURCE_DIR}/base/${TARGET}-*.wad")
    foreach(SRC_FILE_PATH ${WADS})
        get_filename_component(DEST_FILE_NAME "${SRC_FILE_PATH}" NAME)
        add_custom_command(OUTPUT "base/${DEST_FILE_NAME}"
                MAIN_DEPENDENCY "${SRC_FILE_PATH}"
                COMMAND ${CMAKE_COMMAND} -E copy_if_different "${SRC_FILE_PATH}"
                "base/${DEST_FILE_NAME}"
                COMMENT "Copying ${CMAKE_CURRENT_BINARY_DIR}/base/${DEST_FILE_NAME}")
        list(APPEND "${TARGET}_DEST_WADS" "${CMAKE_CURRENT_BINARY_DIR}/base/${DEST_FILE_NAME}")
    endforeach()
    add_custom_target("${TARGET}-wads" DEPENDS ${${TARGET}_DEST_WADS})
    add_dependencies("${PROGRAM_PREFIX}${TARGET}" "${TARGET}-wads")
endforeach()

# Install rules
# strife is not production ready
foreach(TARGET doom heretic hexen)
    install(TARGETS "${PROGRAM_PREFIX}${TARGET}"
            DESTINATION $<IF:$<BOOL:${WIN32}>,.,bin>
            PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_WRITE GROUP_EXECUTE WORLD_READ WORLD_WRITE WORLD_EXECUTE
    )
    install(PROGRAMS "${CMAKE_CURRENT_BINARY_DIR}/${PROGRAM_PREFIX}${TARGET}-setup$<TARGET_FILE_SUFFIX:${PROGRAM_PREFIX}setup>"
            DESTINATION $<IF:$<BOOL:${WIN32}>,.,bin>
            PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_WRITE GROUP_EXECUTE WORLD_READ WORLD_WRITE WORLD_EXECUTE
    )
    install(FILES ${${TARGET}_DEST_WADS}
            DESTINATION $<IF:$<BOOL:${WIN32}>,base,share/${PACKAGE_TARNAME}>
    )
endforeach()
install(FILES ${GUS_PATCHES_DEST_FILES}
        DESTINATION $<IF:$<BOOL:${WIN32}>,base,share/${PACKAGE_TARNAME}>/gus_patches
)
if(WIN32)
    install(TARGETS "${PROGRAM_PREFIX}midiproc"
            DESTINATION .
            PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_WRITE GROUP_EXECUTE WORLD_READ WORLD_WRITE WORLD_EXECUTE
    )
#   TODO install dll libs on windows
#   install(FILES <DLL LIBS>
#           DESTINATION .)
endif()

# Source groups
source_group("Midiproc" REGULAR_EXPRESSION "^${PROJECT_SOURCE_DIR}/midiproc/.+")
source_group("Opl" REGULAR_EXPRESSION "^${PROJECT_SOURCE_DIR}/opl/.+")
source_group("Pcsound" REGULAR_EXPRESSION "^${PROJECT_SOURCE_DIR}/pcsound/.+")
source_group("Textscreen" REGULAR_EXPRESSION "^${PROJECT_SOURCE_DIR}/textscreen/.+")
source_group("Common sources" REGULAR_EXPRESSION "^${CMAKE_CURRENT_SOURCE_DIR}/.+")
source_group("Doom" REGULAR_EXPRESSION "^${CMAKE_CURRENT_SOURCE_DIR}/doom/.+")
source_group("Heretic" REGULAR_EXPRESSION "^${CMAKE_CURRENT_SOURCE_DIR}/heretic/.+")
source_group("Hexen" REGULAR_EXPRESSION "^${CMAKE_CURRENT_SOURCE_DIR}/hexen/.+")
source_group("Strife" REGULAR_EXPRESSION "^${CMAKE_CURRENT_SOURCE_DIR}/strife/.+")
source_group("Setup" REGULAR_EXPRESSION "^${CMAKE_CURRENT_SOURCE_DIR}/setup/.+")
