dnl configure.ac for iviewiir

AC_INIT([iviewiir], [0.1])
AC_PREREQ(2.65)
AC_CONFIG_SRCDIR([iviewiir.c])
AC_CONFIG_AUX_DIR([.])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_FILES([Makefile])
AM_INIT_AUTOMAKE([1.11.1 foreign subdir-objects])
AM_SILENT_RULES([yes])
AC_LANG([C])
AC_PROG_CC
AC_PROG_INSTALL
AC_C_CONST
AM_PROG_LIBTOOL
PKG_CHECK_MODULES([DEPS], [neon json librtmp])
AM_CONFIG_HEADER(config.h)
AC_OUTPUT
