add_library(doom STATIC
            am_map.cpp        am_map.hpp
            deh_ammo.cpp
            deh_bexincl.cpp
            deh_bexpars.cpp   deh_bexpars.hpp
            deh_bexptr.cpp
            deh_bexstr.cpp
            deh_cheat.cpp
            deh_doom.cpp
            deh_frame.cpp
            deh_misc.cpp      deh_misc.hpp
            deh_ptr.cpp
            deh_sound.cpp
            deh_thing.cpp
            deh_weapon.cpp
                            d_englsh.hpp
            d_items.cpp       d_items.hpp
            d_main.cpp        d_main.hpp
            d_net.cpp
                            doomdata.hpp
            doomdef.cpp       doomdef.hpp
            doomstat.cpp      doomstat.hpp
                            d_player.hpp
            dstrings.cpp      dstrings.hpp
                            d_textur.hpp
                            d_think.hpp
            f_finale.cpp      f_finale.hpp
            f_wipe.cpp        f_wipe.hpp
            g_game.cpp        g_game.hpp
            hu_lib.cpp        hu_lib.hpp
            hu_stuff.cpp      hu_stuff.hpp
            info.cpp          info.hpp
                            m_background.hpp
            m_crispy.cpp      m_crispy.hpp
            m_menu.cpp        m_menu.hpp
            m_random.cpp      m_random.hpp
            p_bexptr.cpp
            p_blockmap.cpp
            p_ceilng.cpp
            p_doors.cpp
            p_enemy.cpp
            p_extnodes.cpp    p_extnodes.hpp
            p_extsaveg.cpp    p_extsaveg.hpp
            p_floor.cpp
            p_inter.cpp       p_inter.hpp
            p_lights.cpp
                            p_local.hpp
            p_map.cpp
            p_maputl.cpp
            p_mobj.cpp        p_mobj.hpp
            p_plats.cpp
            p_pspr.cpp        p_pspr.hpp
            p_saveg.cpp       p_saveg.hpp
            p_setup.cpp       p_setup.hpp
            p_sight.cpp
            p_spec.cpp        p_spec.hpp
            p_switch.cpp
            p_telept.cpp
            p_tick.cpp        p_tick.hpp
            p_user.cpp
            r_bmaps.cpp       r_bmaps.hpp
            r_bsp.cpp         r_bsp.hpp
            r_data.cpp        r_data.hpp
                            r_defs.hpp
            r_draw.cpp        r_draw.hpp
                            r_local.hpp
            r_main.cpp        r_main.hpp
            r_plane.cpp       r_plane.hpp
            r_segs.cpp        r_segs.hpp
            r_sky.cpp         r_sky.hpp
                            r_state.hpp
            r_swirl.cpp       r_swirl.hpp
            r_things.cpp      r_things.hpp
            s_musinfo.cpp     s_musinfo.hpp
            s_sound.cpp       s_sound.hpp
            sounds.cpp        sounds.hpp
            statdump.cpp      statdump.hpp
            st_lib.cpp        st_lib.hpp
            st_stuff.cpp      st_stuff.hpp
            wi_stuff.cpp      wi_stuff.hpp event_function_decls.hpp)

target_compile_definitions(doom PRIVATE ${DOOM_COMPILE_DEFINITIONS})
target_include_directories(doom PRIVATE "../" "${CMAKE_CURRENT_BINARY_DIR}/../../")
target_link_libraries(doom SDL2::SDL2 SDL2_mixer::SDL2_mixer SDL2::SDL2_net)
