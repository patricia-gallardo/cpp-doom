add_library(setup STATIC
            compatibility.cpp     compatibility.hpp
            display.cpp           display.hpp
            joystick.cpp          joystick.hpp
            keyboard.cpp          keyboard.hpp
            mainmenu.cpp
            mode.cpp              mode.hpp
            mouse.cpp             mouse.hpp
            multiplayer.cpp       multiplayer.hpp
            sound.cpp             sound.hpp
            execute.cpp           execute.hpp
            txt_joyaxis.cpp       txt_joyaxis.hpp
            txt_joybinput.cpp     txt_joybinput.hpp
            txt_keyinput.cpp      txt_keyinput.hpp
            txt_mouseinput.cpp    txt_mouseinput.hpp)

target_compile_definitions(setup PRIVATE ${DOOM_COMPILE_DEFINITIONS})
target_include_directories(setup PRIVATE "../" "${CMAKE_CURRENT_BINARY_DIR}/../../")

set(SDL2_MIXER_LIB $<IF:$<TARGET_EXISTS:SDL2_mixer::SDL2_mixer>,SDL2_mixer::SDL2_mixer,SDL2_mixer::SDL2_mixer-static>)
target_link_libraries(setup textscreen fmt::fmt SDL2::SDL2 ${SDL2_MIXER_LIB})
