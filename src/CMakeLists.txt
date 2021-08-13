# [Dasperal] Disable "strife" from the compilation
foreach(SUBDIR doom heretic hexen setup)
    add_subdirectory("${SUBDIR}")
endforeach()

# Common library
add_library(Common STATIC EXCLUDE_FROM_ALL
                         crispy.h
                         jn.h
    i_system.c           i_system.h
    m_argv.c             m_argv.h
    m_misc.c             m_misc.h
)

target_include_directories(Common PUBLIC
    "${PROJECT_BINARY_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}"
)

if(PC_SDL2_FOUND AND NOT (SDL2_MAIN_LIBRARY STREQUAL "SDL2_MAIN_LIBRARY-NOTFOUND"))
    target_link_libraries(Common PUBLIC
        SDL2::SDL2main
    )
endif()

target_link_libraries(Common PUBLIC
    SDL2::SDL2
)

# Game Library
add_library(Game STATIC EXCLUDE_FROM_ALL
    aes_prng.c          aes_prng.h
    d_event.c           d_event.h
                        doomkeys.h
                        doomfeatures.h
                        doomtype.h
    d_iwad.c            d_iwad.h
    d_loop.c            d_loop.h
    d_mode.c            d_mode.h
                        d_name.h
                        d_ticcmd.h
    deh_str.c           deh_str.h
    gusconf.c           gusconf.h
    i_cdmus.c           i_cdmus.h
                        icon.h
    i_controller.c      i_controller.h
    i_endoom.c          i_endoom.h
    i_input.c           i_input.h
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
                        rd_menu_control.h
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
    z_zone.c            z_zone.h
)

target_include_directories(Game PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(Game PUBLIC
    Common
    SDL2::mixer
    SDL2::net
    textscreen
    pcsound
    opl
)

if(SAMPLERATE_FOUND)
    target_link_libraries(Game PUBLIC samplerate::samplerate)
endif()
if(PNG_FOUND)
    target_link_libraries(Game PUBLIC PNG::PNG)
endif()

# SetupCommon libraty
add_library(SetupCommon STATIC EXCLUDE_FROM_ALL
    deh_str.c           deh_str.h
    d_mode.c            d_mode.h
    d_iwad.c            d_iwad.h
                        icon.h
    i_timer.c           i_timer.h
    m_config.c          m_config.h
    net_io.c            net_io.h
    net_packet.c        net_packet.h
    net_sdl.c           net_sdl.h
    net_query.c         net_query.h
    net_structrw.c      net_structrw.h
    z_native.c          z_zone.h
)

target_compile_definitions("SetupCommon"
    PRIVATE "___RD_TARGET_SETUP___"
)

target_link_libraries(SetupCommon PUBLIC
    Common
    SDL2::net
    textscreen
)

# Create executables
# [Dasperal] Disable "strife" from the compilation
foreach(TARGET doom heretic hexen)
    add_executable("${PROGRAM_PREFIX}${TARGET}" WIN32
        i_main.c
        "${PROJECT_SOURCE_DIR}/data/${TARGET}_icon.c"
    )

    add_executable("${PROGRAM_PREFIX}${TARGET}-setup" WIN32
        i_main.c
        "${PROJECT_SOURCE_DIR}/data/setup_icon.c"
    )

    # Add resources for windows
    if(WIN32)
        target_sources("${PROGRAM_PREFIX}${TARGET}" PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/${TARGET}/resource.rc")
        target_sources("${PROGRAM_PREFIX}${TARGET}-setup" PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/setup/${TARGET}-resource.rc")
    endif()

    # Dehacked sources
    if(NOT "${TARGET}" STREQUAL "hexen")
        target_sources("${PROGRAM_PREFIX}${TARGET}" PUBLIC
                                deh_defs.h
            deh_io.c            deh_io.h
            deh_main.c          deh_main.h
            deh_mapping.c       deh_mapping.h
            deh_text.c
        )
    endif()

    target_link_libraries("${PROGRAM_PREFIX}${TARGET}" PUBLIC
        ${TARGET}
        Game
    )

    target_link_libraries("${PROGRAM_PREFIX}${TARGET}-setup" PUBLIC
        setup
        SetupCommon
    )

    # Link flags
    set_target_properties("${PROGRAM_PREFIX}${TARGET}" PROPERTIES
        LINK_FLAGS_RELEASE -s
    )

    set_target_properties("${PROGRAM_PREFIX}${TARGET}-setup" PROPERTIES
        LINK_FLAGS_RELEASE -s
    )
endforeach()

# MSVC link flags
if(MSVC)
    # [Dasperal] Disable "strife" from the compilation
    foreach(TARGET doom heretic hexen)
        set_target_properties("${PROGRAM_PREFIX}${TARGET}" PROPERTIES
            LINK_FLAGS "/MANIFEST:NO"
        )
        set_target_properties("${PROGRAM_PREFIX}${TARGET}-setup" PROPERTIES
            LINK_FLAGS "/MANIFEST:NO"
        )
    endforeach()
endif()

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

# [Dasperal] Disable "strife" from the compilation
foreach(TARGET doom heretic hexen)
    add_dependencies("${PROGRAM_PREFIX}${TARGET}" gus-patches)
endforeach()

# Copying game support wads
# [Dasperal] Disable "strife" from the compilation
foreach(TARGET doom heretic hexen)

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

# Windows DLLs
if(WIN32)
    file(GLOB WIN_LIBS "${PROJECT_SOURCE_DIR}/win_libs/${ARCH}/*.dll")

    foreach(SRC_FILE_PATH ${WIN_LIBS})
        get_filename_component(DEST_FILE_NAME "${SRC_FILE_PATH}" NAME)
        add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${DEST_FILE_NAME}"
                MAIN_DEPENDENCY "${SRC_FILE_PATH}"
                COMMAND ${CMAKE_COMMAND} -E copy_if_different "${SRC_FILE_PATH}"
                "${CMAKE_CURRENT_BINARY_DIR}/${DEST_FILE_NAME}"
                COMMENT "Copying ${CMAKE_CURRENT_BINARY_DIR}/${DEST_FILE_NAME}")
        list(APPEND WIN_LIBS_DEST_FILES "${CMAKE_CURRENT_BINARY_DIR}/${DEST_FILE_NAME}")
    endforeach()
    add_custom_target(win-libs DEPENDS ${WIN_LIBS_DEST_FILES})

    # [Dasperal] Disable "strife" from the compilation
    foreach(TARGET doom heretic hexen)
        add_dependencies("${PROGRAM_PREFIX}${TARGET}" win-libs)
        add_dependencies("${PROGRAM_PREFIX}${TARGET}-setup" win-libs)
    endforeach()
endif()

# Install rules
# strife is not production ready
foreach(TARGET doom heretic hexen)
    install(TARGETS "${PROGRAM_PREFIX}${TARGET}" "${PROGRAM_PREFIX}${TARGET}-setup"
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
    install(FILES ${WIN_LIBS}
            DESTINATION .
    )
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
