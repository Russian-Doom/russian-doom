add_library(pcsound STATIC EXCLUDE_FROM_ALL
            pcsound.c       pcsound.h
            pcsound_bsd.c
            pcsound_sdl.c
            pcsound_linux.c
            pcsound_win32.c
                            pcsound_internal.h
)
target_include_directories(pcsound
    INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}"
)
set_target_properties(pcsound PROPERTIES
    C_STANDARD "${RD_C_STANDARD}"
    C_STANDARD_REQUIRED ON
)
target_compile_definitions(pcsound PRIVATE
    "$<$<BOOL:${HAVE_DEV_ISA_SPKRIO_H}>:HAVE_DEV_ISA_SPKRIO_H>"
    "$<$<BOOL:${HAVE_DEV_SPEAKER_SPEAKER_H}>:HAVE_DEV_SPEAKER_SPEAKER_H>"
    "$<$<BOOL:${HAVE_LINUX_KD_H}>:HAVE_LINUX_KD_H>"
)
target_link_libraries(pcsound PRIVATE
    SDL2::mixer
)
