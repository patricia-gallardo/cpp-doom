add_library(heretic STATIC
            am_data.hpp
            am_map.cpp            am_map.hpp
            ct_chat.cpp           ct_chat.hpp
            deh_ammo.cpp
            deh_frame.cpp
            deh_htext.cpp
            deh_htic.cpp          deh_htic.hpp
            deh_sound.cpp
            deh_thing.cpp
            deh_weapon.cpp
            d_main.cpp
            d_net.cpp
                                doomdata.hpp
                                doomdef.hpp
                                dstrings.hpp
            f_finale.cpp
            g_game.cpp
            info.cpp              info.hpp
            in_lude.cpp
            m_random.cpp          m_random.hpp
            mn_menu.cpp
                                p_action.hpp
            p_ceilng.cpp
            p_doors.cpp
            p_enemy.cpp
            p_floor.cpp
            p_inter.cpp
            p_lights.cpp
            p_local.hpp
            p_map.cpp
            p_maputl.cpp
            p_mobj.cpp
            p_plats.cpp
            p_pspr.cpp
            p_saveg.cpp
            p_setup.cpp
            p_sight.cpp
            p_spec.cpp            p_spec.hpp
            p_switch.cpp
            p_telept.cpp
            p_tick.cpp
            p_user.cpp
            r_bsp.cpp
            r_data.cpp
            r_draw.cpp
                                r_local.hpp
            r_main.cpp
            r_plane.cpp
            r_segs.cpp
            r_things.cpp
            sb_bar.cpp
            sounds.cpp            sounds.hpp
            s_sound.cpp           s_sound.hpp)

target_include_directories(heretic PRIVATE "../" "${CMAKE_CURRENT_BINARY_DIR}/../../")
target_link_libraries(heretic textscreen SDL2::SDL2 SDL2::SDL2_mixer SDL2::SDL2_net)
