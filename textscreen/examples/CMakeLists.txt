add_executable(calculator calculator.cpp)
target_link_libraries(calculator PRIVATE textscreen fmt::fmt)
add_executable(guitest guitest.cpp)
target_link_libraries(guitest PRIVATE textscreen fmt::fmt)