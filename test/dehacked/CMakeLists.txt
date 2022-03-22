file(GLOB_RECURSE sources CONFIGURE_DEPENDS "*.cpp")

add_executable(test_dehacked ${sources})
target_link_libraries(test_dehacked Catch2::Catch2 lib_common_dehacked)
target_compile_definitions(test_dehacked PUBLIC CATCH_CONFIG_CONSOLE_WIDTH=300)
target_include_directories(test_dehacked PRIVATE ${CMAKE_SOURCE_DIR}/src)

add_test(NAME test_dehacked COMMAND test_dehacked)
