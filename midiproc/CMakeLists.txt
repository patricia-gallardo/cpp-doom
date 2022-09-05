if(WIN32)
    add_executable("${PROGRAM_PREFIX}midiproc" WIN32 buffer.cpp buffer.hpp main.cpp proto.hpp)
    target_compile_definitions("${PROGRAM_PREFIX}midiproc" PRIVATE ${DOOM_COMPILE_DEFINITIONS})
    target_include_directories("${PROGRAM_PREFIX}midiproc"
                               PRIVATE "../src/" "${CMAKE_CURRENT_BINARY_DIR}/../")
    target_link_libraries("${PROGRAM_PREFIX}midiproc" SDL2::SDL2main SDL2_mixer::SDL2_mixer)
endif()
