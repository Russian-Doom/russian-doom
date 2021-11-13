# [Dasperal] Disable "setup" and "strife" from the compilation
# [JN] Disable "setup" from the compilation entirely
SUBDIRS = doom heretic hexen

execgamesdir = ${bindir}

# [Dasperal] Disable "strife" from the compilation
execgames_PROGRAMS = @PROGRAM_PREFIX@doom          \
                     @PROGRAM_PREFIX@heretic       \
                     @PROGRAM_PREFIX@hexen         
#                    @PROGRAM_PREFIX@doom-setup    \
#                    @PROGRAM_PREFIX@heretic-setup \
#                    @PROGRAM_PREFIX@hexen-setup
#                    @PROGRAM_PREFIX@strife
#                    @PROGRAM_PREFIX@strife-setup

AM_CFLAGS = -I$(top_srcdir)/textscreen            \
            -I$(top_srcdir)/opl                   \
            -I$(top_srcdir)/pcsound               \
            @SDLMIXER_CFLAGS@ @SDLNET_CFLAGS@

# Common source files used by absolutely everything:

COMMON_SOURCE_FILES=\
                     jn.h                  \
i_system.c           i_system.h            \
m_argv.c             m_argv.h              \
m_misc.c             m_misc.h

# Source files used by the game binaries (russian-doom, etc.)

GAME_SOURCE_FILES=\
aes_prng.c           aes_prng.h            \
d_event.c            d_event.h             \
                     doomkeys.h            \
                     doomfeatures.h        \
                     doomtype.h            \
d_iwad.c             d_iwad.h              \
d_loop.c             d_loop.h              \
d_mode.c             d_mode.h              \
                     d_name.h              \
                     d_ticcmd.h            \
deh_str.c            deh_str.h             \
gusconf.c            gusconf.h             \
i_cdmus.c            i_cdmus.h             \
                     icon.h                \
i_controller.c       i_controller.h        \
i_endoom.c           i_endoom.h            \
i_input.c            i_input.h             \
                     i_swap.h              \
i_pcsound.c                                \
i_sdlsound.c                               \
i_sdlmusic.c                               \
i_oplmusic.c                               \
i_sound.c            i_sound.h             \
i_timer.c            i_timer.h             \
i_video.c            i_video.h             \
i_videohr.c          i_videohr.h           \
i_winmusic.c         i_winmusic.h          \
midifile.c           midifile.h            \
mus2mid.c            mus2mid.h             \
m_bbox.c             m_bbox.h              \
m_cheat.c            m_cheat.h             \
m_config.c           m_config.h            \
m_fixed.c            m_fixed.h             \
net_client.c         net_client.h          \
net_common.c         net_common.h          \
net_dedicated.c      net_dedicated.h       \
net_defs.h                                 \
net_gui.c            net_gui.h             \
net_io.c             net_io.h              \
net_loop.c           net_loop.h            \
net_packet.c         net_packet.h          \
net_query.c          net_query.h           \
net_sdl.c            net_sdl.h             \
net_server.c         net_server.h          \
net_structrw.c       net_structrw.h        \
rd_keybinds.c        rd_keybinds.h         \
rd_menu.c            rd_menu.h             \
                     rd_menu_control.h     \
rd_text.c            rd_text.h             \
sha1.c               sha1.h                \
memio.c              memio.h               \
tables.c             tables.h              \
v_diskicon.c         v_diskicon.h          \
v_video.c            v_video.h             \
                     v_patch.h             \
v_trans.c            v_trans.h             \
w_checksum.c         w_checksum.h          \
w_main.c             w_main.h              \
w_wad.c              w_wad.h               \
w_file.c             w_file.h              \
w_file_stdc.c                              \
w_file_posix.c                             \
w_file_win32.c                             \
w_merge.c            w_merge.h             \
z_zone.c             z_zone.h

# source files needed for FEATURE_DEHACKED

FEATURE_DEHACKED_SOURCE_FILES =            \
deh_defs.h                                 \
deh_io.c             deh_io.h              \
deh_main.c           deh_main.h            \
deh_mapping.c        deh_mapping.h         \
deh_text.c

# Some games support dehacked patches, some don't:

SOURCE_FILES = $(COMMON_SOURCE_FILES)              \
               $(GAME_SOURCE_FILES)                \
               i_main.c

SOURCE_FILES_WITH_DEH = $(SOURCE_FILES)                    \
                        $(FEATURE_DEHACKED_SOURCE_FILES)

EXTRA_LIBS =                                               \
               $(top_builddir)/textscreen/libtextscreen.a  \
               $(top_builddir)/pcsound/libpcsound.a        \
               $(top_builddir)/opl/libopl.a                \
               @LDFLAGS@                                   \
               @SDL_LIBS@                                  \
               @SDLMIXER_LIBS@                             \
               @SDLNET_LIBS@

if HAVE_WINDRES
@PROGRAM_PREFIX@doom_SOURCES=$(SOURCE_FILES_WITH_DEH) $(top_builddir)/data/doom_icon.c doom/resource.rc
else
@PROGRAM_PREFIX@doom_SOURCES=$(SOURCE_FILES_WITH_DEH) $(top_builddir)/data/doom_icon.c
endif

@PROGRAM_PREFIX@doom_LDADD = doom/libdoom.a $(EXTRA_LIBS)

if HAVE_WINDRES
@PROGRAM_PREFIX@heretic_SOURCES=$(SOURCE_FILES_WITH_DEH) $(top_builddir)/data/heretic_icon.c heretic/resource.rc
else
@PROGRAM_PREFIX@heretic_SOURCES=$(SOURCE_FILES_WITH_DEH) $(top_builddir)/data/heretic_icon.c
endif

@PROGRAM_PREFIX@heretic_LDADD = heretic/libheretic.a $(EXTRA_LIBS)

if HAVE_WINDRES
@PROGRAM_PREFIX@hexen_SOURCES=$(SOURCE_FILES) $(top_builddir)/data/hexen_icon.c hexen/resource.rc
else
@PROGRAM_PREFIX@hexen_SOURCES=$(SOURCE_FILES) $(top_builddir)/data/hexen_icon.c
endif

@PROGRAM_PREFIX@hexen_LDADD = hexen/libhexen.a $(EXTRA_LIBS)

# [Dasperal] Disable "strife" from the compilation
# if HAVE_WINDRES
# @PROGRAM_PREFIX@strife_SOURCES=$(SOURCE_FILES_WITH_DEH) $(top_builddir)/data/strife_icon.c strife/resource.rc
# else
# @PROGRAM_PREFIX@strife_SOURCES=$(SOURCE_FILES_WITH_DEH) $(top_builddir)/data/strife_icon.c
# endif

# @PROGRAM_PREFIX@strife_LDADD = strife/libstrife.a $(EXTRA_LIBS)

# Source files needed for russian-setup:

# SETUP_FILES=                               \
# deh_str.c            deh_str.h             \
# d_mode.c             d_mode.h              \
# d_iwad.c             d_iwad.h              \
#                      icon.h                \
# i_timer.c            i_timer.h             \
# m_config.c           m_config.h            \
# net_io.c             net_io.h              \
# net_packet.c         net_packet.h          \
# net_sdl.c            net_sdl.h             \
# net_query.c          net_query.h           \
# net_structrw.c       net_structrw.h        \
# z_native.c           z_zone.h

# SETUP_LIBS=                                         \
# setup/libsetup.a                                    \
# $(top_builddir)/textscreen/libtextscreen.a          \
# @LDFLAGS@ @SDL_LIBS@ @SDLMIXER_LIBS@ @SDLNET_LIBS@

# if HAVE_WINDRES
# @PROGRAM_PREFIX@doom_setup_SOURCES=$(SETUP_FILES) $(COMMON_SOURCE_FILES) i_main.c $(top_builddir)/data/setup_icon.c setup/doom-resource.rc
# else
# @PROGRAM_PREFIX@doom_setup_SOURCES=$(SETUP_FILES) $(COMMON_SOURCE_FILES) i_main.c $(top_builddir)/data/setup_icon.c
# endif

# @PROGRAM_PREFIX@doom_setup_CFLAGS = -D___RD_TARGET_SETUP___
# @PROGRAM_PREFIX@doom_setup_LDADD = $(SETUP_LIBS)

# if HAVE_WINDRES
# @PROGRAM_PREFIX@heretic_setup_SOURCES=$(SETUP_FILES) $(COMMON_SOURCE_FILES) i_main.c $(top_builddir)/data/setup_icon.c setup/heretic-resource.rc
# else
# @PROGRAM_PREFIX@heretic_setup_SOURCES=$(SETUP_FILES) $(COMMON_SOURCE_FILES) i_main.c $(top_builddir)/data/setup_icon.c
# endif

# @PROGRAM_PREFIX@heretic_setup_CFLAGS = -D___RD_TARGET_SETUP___
# @PROGRAM_PREFIX@heretic_setup_LDADD = $(SETUP_LIBS)

# if HAVE_WINDRES
# @PROGRAM_PREFIX@hexen_setup_SOURCES=$(SETUP_FILES) $(COMMON_SOURCE_FILES) i_main.c $(top_builddir)/data/setup_icon.c setup/hexen-resource.rc
# else
# @PROGRAM_PREFIX@hexen_setup_SOURCES=$(SETUP_FILES) $(COMMON_SOURCE_FILES) i_main.c $(top_builddir)/data/setup_icon.c
# endif

# @PROGRAM_PREFIX@hexen_setup_CFLAGS = -D___RD_TARGET_SETUP___
# @PROGRAM_PREFIX@hexen_setup_LDADD = $(SETUP_LIBS)

# [Dasperal] Disable "strife" from the compilation
# if HAVE_WINDRES
# @PROGRAM_PREFIX@strife_setup_SOURCES=$(SETUP_FILES) $(COMMON_SOURCE_FILES) i_main.c $(top_builddir)/data/setup_icon.c setup/strife-resource.rc
# else
# @PROGRAM_PREFIX@strife_setup_SOURCES=$(SETUP_FILES) $(COMMON_SOURCE_FILES) i_main.c $(top_builddir)/data/setup_icon.c
# endif

# @PROGRAM_PREFIX@strife_setup_CFLAGS = -D___RD_TARGET_SETUP___
# @PROGRAM_PREFIX@strife_setup_LDADD = $(SETUP_LIBS)

.rc.o:
	$(WINDRES) $< -o $@
%.o : %.rc
	$(WINDRES) $< -o $@
