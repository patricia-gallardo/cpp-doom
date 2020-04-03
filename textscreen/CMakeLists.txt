add_library(textscreen STATIC
            textscreen.hpp
            txt_conditional.cpp   txt_conditional.hpp
            txt_checkbox.cpp      txt_checkbox.hpp
            txt_desktop.cpp       txt_desktop.hpp
            txt_dropdown.cpp      txt_dropdown.hpp
            txt_fileselect.cpp    txt_fileselect.hpp
            txt_gui.cpp           txt_gui.hpp
            txt_inputbox.cpp      txt_inputbox.hpp
            txt_io.cpp            txt_io.hpp
                                txt_main.hpp
            txt_button.cpp        txt_button.hpp
            txt_label.cpp         txt_label.hpp
            txt_radiobutton.cpp   txt_radiobutton.hpp
            txt_scrollpane.cpp    txt_scrollpane.hpp
            txt_separator.cpp     txt_separator.hpp
            txt_spinctrl.cpp      txt_spinctrl.hpp
            txt_sdl.cpp           txt_sdl.hpp
            txt_strut.cpp         txt_strut.hpp
            txt_table.cpp         txt_table.hpp
            txt_utf8.cpp          txt_utf8.hpp
            txt_widget.cpp        txt_widget.hpp
            txt_window.cpp        txt_window.hpp
            txt_window_action.cpp txt_window_action.hpp)
target_include_directories(textscreen
                           INTERFACE "."
                           PRIVATE "../src/")
target_link_libraries(textscreen m SDL2::SDL2)
