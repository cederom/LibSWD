SUBDIRS = src
ACLOCAL_AMFLAGS = -I m4

include $(top_srcdir)/aminclude.am
EXTRA_DIST = Doxyfile
MOSTLYCLEANFILES = DX_CLEANFILES

dist_pkgdata_DATA = \
	README.md \
	COPYING \
	ChangeLog \
	NEWS

uninstall-hook:
	rm -rf $(DESTDIR)$(pkgdatadir)

distclean-local:
	rm -rf doxygen-doc/html doxygen-doc/latex doxygen-doc/libswd.pdf
