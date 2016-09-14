AM_CPPFLAGS+=-I$(top_srcdir)/parser/pythonparser/include
#AM_LDFLAGS=$(COVERAGE_LDFLAGS)

pythonparserdir=$(pkgincludedir)/parser/pythonparser
pythonparser_HEADERS=

pythonparser_HEADERS+=\
    parser/pythonparser/include/pythonparser/pythonparser.h
# noinst_HEADERS+=

lib_LTLIBRARIES+=lib/libpythonparser.la
lib_libpythonparser_la_SOURCES=\
  parser/pythonparser/src/pythonparser.cpp

lib_libpythonparser_la_LIBADD=$(LLVM_LIBS) $(AM_LIBADD) \
  $(top_builddir)/lib/libgrockerutil.la \
  $(top_builddir)/lib/libpythonparser-api.la
lib_libpythonparser_la_CPPFLAGS=\
	$(AM_CPPFLAGS) \
	-I$(top_srcdir)/libgthreads/include \
    $(LLVM_CPPFLAGS)

lib_libpythonparser_la_CXXFLAGS=$(AM_CXXFLAGS) -Wno-strict-aliasing
lib_libpythonparser_la_LDFLAGS=-static $(AM_LDFLAGS) $(LLVM_LDFLAGS)

#check_PROGRAMS+=test/pythonparsertest
#test_pythonparsertest_SOURCES=parser/pythonparser/test/pythonparsertest.cpp
#test_pythonparsertest_LDADD=$(TEST_LIBS) $(AM_LDADD) \
#	$(top_builddir)/lib/libpythonparser.la \
#	$(top_builddir)/lib/libprojectparser.la \
#	$(top_builddir)/lib/libparser.la \
#	$(top_builddir)/lib/libfileparser.la \
#	$(top_builddir)/lib/libcxxparser.la \
#	$(top_builddir)/lib/libgrockerutil.la \
#	$(top_builddir)/lib/libmodel.la
#test_pythonparsertest_LDFLAGS=$(TEST_LDFLAGS) $(AM_LDFLAGS) $(LLVM_LDFLAGS)
#test_pythonparsertest_CPPFLAGS=$(AM_CPPFLAGS) $(TEST_CPPFLAGS) $(LLVM_CPPFLAGS) \
#	-I$(top_srcdir)/parser/projectparser/include \
#	-I$(top_srcdir)/parser/pythonparser/include \
#	-DTOP_SRCDIR=\"$(abs_top_srcdir)/\"

#EXTRA_DIST +=\
 #   parser/pythonparser/test/test_src/simple/gpl-3.0.txt\
#    parser/pythonparser/test/test_src/simple/helloworld.cpp

# check_PROGRAMS+=test/parser_integration
# test_parser_integration_SOURCES=parser/tests/parser_integration.cpp
# test_parser_integration_LDADD=$(TEST_LIBS) $(AM_LDADD) -L$(top_srcdir)/lib/libgeneralprojectparser.la -L$(top_srcdir)/lib/libparser.la -L$(top_srcdir)/lib/libpythonparser.la
# test_parser_integration_LDFLAGS=$(TEST_LDFLAGS) $(AM_LDFLAGS)
# test_parser_integration_CPPFLAGS=$(AM_CPPFLAGS) $(TEST_CPPFLAGS) -I$(top_srcdir)/pythonparser/include -I$(top_srcdir)/projectparser/
