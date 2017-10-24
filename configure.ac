#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([evheap], [m4_esyscmd([./version.sh])])
AC_CONFIG_SRCDIR([src/main.c])
AC_CONFIG_HEADER([src/evheap_config.h])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([foreign no-dist no-installinfo no-installman -Wall])

#
# Checks for programs.
#
AC_PROG_CC(clang llvm-gcc gcc)
AC_PROG_CC_C99


#
# set prefix path
#
AS_IF([test "$prefix" = "NONE"],
    [ AC_SUBST([PKGROOT], ["$ac_default_prefix"] ) ],
    [ AC_SUBST([PKGROOT], ["$prefix"] ) ]
)

#
# Checks for typedefs, structures, and compiler characteristics.
#
AC_C_CONST
AC_C_TYPEOF
AC_CHECK_TYPES([int8_t, int16_t, int32_t, int64_t,
                uint8_t, uint16_t, uint32_t, uint64_t,
                intptr_t, uintptr_t, ptrdiff_t,
                size_t, ssize_t, off_t, mode_t, pid_t ])

#
# Checks for header files.
#
AC_HEADER_STDC

#
# Checks for library functions.
#
AC_FUNC_REALLOC
AC_CHECK_FUNCS([ \
    ctime_r \
    setsockopt \
])

#
# debug option
#
AC_ARG_ENABLE(
    [debug],
    AS_HELP_STRING([--enable-debug], [enable debug build.]),
    [ENABLE_DEBUG=$enableval], [ENABLE_DEBUG=no]
)
AS_IF([test "$ENABLE_DEBUG" != "no"],
    [ AC_DEFINE([DEBUG], [1], [Define if you enabled debug]) ],
    [ AC_SUBST([LDFLAGS], ["-all-static $LDFLAGS"])
      AC_SUBST([PKG_CONFIG], ["${PKG_CONFIG} --static"])
      AC_SUBST([CFLAGS], ["-O3"]) ]
)


#
# set flags
#
AC_SUBST([CPPFLAGS], ["-fvisibility=hidden $CPPFLAGS"])

#
# warnings
#
AC_SUBST([WARNINGS], ["-Wall -Wno-trigraphs -Wmissing-field-initializers -Wmissing-prototypes -Wreturn-type -Wmissing-braces -Wparentheses -Wswitch -Wunused-function -Wunused-label -Wunused-parameter -Wunused-variable -Wunused-value -Wuninitialized -Wunknown-pragmas -Wshadow -Wsign-compare -Wextra -Wwrite-strings -Wcast-qual -Wredundant-decls -Wmissing-declarations -Wpointer-arith -Wstrict-prototypes -Wnested-externs -Wformat=2"])


AC_CONFIG_FILES([Makefile
                 src/Makefile \
                 test/Makefile])

AC_OUTPUT
