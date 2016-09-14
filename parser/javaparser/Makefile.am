AM_CPPFLAGS+=-I$(top_srcdir)/parser/javaparser/include

javaparserdir=$(pkgincludedir)/parser/javaparser

lib_LTLIBRARIES+=lib/libjavaparser.la
lib_libjavaparser_la_SOURCES=\
  parser/javaparser/src/javaparser.cpp

$(lib_libjavaparser_la_SOURCES): javaparser-api-rule

lib_libjavaparser_la_LIBADD=$(AM_LIBADD) \
  $(top_builddir)/lib/libgrockerutil.la \
  $(top_builddir)/lib/libjavaparser-api.la

lib_libjavaparser_la_CPPFLAGS=$(AM_CPPFLAGS) -I$(top_srcdir)/libgthreads/include #$(LLVM_CPPFLAGS)
lib_libjavaparser_la_CXXFLAGS=$(AM_CXXFLAGS)
lib_libjavaparser_la_LDFLAGS=-static $(AM_LDFLAGS) #$(LLVM_LDFLAGS)

