AM_CPPFLAGS+=-I$(top_srcdir)/parser/searchparser/include
#AM_LDFLAGS=$(COVERAGE_LDFLAGS)

searchparserdir=$(pkgincludedir)/parser/searchparser
searchparser_HEADERS=

searchparser_HEADERS+=\
    parser/searchparser/include/searchparser/searchparser.h
# noinst_HEADERS+=

lib_LTLIBRARIES+=lib/libsearchparser.la
lib_libsearchparser_la_SOURCES=\
  parser/searchparser/src/searchparser.cpp

lib_libsearchparser_la_LIBADD=$(LLVM_LIBS) $(AM_LIBADD) \
  $(top_builddir)/lib/libgrockerutil.la \
  $(top_builddir)/lib/libsearch_indexer-api.la
lib_libsearchparser_la_CPPFLAGS=\
	$(AM_CPPFLAGS) \
	-I$(top_srcdir)/libgthreads/include \
    $(LLVM_CPPFLAGS)

lib_libsearchparser_la_CXXFLAGS=$(AM_CXXFLAGS) -Wno-strict-aliasing
lib_libsearchparser_la_LDFLAGS=\
	-static\
	$(AM_LDFLAGS)\
    $(LLVM_LDFLAGS)

EXTRA_DIST +=\
    parser/searchparser/test/test_src/simple/gpl-3.0.txt\
    parser/searchparser/test/test_src/simple/helloworld.cpp
