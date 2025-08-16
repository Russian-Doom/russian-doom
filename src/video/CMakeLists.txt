target_sources(Common PRIVATE
    config.c video_config.h
)

target_include_directories(Common PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}"
)