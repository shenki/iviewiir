dnl configure.ac for iviewiir

AC_INIT([iviewiir], [0.1])
AC_PREREQ(2.65)
AC_CONFIG_SRCDIR([src/iviewiir.c])
AC_CONFIG_AUX_DIR([config])
AC_CONFIG_FILES([Makefile])
AM_INIT_AUTOMAKE([1.11.1])
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_RANLIB
AC_C_CONST
AC_C_INLINE
AC_CHECK_LIB([neon], [ne_get_response_header], ,
             AC_MSG_ERROR([neon headers not found.]))
AC_CHECK_LIB([json], [json_tokener_parse], ,
             AC_MSG_ERROR([json-c headers not found.]))
AM_CONFIG_HEADER(config.h)
AC_OUTPUT
