add_library(heretic STATIC
            am_map.c
            ct_chat.c
            deh_ammo.c
            deh_frame.c
            deh_htext.c
            deh_htic.c          deh_htic.h
            deh_sound.c
            deh_thing.c
            deh_weapon.c
            f_finale.c
            g_sk_unm.c
            g_game.c
                                hr_local.h
            hr_main.c
            hr_net.c
            id_lang.c           id_lang.h
            info.c              info.h
            in_lude.c
            m_random.c
            mn_menu.c
                                p_action.h
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
            sb_bar.c
            sounds.c            sounds.h
            s_sound.c           s_sound.h
)
target_common_settings(heretic)
target_compile_definitions(heretic PRIVATE
    PACKAGE_VERSION="${PROJECT_VERSION}"
    PACKAGE_PREFIX="${PACKAGE_PREFIX}"
    PROGRAM_PREFIX="${PROGRAM_PREFIX}"
)
target_link_libraries(heretic PRIVATE
    Common
)
if(NOT RD_GIT_NO_HASH)
    add_dependencies(heretic revision_check)
endif()
