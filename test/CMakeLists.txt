find_package(Catch2 REQUIRED)

file(GLOB_RECURSE sources CONFIGURE_DEPENDS "*.cpp")

add_executable(test_cpp_doom ${sources})
target_link_libraries(test_cpp_doom Catch2::Catch2 lib_common_cpp_doom lib_map)
#target_compile_definitions(test_cpp_doom PUBLIC CATCH_CONFIG_CONSOLE_WIDTH=300)
target_include_directories(test_cpp_doom PRIVATE ${CMAKE_SOURCE_DIR}/src)

add_test(NAME test_cpp_doom COMMAND test_cpp_doom)
