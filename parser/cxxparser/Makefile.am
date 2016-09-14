AM_CPPFLAGS+=-I$(top_srcdir)/parser/cxxparser/include
#AM_LDFLAGS=$(COVERAGE_LDFLAGS)

cxxparserdir=$(pkgincludedir)/parser/cxxparser
cxxparser_HEADERS=

cxxparser_HEADERS+=\
    parser/cxxparser/include/cxxparser/nullparser.h\
    parser/cxxparser/include/cxxparser/cxxparser.h\
    parser/cxxparser/include/cxxparser/internal/filelocutilbase.h\
    parser/cxxparser/include/cxxparser/cxxastpersister.h
# noinst_HEADERS+=

lib_LTLIBRARIES+=lib/libcxxparser.la
lib_libcxxparser_la_SOURCES=\
  parser/cxxparser/src/documentationcommentcollector.h\
  parser/cxxparser/src/documentationcommentformatter.cpp\
  parser/cxxparser/src/documentationcommentformatter.h\
  parser/cxxparser/src/nullparser.cpp\
  parser/cxxparser/src/cxxparser.cpp\
  parser/cxxparser/src/assignmentcollector.h\
  parser/cxxparser/src/declcontextvisitor.h\
  parser/cxxparser/src/clangastvisitor.h\
  parser/cxxparser/src/cxxparsesession.h\
  parser/cxxparser/src/nodeinfocollector.h\
  parser/cxxparser/src/relationcollector.h\
  parser/cxxparser/src/refcollector.h\
  parser/cxxparser/src/metricscollector.h\
  parser/cxxparser/src/filelocutil.h\
  parser/cxxparser/src/filelocutil.cpp\
  parser/cxxparser/src/opnames.h\
  parser/cxxparser/src/opnames.cpp\
  parser/cxxparser/src/messagehandler.h\
  parser/cxxparser/src/messagehandler.cpp\
  parser/cxxparser/src/symbolhelper.h\
  parser/cxxparser/src/symbolhelper.cpp\
  parser/cxxparser/src/ccppcallbacks.h\
  parser/cxxparser/src/ccppcallbacks.cpp\
  parser/cxxparser/src/ccfrontendaction.h\
  parser/cxxparser/src/ccfrontendaction.cpp\
  parser/cxxparser/src/ccastconsumer.h\
  parser/cxxparser/src/ccastconsumer.cpp\
  parser/cxxparser/src/cxxastpersister.cpp\
  parser/cxxparser/src/parseutil.h\
  parser/cxxparser/src/parseutil.cpp\
  parser/cxxparser/src/tracer.h\
  parser/cxxparser/src/tracer.cpp\
  parser/cxxparser/src/parameters.h\
  parser/cxxparser/src/parameters.cpp

lib_libcxxparser_la_LIBADD=$(LLVM_LIBS) $(AM_LIBADD) $(top_builddir)/lib/libgrockerutil.la
lib_libcxxparser_la_CPPFLAGS=$(AM_CPPFLAGS) -I$(top_srcdir)/libgthreads/include $(LLVM_CPPFLAGS)
lib_libcxxparser_la_CXXFLAGS=$(AM_CXXFLAGS) -Wno-strict-aliasing
lib_libcxxparser_la_LDFLAGS=-static $(AM_LDFLAGS) $(LLVM_LDFLAGS)

# check_PROGRAMS+=test/parser_integration
# test_parser_integration_SOURCES=parser/tests/parser_integration.cpp
# test_parser_integration_LDADD=$(TEST_LIBS) $(AM_LDADD) -L$(top_srcdir)/lib/libgeneralprojectparser.la -L$(top_srcdir)/lib/libparser.la -L$(top_srcdir)/lib/libcxxparser.la
# test_parser_integration_LDFLAGS=$(TEST_LDFLAGS) $(AM_LDFLAGS)
# test_parser_integration_CPPFLAGS=$(AM_CPPFLAGS) $(TEST_CPPFLAGS) -I$(top_srcdir)/cxxparser/include -I$(top_srcdir)/projectparser/
