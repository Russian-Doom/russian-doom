add_library(opl STATIC EXCLUDE_FROM_ALL
                            opl_internal.h
            opl.c           opl.h
            opl_linux.c
            opl_obsd.c
            opl_queue.c     opl_queue.h
            opl_sdl.c
            opl_timer.c     opl_timer.h
            opl_win32.c
            ioperm_sys.c    ioperm_sys.h
            opl3.c          opl3.h
)
target_include_directories(opl
    INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}"
)
set_target_properties(opl PROPERTIES
    C_STANDARD "${RD_C_STANDARD}"
    C_STANDARD_REQUIRED ON
)
target_compile_definitions(opl PRIVATE
    "$<$<BOOL:${HAVE_LIBI386}>:HAVE_LIBI386>"
    "$<$<BOOL:${HAVE_LIBAMD64}>:HAVE_LIBAMD64>"
    "$<$<BOOL:${HAVE_IOPERM}>:HAVE_IOPERM>"
)
target_link_libraries(opl PRIVATE
    SDL2::mixer
)
