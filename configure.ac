##
## Copyright (c) 2009 Alexey Smirnov <alexey.smirnov@gmx.com>
## Copyright (c) 2009 Sergey I. Sharybin <g.ulairi@gmail.com>
##
## See the file LICENSE for information on usage and redistribution
## of this file, and for a DISCLAMER OF ALL WARRANTIES.
##

m4_define([VERSION_NUMBER], [0.0.1])

AC_PREREQ(2.61)
AC_INIT([xlockmost], [VERSION_NUMBER], [g.ulairi@gmail.com])
AM_INIT_AUTOMAKE([test], [VERSION_NUMBER])

AC_CONFIG_HEADER(config.h)

AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_RANLIB

AC_HEADER_STDC

AC_PATH_X

AC_CHECK_HEADERS([signal.h pwd.h string.h],,
    [
     AC_MSG_ERROR([missing required header (see above)])
    ])

AC_ARG_ENABLE(debug,
    AC_HELP_STRING([--enable-debug], [enable debugging code]),
    [if test "x$enableval" = "xyes"; then
        AC_DEFINE(HAVE_DEBUG, [1], [Define to enable debugging])
        CFLAGS="-Wall -g -O0"
     fi
    ])

AC_FUNC_VPRINTF

AC_TYPE_UID_T
AC_TYPE_SIGNAL

AC_OUTPUT([Makefile
           mk/rules.mk
           src/Makefile])
