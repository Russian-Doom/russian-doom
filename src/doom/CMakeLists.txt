if(RD_COMPILE_DOOM)
    add_library(doom STATIC
                am_map.c        am_map.h
                deh_ammo.c
                deh_bexstr.c
                deh_bexincl.c
                deh_bexpars.c     deh_bexpars.h
                deh_bexptr.c
                deh_cheat.c
                deh_doom.c
                deh_frame.c
                deh_misc.c      deh_misc.h
                deh_ptr.c
                deh_sound.c
                deh_thing.c
                deh_weapon.c
                ct_chat.c       ct_chat.h
                d_items.c       d_items.h
                d_main.c        d_main.h
                d_net.c
                                doomdef.h
                doomstat.c      doomstat.h
                                d_think.h
                f_finale.c      f_finale.h
                f_wipe.c        f_wipe.h
                g_game.c        g_game.h
        g_sk_unm.c
                info.c          info.h
                id_lang.c       id_lang.h
                m_menu.c        m_menu.h
                m_random.c      m_random.h
                p_bexptr.c
                p_ceilng.c
                p_doors.c
                p_enemy.c
                p_fix.c
                p_floor.c
                p_inter.c
                p_lights.c
                                p_local.h
                p_map.c
                p_maputl.c
                p_mobj.c
                p_plats.c
                p_pspr.c
                p_saveg.c
                p_setup.c
                p_sight.c
                p_spec.c
                p_switch.c
                p_telept.c
                p_tick.c
                p_user.c
                r_bmaps.c
                r_bsp.c
                r_data.c
                r_draw.c
                                r_local.h
                r_main.c
                r_plane.c
                r_segs.c
                r_swirl.c
                r_things.c
                s_sound.c       s_sound.h
                sounds.c        sounds.h
                statdump.c      statdump.h
                st_bar.c        st_bar.h
                wi_stuff.c      wi_stuff.h
    )
    target_common_settings(doom)
    target_compile_definitions(doom PRIVATE
        PACKAGE_VERSION="${PROJECT_VERSION}"
        PACKAGE_PREFIX="${PACKAGE_PREFIX}"
        PROGRAM_PREFIX="${PROGRAM_PREFIX}"
    )
    target_link_libraries(doom PRIVATE
        Common
    )
    if(NOT RD_GIT_NO_HASH)
        add_dependencies(doom revision_check)
    endif()
endif()
