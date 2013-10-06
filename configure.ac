AC_INIT([libswd], [master-GIT-devel], [tomek@cedro.info])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])
AC_ARG_ENABLE([application],
 AS_HELP_STRING([--disable-application], [do not build the LibSWD Application]),
  [],
  [application=yes]
)
AM_CONDITIONAL([APPLICATION], [test x$application = xyes])
LIBREADLINE=
AS_IF([test "x$enable_application" != xno],
  [
   AC_CHECK_LIB([readline], [readline],
     [AC_SUBST([LIBREADLINE], ["-lreadline"])
      AC_DEFINE([HAVE_READLINE], [1], [Define if you have libreadline])],
     [AC_MSG_FAILURE([Readline not found, required by LibSWD Application!])]
   )
  ]
)
LIBFTDI=
AS_IF([test "x$enable_application" != xno],
  [
   AC_CHECK_LIB([ftdi], [ftdi_new], 
     [AC_SUBST([LIBFTDI], ["-lftdi"])
      AC_DEFINE([HAVE_FTDI], [1], [Define if you have libftdi])],
     [AC_MSG_FAILURE([LibFTDI not found, required by LibSWD Application!])]
   )
  ]
)
AM_INIT_AUTOMAKE([foreign -Wall -Werror])
DX_PDF_FEATURE(ON)
DX_HTML_FEATURE(ON)
DX_PS_FEATURE(OFF)
DX_MAN_FEATURE(OFF)
DX_INIT_DOXYGEN($PACKAGE_STRING)
m4_pattern_allow([AM_PROG_AR])
AM_PROG_AR
AC_PROG_CC
LT_INIT
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile src/Makefile])
AC_OUTPUT
