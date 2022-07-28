add_copy_target(win-libs SOURCE
    libFLAC-8.dll
    libgcc_s_dw2-1.dll
    libmodplug-1.dll
    libmpg123-0.dll
    libogg-0.dll
    libpng16-16.dll
    libsamplerate-0.dll
    libvorbis-0.dll
    libvorbisfile-3.dll
    SDL2.dll
    SDL2_mixer.dll
    SDL2_net.dll
    zlib1.dll
    DESTINATION "${PROJECT_BINARY_DIR}/src"
)
