
SUBDIRS = doom heretic hexen strife setup

execgamesdir = ${bindir}

execgames_PROGRAMS = @PROGRAM_PREFIX@doom     \
                     @PROGRAM_PREFIX@heretic  \
                     @PROGRAM_PREFIX@hexen    \
                     @PROGRAM_PREFIX@strife


noinst_PROGRAMS = @PROGRAM_PREFIX@setup

SETUP_BINARIES = @PROGRAM_PREFIX@doom-setup$(EXEEXT)    \
                 @PROGRAM_PREFIX@heretic-setup$(EXEEXT) \
                 @PROGRAM_PREFIX@hexen-setup$(EXEEXT)   \
                 @PROGRAM_PREFIX@strife-setup$(EXEEXT)

execgames_SCRIPTS = $(SETUP_BINARIES)

AM_CFLAGS = -I$(top_srcdir)/textscreen            \
            -I$(top_srcdir)/opl                   \
            -I$(top_srcdir)/pcsound               \
            @SDLMIXER_CFLAGS@ @SDLNET_CFLAGS@

# Common source files used by absolutely everything:

COMMON_SOURCE_FILES=\
                     crispy.h              \
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
                     d_ticcmd.h            \
deh_str.c            deh_str.h             \
gusconf.c            gusconf.h             \
i_cdmus.c            i_cdmus.h             \
                     icon.h                \
i_endoom.c           i_endoom.h            \
i_input.c            i_input.h             \
i_joystick.c         i_joystick.h          \
                     i_swap.h              \
i_midipipe.c         i_midipipe.h          \
i_pcsound.c                                \
i_sdlsound.c                               \
i_sdlmusic.c                               \
i_oplmusic.c                               \
i_sound.c            i_sound.h             \
i_timer.c            i_timer.h             \
i_video.c            i_video.h             \
i_videohr.c          i_videohr.h           \
midifile.c           midifile.h            \
mus2mid.c            mus2mid.h             \
m_bbox.c             m_bbox.h              \
m_cheat.c            m_cheat.h             \
m_config.c           m_config.h            \
m_controls.c         m_controls.h          \
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

if HAVE_WINDRES
@PROGRAM_PREFIX@strife_SOURCES=$(SOURCE_FILES_WITH_DEH) $(top_builddir)/data/strife_icon.c strife/resource.rc
else
@PROGRAM_PREFIX@strife_SOURCES=$(SOURCE_FILES_WITH_DEH) $(top_builddir)/data/strife_icon.c
endif

@PROGRAM_PREFIX@strife_LDADD = strife/libstrife.a $(EXTRA_LIBS)

$(SETUP_BINARIES): @PROGRAM_PREFIX@setup$(EXEEXT)
	cp @PROGRAM_PREFIX@setup$(EXEEXT) $@

# Source files needed for russian-setup:

SETUP_FILES=                               \
deh_str.c            deh_str.h             \
d_mode.c             d_mode.h              \
d_iwad.c             d_iwad.h              \
                     icon.h                \
i_timer.c            i_timer.h             \
m_config.c           m_config.h            \
m_controls.c         m_controls.h          \
net_io.c             net_io.h              \
net_packet.c         net_packet.h          \
net_sdl.c            net_sdl.h             \
net_query.c          net_query.h           \
net_structrw.c       net_structrw.h        \
z_native.c           z_zone.h

if HAVE_WINDRES
@PROGRAM_PREFIX@setup_SOURCES=$(SETUP_FILES) $(COMMON_SOURCE_FILES) i_main.c $(top_builddir)/data/setup_icon.c setup-res.rc
else
@PROGRAM_PREFIX@setup_SOURCES=$(SETUP_FILES) $(COMMON_SOURCE_FILES) i_main.c $(top_builddir)/data/setup_icon.c
endif
@PROGRAM_PREFIX@setup_LDADD = setup/libsetup.a                             \
                        $(top_builddir)/textscreen/libtextscreen.a   \
                        @LDFLAGS@ @SDL_LIBS@ @SDLMIXER_LIBS@ @SDLNET_LIBS@

CLEANFILES = $(execgames_SCRIPTS)

.rc.o:
	$(WINDRES) $< -o $@
%.o : %.rc
	$(WINDRES) $< -o $@
