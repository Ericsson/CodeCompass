AM_CPPFLAGS+=-I$(top_srcdir)/parser/reparser/include
#AM_LDFLAGS=$(COVERAGE_LDFLAGS)

reparserdir=$(pkgincludedir)/parser/reparser
reparser_HEADERS=\
    parser/reparser/include/reparser/reparser.h

lib_LTLIBRARIES+=lib/libreparser.la
lib_libreparser_la_SOURCES=\
  parser/reparser/src/reparser.cpp

lib_libreparser_la_LIBADD=$(LLVM_LIBS) $(AM_LIBADD) $(top_builddir)/lib/libgrockerutil.la
lib_libreparser_la_CPPFLAGS=$(AM_CPPFLAGS) $(LLVM_CPPFLAGS)
lib_libreparser_la_CXXFLAGS=$(AM_CXXFLAGS)
lib_libreparser_la_LDFLAGS=$(AM_LDFLAGS) $(LLVM_LDFLAGS)
