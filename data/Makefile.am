
iconsdir = $(prefix)/share/icons
icons_DATA = @PROGRAM_PREFIX@doom.png   \
             @PROGRAM_PREFIX@setup.png

@PROGRAM_PREFIX@doom.png : doom.png
	cp $(top_srcdir)/data/doom.png $@

@PROGRAM_PREFIX@setup.png : setup.png
	cp $(top_srcdir)/data/setup.png $@

CLEANFILES = $(icons_DATA)
