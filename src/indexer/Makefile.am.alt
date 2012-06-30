bin_PROGRAMS=kiwix-index
AM_CPPFLAGS=-I$(top_srcdir)/src/common -I$(top_srcdir)/src/zimlib/include -I$(top_srcdir)/src/ctpp2/include
kiwix_index_SOURCES= \
	kiwix-index.cpp \
	../common/kiwix/xapian/myhtmlparse.cc \
	../common/kiwix/xapian/htmlparse.cc \
	../common/unaccent.cpp \
	../common/kiwix/indexer.cpp \
	../common/kiwix/reader.cpp \
	../common/kiwix/xapianIndexer.cpp \
	../common/kiwix/cluceneIndexer.cpp \
	../common/kiwix/xapian/myhtmlparse.h \
	../common/kiwix/xapian/namedentities.h \
	../common/kiwix/xapian/htmlparse.h \
	../common/unaccent.h \
	../common/kiwix/indexer.h \
	../common/kiwix/reader.h \
	../common/kiwix/xapianIndexer.h \
	../common/kiwix/cluceneIndexer.h 
kiwix_index_CXXFLAGS=$(XAPIAN_CFLAGS) $(CLUCENE_CFLAGS) $(ICU_CFLAGS) $(LZMA_CFLAGS) $(ZLIB_CFLAGS) -I../common/

kiwix_index_LDFLAGS=$(CHECK_LDFLAGS)

if STATIC_BINS
kiwix_index_LDFLAGS+= $(XAPIAN_STATIC_LDFLAGS) $(CLUCENE_STATIC_LDFLAGS) $(ICU_STATIC_LDFLAGS) ../zimlib/src/.libs/libzim.a $(LZMA_STATIC_LDFLAGS) $(ZLIB_STATIC_LDFLAGS) $(UUID_STATIC_LDFLAGS) $(STATIC_LIBM_ARCHIVE) $(STATIC_LIBSTDCPP_ARCHIVE) $(STATIC_LIBDL_ARCHIVE) $(STATIC_LIBGCC_ARCHIVE) $(STATIC_LIBPTHREAD_ARCHIVE) --static
else
kiwix_index_LDFLAGS+= $(XAPIAN_LDFLAGS) $(CLUCENE_LDFLAGS) $(ICU_LDFLAGS) ../zimlib/src/.libs/libzim.a $(LZMA_LDFLAGS) $(ZLIB_LDFLAGS) -lpthread $(UUID_LDFLAGS) -ldl
endif
