FONT_HDRS = small.h small_en.h normal.h normal_en.h large.h large_en.h

noinst_DATA = $(FONT_HDRS)

if HAVE_PYTHON

small.h: small.png convert-font
	./convert-font small small.png small.h

small_en.h: small_en.png convert-font
	./convert-font small_en small_en.png small_en.h

normal.h: normal.png convert-font
	./convert-font normal normal.png normal.h

normal_en.h: normal_en.png convert-font
	./convert-font normal_en normal_en.png normal_en.h

large.h: large.png convert-font
	./convert-font large large.png large.h

large_en.h: large_en.png convert-font
	./convert-font large_en large_en.png large_en.h

endif
