AC_INIT([libswd], [GIT-][m4_esyscmd_s([git rev-parse --short HEAD])], [tomek@cedro.info])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])

AC_ARG_ENABLE([application],
 AS_HELP_STRING([--enable-application],
  [build the LibSWD Application (default: no)]),
  [case "${enableval}" in
   yes) enable_application=true ;;
   no)  enable_application=false ;;
   *)   AC_MSG_ERROR([bad value ${enableval} for --enable-application]) ;;
  esac],
  [enable_application=false]
)
AM_CONDITIONAL([APPLICATION], [test x"$enable_application" = x"true"])
LIBREADLINE=
AS_IF([test x"$enable_application" == x"true"],
  [
   AC_CHECK_LIB([readline], [readline],
     [AC_SUBST([LIBREADLINE], ["-lreadline"])
      AC_DEFINE([HAVE_READLINE], [1], [Define if you have libreadline])],
     [AC_MSG_FAILURE([Readline not found, required by LibSWD Application!])]
   )
  ]
)
LIBFTDI=
AS_IF([test x"$enable_application" == x"true"],
  [
   AC_CHECK_LIB([ftdi], [ftdi_new],
     [AC_SUBST([LIBFTDI], ["-lftdi"])
      AC_DEFINE([HAVE_FTDI], [1], [Define if you have libftdi])],
   )
  ]
)
LIBFTDI1=
AS_IF([test x"$enable_application" == x"true"],
  [
   AC_CHECK_LIB([ftdi1], [ftdi_new],
     [AC_SUBST([LIBFTDI], ["-lftdi1"])
      AC_DEFINE([HAVE_FTDI1], [1], [Define if you have libftdi1])],
   )
  ]
)
LIBUSB=
AS_IF([test x"$enable_application" == x"true"],
  [
   AS_IF(
   [test `uname` == "FreeBSD"],
   [
    AC_CHECK_LIB([usb], [libusb_init],
      [AC_SUBST([LIBUSB], ["-lusb"])
       AC_DEFINE([HAVE_USB1], [1], [Define if you have libusb-1])]
      ,[AC_MSG_WARN([LibUSB-1 not found, no async USB I/O available...])]
    )
   ],
   [
    AC_CHECK_LIB([usb-1.0], [libusb_init],
      [AC_SUBST([LIBUSB], ["-lusb-1.0"])
       AC_DEFINE([HAVE_USB1], [1], [Define if you have libusb-1])]
      ,[AC_MSG_WARN([LibUSB-1 not found, no async USB I/O available...])]
    )
   ]
  )
 ]
)
#Check if we have LibFTDI. If using LibFTDI1 also verify LibUSB1 presence.
AS_IF([test x"$enable_application" == x"true"],
 [
  AC_CHECK_DECL(HAVE_FTDI1,
   [
    AC_CHECK_DECL(HAVE_USB1,
      [AC_MSG_NOTICE([LibFTDI1+LibUSB1 found, async USB I/O available!])],
      [AC_MSG_FAILURE([LibFTDI1 requires LibUSB1, but it was not found!])]
    )
   ],
   [
    AC_CHECK_DECL(HAVE_FTDI,
     [AC_MSG_NOTICE([Old LibFTDI found, no async USB I/O available!])],
     [AC_MSG_FAILURE([Neither LibFTDI nor LibFTDI1 was found, but required by LibSWD Application!])]
    )
   ]
  )
 ]
)

AC_ARG_ENABLE(debug,
 AS_HELP_STRING([--enable-debug], [build with Debug Symbols (default: no)]),
 [case "${enableval}" in
   yes) debug=true ;;
   no)  debug=false ;;
   *)   AC_MSG_ERROR([bad value ${enableval} for --enable-debug]) ;;
  esac],
 [debug=false]
)
AM_CONDITIONAL(DEBUG, test x"$debug" = x"true")

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
