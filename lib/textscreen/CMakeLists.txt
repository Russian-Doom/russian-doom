add_library(textscreen STATIC EXCLUDE_FROM_ALL
                                textscreen.h
            txt_conditional.c   txt_conditional.h
            txt_checkbox.c      txt_checkbox.h
            txt_desktop.c       txt_desktop.h
            txt_dropdown.c      txt_dropdown.h
            txt_fileselect.c    txt_fileselect.h
            txt_gui.c           txt_gui.h
            txt_inputbox.c      txt_inputbox.h
            txt_io.c            txt_io.h
                                txt_main.h
            txt_button.c        txt_button.h
            txt_label.c         txt_label.h
            txt_radiobutton.c   txt_radiobutton.h
            txt_scrollpane.c    txt_scrollpane.h
            txt_separator.c     txt_separator.h
            txt_spinctrl.c      txt_spinctrl.h
            txt_sdl.c           txt_sdl.h
            txt_strut.c         txt_strut.h
            txt_table.c         txt_table.h
            txt_utf8.c          txt_utf8.h
            txt_widget.c        txt_widget.h
            txt_window.c        txt_window.h
            txt_window_action.c txt_window_action.h
)
target_include_directories(textscreen
    INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}"
    PRIVATE "${PROJECT_SOURCE_DIR}/src"     # [Dasperal] FIXME: This should be removed but I dont know how to do it properly
)
set_target_properties(textscreen PROPERTIES
    C_STANDARD "${RD_C_STANDARD}"
    C_STANDARD_REQUIRED ON
)
target_link_libraries(textscreen PRIVATE
    m
    SDL2::SDL2
)

# [Dasperal] Disable clang warnings for windows-1251 strings
if (CMAKE_C_COMPILER_ID STREQUAL "Clang")
    target_compile_options(textscreen PRIVATE -Wno-invalid-source-encoding)
endif()
