AC_INIT(Russian Doom, 5.0, julian.nechaevsky@outlook.com,
        russian-doom)

BUILD_HERETIC_VERSION="4.3"
BUILD_HEXEN_VERSION="4.1"
BUILD_STRIFE_VERSION="0.0"
PACKAGE_COPYRIGHT="Copyright \\xA9 2016-2021 Julian Nechaevsky. Licensed under GNU General Public License, version 2"

BUILD_DOOM_VERSION=${PACKAGE_VERSION}
AC_DEFINE_UNQUOTED(BUILD_DOOM_VERSION, "$BUILD_DOOM_VERSION", "Doom build version")
AC_DEFINE_UNQUOTED(BUILD_HERETIC_VERSION, "$BUILD_HERETIC_VERSION", "Heretic build version")
AC_DEFINE_UNQUOTED(BUILD_HEXEN_VERSION, "$BUILD_HEXEN_VERSION", "Hexen build version")
AC_DEFINE_UNQUOTED(BUILD_STRIFE_VERSION, "$BUILD_STRIFE_VERSION", "Strife build version")

AC_CONFIG_AUX_DIR(autotools)
AC_CANONICAL_HOST

orig_CFLAGS="$CFLAGS"

AC_PROG_CC
AC_PROG_RANLIB
AC_CHECK_PROG(HAVE_PYTHON, python, true, false)

OPT_LEVEL=2

# If this is gcc, we have some options we'd like to turn on.  Turn on 
# optimisation and debugging symbols.

if test "$GCC" = "yes"
then
        WARNINGS="-Wall -Wdeclaration-after-statement -Wredundant-decls -Wno-unknown-pragmas"
        CFLAGS="-O$OPT_LEVEL -g $WARNINGS $orig_CFLAGS"
fi

PKG_CHECK_MODULES(SDL, [sdl2 >= 2.0.1])
PKG_CHECK_MODULES(SDLMIXER, [SDL2_mixer >= 2.0.0])
PKG_CHECK_MODULES(SDLNET, [SDL2_net >= 2.0.0])

# Check for libsamplerate.
AC_ARG_WITH([libsamplerate],
AS_HELP_STRING([--without-libsamplerate],
    [Build without libsamplerate @<:@default=check@:>@]),
[],
[
    [with_libsamplerate=check]
])
AS_IF([test "x$with_libsamplerate" != xno], [
    PKG_CHECK_MODULES(SAMPLERATE, samplerate >= 0.1.8, [
        AC_DEFINE([HAVE_LIBSAMPLERATE], [1], [libsamplerate installed])
    ], [
        AS_IF([test "x$with_libsamplerate" != xcheck], [AC_MSG_FAILURE(
            [--with-libsamplerate was given, but test for libsamplerate failed])
        ])
    ])
])

# Check for libpng.
AC_ARG_WITH([libpng],
AS_HELP_STRING([--without-libpng],
    [Build without libpng @<:@default=check@:>@]),
[],
[
    [with_libpng=check]
])
AS_IF([test "x$with_libpng" != xno], [
    PKG_CHECK_MODULES(PNG, libpng >= 1.6.10, [
        AC_DEFINE([HAVE_LIBPNG], [1], [libpng installed])
    ], [
        AS_IF([test "x$with_libpng" != xcheck], [AC_MSG_FAILURE(
            [--with-libpng was given, but test for libpng failed])
        ])
    ])
])

# TODO: We currently link everything against libraries that don't need it.
# Use the specific library CFLAGS/LIBS variables instead of setting them here.
CFLAGS="$CFLAGS $SDL_CFLAGS ${SAMPLERATE_CFLAGS:-} ${PNG_CFLAGS:-}"
LDFLAGS="$LDFLAGS $SDL_LIBS ${SAMPLERATE_LIBS:-} ${PNG_LIBS:-}"
AC_CHECK_LIB(m, log)

AC_CHECK_HEADERS([linux/kd.h dev/isa/spkrio.h dev/speaker/speaker.h])
AC_CHECK_FUNCS(mmap ioperm)
AC_CHECK_DECLS([strcasecmp, strncasecmp], [], [], [[#include <strings.h>]])

# OpenBSD I/O i386 library for I/O port access.
# (64 bit has the same thing with a different name!)

AC_CHECK_LIB(i386, i386_iopl)
AC_CHECK_LIB(amd64, amd64_iopl)

case "$host" in
  *-*-mingw* | *-*-cygwin* | *-*-msvc* )
    AC_CHECK_TOOL(WINDRES, windres, )
    ;;
  *)
    WINDRES=
    ;;
esac

AC_CHECK_TOOL(STRIP, strip, )

AM_CONDITIONAL(HAVE_WINDRES, test "$WINDRES" != "")
AM_CONDITIONAL(HAVE_PYTHON, $HAVE_PYTHON)

dnl Automake v1.8.0 is required, please upgrade!

AM_INIT_AUTOMAKE([1.8.0 foreign subdir-objects])
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

WINDOWS_RC_DOOM_VERSION=`echo $BUILD_DOOM_VERSION | sed 's/\./, /g'`
WINDOWS_RC_HERETIC_VERSION=`echo $BUILD_HERETIC_VERSION | sed 's/\./, /g'`
WINDOWS_RC_HEXEN_VERSION=`echo $BUILD_HEXEN_VERSION | sed 's/\./, /g'`
WINDOWS_RC_STRIFE_VERSION=`echo $BUILD_STRIFE_VERSION | sed 's/\./, /g'`

# This controls the prefix added to the start of program names.  For example,
# if this is changed to "lemon-", the programs generated will be named
# lemon-doom, lemon-heretic, etc.

PROGRAM_PREFIX=russian-
PACKAGE_PREFIX=Russian

AC_SUBST(PROGRAM_PREFIX)
AC_DEFINE_UNQUOTED(PROGRAM_PREFIX, "$PROGRAM_PREFIX",
                   Change this when you create your awesome forked version)

AM_CONFIG_HEADER(config.h:config.hin)

AC_SUBST(PACKAGE_PREFIX)
AC_SUBST(BUILD_HERETIC_VERSION)
AC_SUBST(BUILD_HEXEN_VERSION)
AC_SUBST(BUILD_STRIFE_VERSION)
AC_SUBST(BUILD_DOOM_VERSION)
AC_SUBST(WINDOWS_RC_DOOM_VERSION)
AC_SUBST(WINDOWS_RC_HERETIC_VERSION)
AC_SUBST(WINDOWS_RC_HEXEN_VERSION)
AC_SUBST(WINDOWS_RC_STRIFE_VERSION)
AC_SUBST(SDLMIXER_CFLAGS)
AC_SUBST(SDLMIXER_LIBS)

AC_SUBST(SDLNET_CFLAGS)
AC_SUBST(SDLNET_LIBS)

AC_SUBST(ac_aux_dir)
AC_SUBST(PACKAGE_COPYRIGHT)

dnl Shut up the datarootdir warnings.
AC_DEFUN([AC_DATAROOTDIR_CHECKED])

AC_OUTPUT([
Makefile
midiproc/Makefile
opl/Makefile
pcsound/Makefile
data/Makefile
src/Makefile
src/doom/Makefile
src/heretic/Makefile
src/hexen/Makefile
src/strife/Makefile
src/doom/resource.rc
src/heretic/resource.rc
src/hexen/resource.rc
src/strife/resource.rc
src/setup/doom-resource.rc
src/setup/heretic-resource.rc
src/setup/hexen-resource.rc
src/setup/strife-resource.rc
src/setup/Makefile
src/setup/setup-manifest.xml
textscreen/Makefile
textscreen/fonts/Makefile
])

