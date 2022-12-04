add_copy_target(win-libs SOURCE
    libmodplug-1.dll
    libogg-0.dll
    libopus-0.dll
    libopusfile-0.dll
    libsamplerate-0.dll
    SDL2.dll
    SDL2_mixer.dll
    SDL2_net.dll
    DESTINATION "${PROJECT_BINARY_DIR}/src"
)
