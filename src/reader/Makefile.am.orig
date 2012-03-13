bin_PROGRAMS=kiwix-read
AM_CPPFLAGS=-I$(top_srcdir)/src/common -I$(top_srcdir)/src/zimlib/include
kiwix_read_SOURCES= \
	kiwix-read.cpp \
	../common/kiwix/reader.cpp
include_HEADERS= \
	../common/kiwix/reader.h
kiwix_read_CXXFLAGS=$(LIBZIM_CFLAGS) -I../common/
if IS_STATIC
kiwix_read_LDFLAGS=$(LIBZIM_STATIC_LDFLAGS) $(LIBZIMSELF_STATIC_LDFLAGS) $(LIBLZMA_STATIC_LDFLAGS)
else
kiwix_read_LDFLAGS=$(LIBZIM_LDFLAGS)
kiwix_read_LDADD=$(top_builddir)/src/zimlib/src/libzim.la
endif
