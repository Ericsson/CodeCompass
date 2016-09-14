AM_CPPFLAGS+=-I$(top_srcdir)/parser/metricsparser/include

metricsparserdir=$(pkgincludedir)/parser/metricsparser
metricsparser_HEADERS= \
    parser/metricsparser/include/metricsparser/metricsparser.h 

lib_LTLIBRARIES+=lib/libmetricsparser.la
lib_libmetricsparser_la_SOURCES=\
  parser/metricsparser/src/metricsparser.cpp 

lib_libmetricsparser_la_LIBADD= \
     $(AM_LIBADD) \
     $(top_builddir)/lib/libgrockerutil.la
lib_libmetricsparser_la_CPPFLAGS= $(AM_CPPFLAGS)
lib_libmetricsparser_la_CXXFLAGS=$(AM_CXXFLAGS) -Wno-strict-aliasing
lib_libmetricsparser_la_LDFLAGS= -static $(AM_LDFLAGS)
