add_library(pcsound STATIC
            pcsound.cpp       pcsound.hpp
            pcsound_bsd.cpp
            pcsound_sdl.cpp
            pcsound_linux.cpp
            pcsound_win32.cpp
                            pcsound_internal.hpp)
target_include_directories(pcsound
                           INTERFACE "."
                           PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/../")

set(SDL2_MIXER_LIB $<IF:$<TARGET_EXISTS:SDL2_mixer::SDL2_mixer>,SDL2_mixer::SDL2_mixer,SDL2_mixer::SDL2_mixer-static>)
target_link_libraries(pcsound SDL2::SDL2 ${SDL2_MIXER_LIB})
