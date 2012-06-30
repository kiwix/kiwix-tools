bin_PROGRAMS=kiwix-read
AM_CPPFLAGS=-I$(top_srcdir)/src/common -I$(top_srcdir)/src/zimlib/include
kiwix_read_SOURCES= \
	kiwix-read.cpp \
	../common/kiwix/reader.cpp \
	../common/kiwix/reader.h
kiwix_read_CXXFLAGS=$(ZIMLIB_CFLAGS) $(LZMA_CFLAGS) -I../common/

kiwix_read_LDFLAGS=$(CHECK_LDFLAGS)

if STATIC_BINS
kiwix_read_LDFLAGS+= $(ZIMLIB_STATIC_LDFLAGS) $(top_builddir)/src/zimlib/src/.libs/libzim.a $(LZMA_STATIC_LDFLAGS) $(STATIC_LIBM_ARCHIVE) $(STATIC_LIBSTDCPP_ARCHIVE) --static
else
kiwix_read_LDFLAGS+= $(ZIMLIB_LDFLAGS) $(top_builddir)/src/zimlib/src/.libs/libzim.a $(LZMA_LDFLAGS)
endif
