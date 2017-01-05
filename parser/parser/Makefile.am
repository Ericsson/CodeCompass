AM_CPPFLAGS+=-I$(top_srcdir)/parser/parser/include

parserdir=$(pkgincludedir)/parser/parser/
parser_HEADERS=\
  parser/parser/include/parser/parser.h\
  parser/parser/include/parser/projectparser.h\
  parser/parser/include/parser/sourcemanager.h\
  parser/parser/include/parser/traversal.h\
  parser/parser/include/parser/commondefs.h\
  parser/parser/include/parser/parsertest.h\
  parser/parser/include/parser/crashprotect.h
lib_LTLIBRARIES+=lib/libparser.la
lib_libparser_la_SOURCES=\
  parser/parser/src/parser.cpp\
  parser/parser/src/projectparser.cpp\
  parser/parser/src/sourcemanager.cpp\
  parser/parser/src/actionprocessor.cpp\
  parser/parser/src/actionprocessor.h\
  parser/parser/src/traversal.cpp\
  parser/parser/src/crashprotect.cpp

$(lib_libparser_la_SOURCES) : $(lib_libmodel_la_ALLSOURCES)
#EXTRA_lib_libparser_la_DEPENDENCIES= $(top_builddir)/lib/libmodel.la

lib_libparser_la_LIBADD = $(AM_LIBADD) \
  $(top_builddir)/lib/libmodel.la \
  $(top_builddir)/lib/libsearch_indexer-api.la \
  -lboost_thread \
  -lmagic

lib_libparser_la_CPPFLAGS=$(AM_CPPFLAGS) \
	-I$(top_srcdir)/libgthreads/include \
	-D__STDC_CONSTANT_MACROS \
	-D__STDC_FORMAT_MACROS \
	-D__STDC_LIMIT_MACROS

PARSER_LIBS=\
	$(top_builddir)/lib/libprojectparser.la \
	$(top_builddir)/lib/libparser.la \
	$(top_builddir)/lib/libfileparser.la \
	$(top_builddir)/lib/libcxxparser.la \
	$(top_builddir)/lib/libgrockerutil.la \
	$(top_builddir)/lib/libmodel.la \
	$(top_builddir)/lib/libgitparser.la \
	$(top_builddir)/lib/libsearchparser.la \
	$(top_builddir)/lib/libjavaparser.la \
	$(top_builddir)/lib/libmetricsparser.la

if ENABLE_PYTHON_PARSER
PARSER_LIBS+= $(top_builddir)/lib/libpythonparser.la
endif

bin_PROGRAMS+=parser/bin/parse
parser_bin_parse_SOURCES=parser/parser/src/parse.cpp
parser_bin_parse_LDADD=$(AM_LDADD) \
	$(PARSER_LIBS)\
	-lboost_program_options

parser_bin_parse_LDFLAGS=$(AM_LDFLAGS)
parser_bin_parse_CPPFLAGS=$(AM_CPPFLAGS) -I$(top_srcdir)/parser/cxxparser/include -I$(top_srcdir)/parser/projectparser/include \
	-I$(top_srcdir)/parser/javaparser/include -I$(top_srcdir)/parser/pythonparser/include

bin_PROGRAMS+=parser/bin/query
parser_bin_query_SOURCES=parser/parser/src/query.cpp
parser_bin_query_LDADD=$(AM_LDADD) $(PARSER_LIBS)
parser_bin_query_LDFLAGS=$(AM_LDFLAGS)
parser_bin_query_CPPFLAGS=$(AM_CPPFLAGS) -I$(top_srcdir)/parser/cxxparser/include -I$(top_srcdir)/parser/projectparser/include

bin_PROGRAMS+=parser/bin/dumpclangast
parser_bin_dumpclangast_SOURCES=parser/parser/src/dumpclangast.cpp
parser_bin_dumpclangast_LDADD=$(AM_LDADD) $(PARSER_LIBS)
parser_bin_dumpclangast_LDFLAGS=$(AM_LDFLAGS)
parser_bin_dumpclangast_CPPFLAGS=$(AM_CPPFLAGS) \
        -D__STDC_CONSTANT_MACROS \
	-D__STDC_FORMAT_MACROS \
	-D__STDC_LIMIT_MACROS

check_PROGRAMS+=test/parser_integration
test_parser_integration_SOURCES=parser/parser/test/parser_integration.cpp
test_parser_integration_LDADD=$(TEST_LIBS) $(AM_LDADD) $(PARSER_LIBS)
test_parser_integration_LDFLAGS=$(TEST_LDFLAGS) $(AM_LDFLAGS)
test_parser_integration_CPPFLAGS=$(AM_CPPFLAGS) $(TEST_CPPFLAGS) \
	-I$(top_srcdir)/parser/cxxparser/include \
	-I$(top_srcdir)/parser/projectparser/include \
	-DTOP_SRCDIR=\"$(abs_top_srcdir)/\" 

check_PROGRAMS+=test/mangling
test_mangling_SOURCES=parser/parser/test/mangling.cpp
test_mangling_LDADD=$(TEST_LIBS) $(AM_LDADD) $(PARSER_LIBS)
test_mangling_LDFLAGS=$(TEST_LDFLAGS) $(AM_LDFLAGS)
test_mangling_CPPFLAGS=$(AM_CPPFLAGS) $(TEST_CPPFLAGS) -I$(top_srcdir)/parser/cxxparser/include \
	-I$(top_srcdir)/parser/projectparser/include \
	-DTOP_SRCDIR=\"$(abs_top_srcdir)/\" 

check_PROGRAMS+=test/mixedtests
test_mixedtests_SOURCES=parser/parser/test/mixedtests.cpp
test_mixedtests_LDADD=$(TEST_LIBS) $(AM_LDADD) $(PARSER_LIBS)
test_mixedtests_LDFLAGS=$(TEST_LDFLAGS) $(AM_LDFLAGS)
test_mixedtests_CPPFLAGS=$(AM_CPPFLAGS) $(TEST_CPPFLAGS) -I$(top_srcdir)/parser/cxxparser/include \
	-I$(top_srcdir)/parser/projectparser/include \
	-DTOP_SRCDIR=\"$(abs_top_srcdir)/\" 

check_PROGRAMS+=test/traversal
test_traversal_SOURCES=parser/parser/test/traversal.cpp
test_traversal_LDADD=$(TEST_LIBS) $(AM_LDADD) $(PARSER_LIBS)
test_traversal_LDFLAGS=$(TEST_LDFLAGS) $(AM_LDFLAGS)
test_traversal_CPPFLAGS=$(AM_CPPFLAGS) $(TEST_CPPFLAGS) -I$(top_srcdir)/parser/cxxparser/include \
	-I$(top_srcdir)/parser/projectparser/include \
	-DTOP_SRCDIR=\"$(abs_top_srcdir)/\" 

check_PROGRAMS+=test/callbacktest
test_callbacktest_SOURCES=parser/parser/test/callbacktest.cpp
test_callbacktest_LDADD=$(TEST_LIBS) $(AM_LDADD) $(PARSER_LIBS)
test_callbacktest_LDFLAGS=$(TEST_LDFLAGS) $(AM_LDFLAGS)
test_callbacktest_CPPFLAGS=$(AM_CPPFLAGS) $(TEST_CPPFLAGS) -I$(top_srcdir)/parser/cxxparser/include \
	-I$(top_srcdir)/parser/projectparser/include \
	-DTOP_SRCDIR=\"$(abs_top_srcdir)/\" 

check_PROGRAMS+=test/srcmanagertest
test_srcmanagertest_SOURCES=parser/parser/test/srcmanagertest.cpp
test_srcmanagertest_LDADD=$(TEST_LIBS) $(AM_LDADD) $(PARSER_LIBS)
test_srcmanagertest_LDFLAGS=$(TEST_LDFLAGS) $(AM_LDFLAGS)
test_srcmanagertest_CPPFLAGS=$(AM_CPPFLAGS) $(TEST_CPPFLAGS) \
	-I$(top_srcdir)/parser/cxxparser/include \
	-I$(top_srcdir)/parser/projectparser/include \
	-DTOP_SRCDIR=\"$(abs_top_srcdir)/\" 
	
check_PROGRAMS+=test/typeusagetest
test_typeusagetest_SOURCES=parser/parser/test/typeusagetest.cpp
test_typeusagetest_LDADD=$(TEST_LIBS) $(AM_LDADD) $(PARSER_LIBS)
test_typeusagetest_LDFLAGS=$(TEST_LDFLAGS) $(AM_LDFLAGS)
test_typeusagetest_CPPFLAGS=$(AM_CPPFLAGS) $(TEST_CPPFLAGS) \
	-I$(top_srcdir)/parser/cxxparser/include \
	-I$(top_srcdir)/parser/projectparser/include \
	-DTOP_SRCDIR=\"$(abs_top_srcdir)/\" 

check_PROGRAMS+=test/functioncalltest
test_functioncalltest_SOURCES=parser/parser/test/functioncalltest.cpp
test_functioncalltest_LDADD=$(TEST_LIBS) $(AM_LDADD) $(PARSER_LIBS)
test_functioncalltest_LDFLAGS=$(TEST_LDFLAGS) $(AM_LDFLAGS)
test_functioncalltest_CPPFLAGS=$(AM_CPPFLAGS) $(TEST_CPPFLAGS) \
	-I$(top_srcdir)/parser/cxxparser/include \
	-I$(top_srcdir)/parser/projectparser/include \
	-DTOP_SRCDIR=\"$(abs_top_srcdir)/\" 

check_PROGRAMS+=test/templatetest
test_templatetest_SOURCES=parser/parser/test/templatetest.cpp
test_templatetest_LDADD=$(TEST_LIBS) $(AM_LDADD) $(PARSER_LIBS)
test_templatetest_LDFLAGS=$(TEST_LDFLAGS) $(AM_LDFLAGS)
test_templatetest_CPPFLAGS=$(AM_CPPFLAGS) $(TEST_CPPFLAGS) \
	-I$(top_srcdir)/parser/cxxparser/include \
	-I$(top_srcdir)/parser/projectparser/include \
	-DTOP_SRCDIR=\"$(abs_top_srcdir)/\"	

check_PROGRAMS+=test/xmlparsertest
test_xmlparsertest_SOURCES=parser/parser/test/xmlparsertest.cpp
test_xmlparsertest_LDADD=$(TEST_LIBS) $(AM_LDADD) $(PARSER_LIBS)
test_xmlparsertest_LDFLAGS=$(TEST_LDFLAGS) $(AM_LDFLAGS)
test_xmlparsertest_CPPFLAGS=$(AM_CPPFLAGS) $(TEST_CPPFLAGS) \
	-I$(top_srcdir)/parser/cxxparser/include \
	-I$(top_srcdir)/parser/projectparser/include \
	-DTOP_SRCDIR=\"$(abs_top_srcdir)/\"

#if ENABLE_PYTHON_PARSER
#
#check_PROGRAMS+=test/pythonparsertest
#test_pythonparsertest_LDADD=$(TEST_LIBS) $(AM_LDADD) $(PARSER_LIBS)
#test_pythonparsertest_LDFLAGS=$(TEST_LDFLAGS) $(AM_LDFLAGS)
#test_pythonparsertest_CPPFLAGS=$(AM_CPPFLAGS) $(TEST_CPPFLAGS) \
#    -DTOP_SRCDIR=\"$(abs_top_srcdir)/\" 
#
#include $(srcdir)/parser/parser/test/pythonparsertest/Makefile.am
#
#endif

MOSTLYCLEANFILES +=\
    var_function_test.sqlite\
    wrflag_test.sqlite\
    local_global_flag_test.sqlite\
    typetest.sqlite\
    functiontest.sqlite \
    mangled_name_test.sqlite \
    parser_integration_test.sqlite \
    featuretest.sqlite \
    srcmanagertest.sqlite \
    qualtypetest.sqlite\
    templatetest.sqlite \
    traversaltest.sqlite \
    searchdummy.sqlite \
    callback_test.sqlite \
    traversal_test.sqlite \
    typeusage.sqlite \
    doccommenttest.sqlite
#    pythonparsertest.sqlite

EXTRA_DIST +=\
    parser/parser/test/test_src/first/a.cpp\
    parser/parser/test/test_src/first/b.cpp\
    parser/parser/test/test_src/type/a.cpp\
    parser/parser/test/test_src/mangledname/a.cpp\
    parser/parser/test/test_src/wr_flag_test/b.cpp\
    parser/parser/test/test_src/tinyxml.xml\
    parser/parser/test/test_src/tinyxml/tinyxml2.h\
    parser/parser/test/test_src/tinyxml/tinyxml2.cpp\
    parser/parser/test/test_src/tinyxml/xmltest.h\
    parser/parser/test/test_src/tinyxml/xmltest.cpp\
    parser/parser/test/test_src/buildxml/empty.xml\
    parser/parser/test/test_src/buildxml/oneaction.xml\
    parser/parser/test/test_src/buildxml/noconfig1.xml\
    parser/parser/test/test_src/buildxml/noconfig2.xml\
    parser/parser/test/test_src/buildxml/twoaction.xml\
    parser/parser/test/test_src/buildxml/noaction.xml\
    parser/parser/test/test_src/buildxml/badroot.xml\
    parser/parser/test/test_src/buildxml/emptyaction.xml\
    parser/parser/test/test_src/functionpointers/funcptrs.cpp\
    parser/parser/test/test_src/functioncalls/functioncalls.cpp\
    parser/parser/test/test_src/typedef/typedef.cpp\
    parser/parser/test/test_src/typeinfo/typeinfo.cpp\
    parser/parser/test/test_src/typeusage/typeusage.cpp\
    parser/parser/test/test_src/qualtype/a.cpp\
    parser/parser/test/test_src/traversal/a.txt\
    parser/parser/test/test_src/traversal/binary\
    parser/parser/test/test_src/traversal/Makefile\
    parser/parser/test/test_src/traversal/README\
    parser/parser/test/test_src/traversal/directory/b.txt\
    parser/parser/test/test_src/template/a.cpp\
    parser/parser/test/test_src/doccomment/a.cpp\
    parser/parser/test/test_src/javatest/main.java\
    parser/parser/test/test_src/javatest/b.java\
    parser/parser/test/test_src/javatest/Makefile


