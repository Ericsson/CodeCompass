AM_CPPFLAGS+=-I$(top_srcdir)/parser/fileparser/include

fileparserdir=$(pkgincludedir)/parser/fileparser
fileparser_HEADERS=\
    parser/fileparser/include/fileparser/fileparser.h
# noinst_HEADERS+=

lib_LTLIBRARIES+=lib/libfileparser.la
lib_libfileparser_la_SOURCES=\
  parser/fileparser/src/fileparser.cpp
lib_libfileparser_la_LIBADD=$(AM_LIBADD) $(builddir)/lib/libgrockerutil.la
lib_libfileparser_la_CPPFLAGS=$(AM_CPPFLAGS) -I$(top_srcdir)/libgthreads/include
# 
# check_PROGRAMS+=test/parser_integration
# test_parser_integration_SOURCES=parser/tests/parser_integration.cpp
# test_parser_integration_LDADD=$(TEST_LIBS) $(AM_LDADD) $(top_builddir)/lib/libgeneralprojectparser.la $(top_builddir)/lib/libparser.la $(top_builddir)/lib/libcxxparser.la $(top_builddir)/lib/libgrocker.la  $(top_builddir)/lib/libgrockerutil.la
# test_parser_integration_LDFLAGS=$(TEST_LDFLAGS) $(AM_LDFLAGS)
# test_parser_integration_CPPFLAGS=$(AM_CPPFLAGS) $(TEST_CPPFLAGS) -I$(top_srcdir)/cxxparser/include -I$(top_srcdir)/projectparser/include -DTOP_SRCDIR=\"$(abs_top_srcdir)\"
# 
# EXTRA_DIST +=\
#     test/test_src/\
#     test/test_src/a.cpp
