pkglib_LTLIBRARIES=libZimAccessor.la
pkglibdir=$(libdir)/kiwix/components/ctype
AM_CPPFLAGS=-I$(top_srcdir)/src/common -I$(top_srcdir)/src/zimlib/include
libZimAccessor_la_SOURCES= \
	zimAccessor.cpp \
	zimAccessorWrapper.cpp \
	$(top_srcdir)/src/common/kiwix/reader.cpp \
	IZimAccessor.h \
	zimAccessorWrapper.h \
	$(top_srcdir)/src/common/kiwix/reader.h \
	$(top_srcdir)/src/common/pathTools.h \
	$(top_srcdir)/src/common/pathTools.cpp \
libZimAccessor_la_CXXFLAGS=$(LIBZIM_CFLAGS) -I../../common/
if IS_STATIC
libZimAccessor_la_LDFLAGS=$(LIBZIM_STATIC_LDFLAGS) $(LIBZIMSELF_STATIC_LDFLAGS) $(LIBLZMA_STATIC_LDFLAGS)
else
libZimAccessor_la_LDFLAGS=$(LIBZIM_LDFLAGS)
libZimAccessor_la_LIBADD=$(top_builddir)/src/zimlib/src/libzim.la
endif
if IS_OSX
libZimAccessor_la_LDFLAGS=$(LIBZIM_STATIC_LDFLAGS) $(LIBZIMSELF_STATIC_LDFLAGS)
endif
CLEANFILES=*.lib *.obj *.dll
