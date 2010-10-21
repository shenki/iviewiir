dnl configure.ac for iviewiir

AC_INIT([iviewiir], [0.1])
AC_PREREQ(2.65)
AC_CONFIG_SRCDIR([iviewiir.c])
AC_CONFIG_AUX_DIR([.])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_FILES([Makefile])
AM_INIT_AUTOMAKE([1.11.1 foreign subdir-objects])
AC_PROG_CC
AC_PROG_INSTALL
AC_C_CONST
AM_PROG_LIBTOOL
AC_CHECK_LIB([neon], [ne_get_response_header], ,
                          AC_MSG_ERROR([neon headers not found.]))
AC_CHECK_LIB([json], [json_tokener_parse], ,
                          AC_MSG_ERROR([json-c headers not found.]))
AC_CHECK_LIB([rtmp], [RTMP_Alloc], ,
                          AC_MSG_ERROR([RTMP headers not found.]))
AM_CONFIG_HEADER(config.h)
AC_OUTPUT
