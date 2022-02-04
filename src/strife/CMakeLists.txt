set(STRIFE_SOURCES
    am_map.cpp        am_map.hpp
    deh_ammo.cpp
    deh_cheat.cpp
    deh_strife.cpp
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
    m_menu.cpp        m_menu.hpp
    m_random.cpp      m_random.hpp
    m_saves.cpp       m_saves.hpp
    p_ceilng.cpp
    p_dialog.cpp      p_dialog.hpp
    p_doors.cpp
    p_enemy.cpp
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
    r_things.cpp      r_things.hpp
    s_sound.cpp       s_sound.hpp
    sounds.cpp        sounds.hpp
    st_lib.cpp        st_lib.hpp
    st_stuff.cpp      st_stuff.hpp
    wi_stuff.cpp      wi_stuff.hpp action_hooks.hpp)

add_library(strife STATIC ${STRIFE_SOURCES})

target_include_directories(strife PRIVATE "../" "../../win32/" "${CMAKE_CURRENT_BINARY_DIR}/../../")
target_link_libraries(strife textscreen SDL2::SDL2 SDL2::SDL2_mixer SDL2::SDL2_net)
