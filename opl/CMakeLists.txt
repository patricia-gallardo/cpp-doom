add_library(opl STATIC
            opl_internal.hpp
            opl.cpp           opl.hpp
            opl_linux.cpp
            opl_obsd.cpp
            opl_queue.cpp     opl_queue.hpp
            opl_sdl.cpp
            opl_timer.cpp     opl_timer.hpp
            opl_win32.cpp
            ioperm_sys.cpp    ioperm_sys.hpp
            opl3.cpp          opl3.hpp)
target_include_directories(opl
                           INTERFACE "."
                           PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/../")

set(SDL2_MIXER_LIB $<IF:$<TARGET_EXISTS:SDL2_mixer::SDL2_mixer>,SDL2_mixer::SDL2_mixer,SDL2_mixer::SDL2_mixer-static>)
target_link_libraries(opl SDL2::SDL2 ${SDL2_MIXER_LIB})
