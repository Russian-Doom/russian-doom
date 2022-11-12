add_copy_target(win-libs SOURCE
    libmodplug-1.dll
    libogg-0.dll
    libpng16-16.dll
    samplerate.dll
    SDL2.dll
    SDL2_mixer.dll
    SDL2_net.dll
    zlib1.dll
    DESTINATION "${PROJECT_BINARY_DIR}/src"
)
