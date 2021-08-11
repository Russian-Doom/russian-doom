
AM_CFLAGS = @SDL_CFLAGS@                                           \
            @SDLMIXER_CFLAGS@                                      \
            -I$(top_srcdir)/textscreen -I$(top_srcdir)/src

noinst_LIBRARIES = libsetup.a

SOURCE_FILES =                                  \
    compatibility.c   compatibility.h           \
    display.c         display.h                 \
    mainmenu.c                                  \
    mode.c            mode.h                    \
    mouse.c           mouse.h                   \
    multiplayer.c     multiplayer.h             \
    sound.c           sound.h                   \
    execute.c         execute.h

libsetup_a_SOURCES = $(SOURCE_FILES)
