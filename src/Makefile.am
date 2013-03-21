lib_LTLIBRARIES = libswd.la
bin_PROGRAMS = libswd
libswd_la_LDFLAGS = -version-info 0:6:0
include_HEADERS = libswd.h
LDADD = $(LIBOBJS) $(ALLOCA)
EXTRA_DIST = \
 examples/libswd_drv_urjtag.c \
 examples/libswd_drv_openocd.h \
 examples/libswd_drv_openocd.c

libswd_la_SOURCES = \
 libswd.h \
 libswd_bin.c \
 libswd_bitgen.c \
 libswd_bus.c \
 libswd_cmd.c \
 libswd_cmdq.c \
 libswd_core.c \
 libswd_dap.c \
 libswd_drv.c \
 libswd_error.c \
 libswd_log.c \
 libswd_cli.c

libswd_SOURCES = \
 libswd_app.c
libswd_LDADD = -lswd -lreadline -lftdi
