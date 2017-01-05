# service test "framework"
AM_CPPFLAGS+=\
	-I$(srcdir)/service/test/include

EXTRA_DIST +=\
	service/test/include/servicetest/servicetest.h \
	service/test/env/create_databases.sh \
	service/test/sources/simple/simple.cpp \
	service/test/sources/tinyxml/tinyxml2.cpp \
	service/test/sources/tinyxml/tinyxml2.h \
	service/test/sources/tinyxml/xmltest.cpp \
	service/test/sources/tinyxml/xmltest.h \
	service/test/sources/pdg/first.cpp \
	service/test/sources/tinyxml/xmltest.h \
	service/test/sources/simplejava/two/Foo.java \
	service/test/sources/simplejava/two/Bar.java
	

AM_TESTS_ENVIRONMENT+=\
	SERVICE_TEST_DB_DIR="test/tmp_servicetest"; \
	export SERVICE_TEST_DB_DIR; \
	flock srvenv.lock -c $(abs_top_srcdir)/"service/test/env/create_databases.sh \
		\"$(abs_top_builddir)/parser/bin/parse\" \
		\"$(abs_top_srcdir)\" \
		\"test/$(am__dirstamp)\""; 

MOSTLYCLEANFILES+=\
	srvenv.lock

#this is the plugin dir for the codecompass services
webplugindir=$(libdir)/$(PACKAGE)/webplugin
webplugin_LTLIBRARIES=

#this is the plugin dir for the codecompass service mocks
webmockplugindir=$(libdir)/$(PACKAGE)/webmockplugin
webmockplugin_LTLIBRARIES=

#we need this directory so this will be given as an include path for the thrift compiler
CCTHRIFT_INCLUDE=$(srcdir)/service
#we would like to collect the directories in which we put the thrift generated headers
CCTHRIFT_GEN_INCLUDE=

include $(srcdir)/service/core/Makefile.am
include $(srcdir)/service/core-api/Makefile.am
include $(srcdir)/service/language-api/Makefile.am
include $(srcdir)/service/langservicelib/Makefile.am
include $(srcdir)/service/cppservice/Makefile.am
include $(srcdir)/service/javaservice/Makefile.am
include $(srcdir)/service/search-api/Makefile.am
include $(srcdir)/service/search/Makefile.am
include $(srcdir)/service/cppservicehelper/Makefile.am
include $(srcdir)/service/javaservicehelper/Makefile.am
include $(srcdir)/service/workspace-api/Makefile.am
include $(srcdir)/service/workspace/Makefile.am
include $(srcdir)/service/version-api/Makefile.am
include $(srcdir)/service/version/Makefile.am
include $(srcdir)/service/plugin-api/Makefile.am
include $(srcdir)/service/plugin/Makefile.am
include $(srcdir)/service/uscs-api/Makefile.am
include $(srcdir)/service/uscs/Makefile.am
include $(srcdir)/service/codechecker-api/Makefile.am
include $(srcdir)/service/codechecker/Makefile.am
include $(srcdir)/service/metrics-api/Makefile.am
include $(srcdir)/service/metricsservice/Makefile.am

if ENABLE_PYTHON_PARSER
include $(srcdir)/service/pythonservice/Makefile.am
endif

mostlyclean-servicetests:
	rm -fr "test/tmp_servicetest"

check_PROGRAMS+=test/cppservicehelpertest
test_cppservicehelpertest_SOURCES=service/test/src/cppservicehelpertest.cpp
test_cppservicehelpertest_LDADD=$(TEST_LIBS) $(AM_LDADD) \
	$(top_builddir)/lib/libcppservicehelper.la \
	$(top_builddir)/lib/libmodel.la
test_cppservicehelpertest_LDFLAGS=$(TEST_LDFLAGS) $(AM_LDFLAGS)
test_cppservicehelpertest_CPPFLAGS=$(AM_CPPFLAGS) $(TEST_CPPFLAGS) $(AM_CPPFLAGS) $(CCTHRIFT_GEN_INCLUDE)\
	-DTOP_SRCDIR=\"$(abs_top_srcdir)/\" \
	-DTOP_BLDDIR=\"$(abs_top_builddir)/\" 
	
check_PROGRAMS+=test/symbolhandlertest
test_symbolhandlertest_SOURCES=service/test/src/symbolhandlertest.cpp
test_symbolhandlertest_LDADD=$(TEST_LIBS) $(AM_LDADD) \
	$(top_builddir)/lib/libcppservicehelper.la \
	$(top_builddir)/lib/libgrockerutil.la \
	$(top_builddir)/lib/libmodel.la
test_symbolhandlertest_LDFLAGS=$(TEST_LDFLAGS) $(AM_LDFLAGS)
test_symbolhandlertest_CPPFLAGS=$(AM_CPPFLAGS) $(TEST_CPPFLAGS) $(AM_CPPFLAGS) $(CCTHRIFT_GEN_INCLUDE)\
	-DTOP_SRCDIR=\"$(abs_top_srcdir)/\" \
	-DTOP_BLDDIR=\"$(abs_top_builddir)/\" 
	
check_PROGRAMS+=test/reparsertest
test_reparsertest_SOURCES=service/test/src/reparsertest.cpp
test_reparsertest_LDADD=$(TEST_LIBS) $(AM_LDADD) \
	$(top_builddir)/lib/libgrockerutil.la \
	$(top_builddir)/lib/libmodel.la \
	$(top_builddir)/lib/libcppservicehelper.la
test_reparsertest_LDFLAGS=$(TEST_LDFLAGS) $(AM_LDFLAGS)
test_reparsertest_CPPFLAGS=$(AM_CPPFLAGS) $(TEST_CPPFLAGS) $(AM_CPPFLAGS) $(CCTHRIFT_GEN_INCLUDE)\
	-DTOP_SRCDIR=\"$(abs_top_srcdir)/\" \
	-DTOP_BLDDIR=\"$(abs_top_builddir)/\" 
	
check_PROGRAMS+=test/pdgtest
test_pdgtest_SOURCES=service/test/src/pdgtest.cpp
test_pdgtest_LDADD=$(TEST_LIBS) $(AM_LDADD) \
	$(top_builddir)/lib/libgrockerutil.la \
	$(top_builddir)/lib/libmodel.la \
	$(top_builddir)/lib/libcppservicehelper.la\
	$(top_builddir)/webplugin/cppservice.la \
	$(top_builddir)/lib/libmongoose.la\
	-lboost_program_options
test_pdgtest_LDFLAGS=$(TEST_LDFLAGS) $(AM_LDFLAGS)
test_pdgtest_CPPFLAGS=$(AM_CPPFLAGS) $(TEST_CPPFLAGS) $(AM_CPPFLAGS) $(CCTHRIFT_GEN_INCLUDE)\
	-DTOP_SRCDIR=\"$(abs_top_srcdir)/\" \
	-DTOP_BLDDIR=\"$(abs_top_builddir)/\" 

check_PROGRAMS+=test/cppservicetest
test_cppservicetest_SOURCES=service/test/src/cppservicetest.cpp
test_cppservicetest_LDADD=$(TEST_LIBS) $(AM_LDADD) \
	$(top_builddir)/lib/libcppservicehelper.la \
	$(top_builddir)/lib/libgrockerutil.la \
	$(top_builddir)/lib/libmodel.la \
	$(top_builddir)/webplugin/cppservice.la \
	$(top_builddir)/lib/libmongoose.la\
	-lboost_program_options
test_cppservicetest_LDFLAGS=$(TEST_LDFLAGS) $(AM_LDFLAGS)
test_cppservicetest_CPPFLAGS=$(AM_CPPFLAGS) $(TEST_CPPFLAGS) $(AM_CPPFLAGS) $(CCTHRIFT_GEN_INCLUDE)\
	-DTOP_SRCDIR=\"$(abs_top_srcdir)/\" \
	-DTOP_BLDDIR=\"$(abs_top_builddir)/\" 
	
check_PROGRAMS+=test/javaservicetest
test_javaservicetest_SOURCES=service/test/src/javaservicetest.cpp
test_javaservicetest_LDADD=$(TEST_LIBS) $(AM_LDADD) \
	$(top_builddir)/lib/libjavaservicehelper.la \
	$(top_builddir)/lib/libgrockerutil.la \
	$(top_builddir)/lib/libmodel.la \
	$(top_builddir)/webplugin/javaservice.la \
	$(top_builddir)/lib/libmongoose.la\
	-lboost_program_options
test_javaservicetest_LDFLAGS=$(TEST_LDFLAGS) $(AM_LDFLAGS)
test_javaservicetest_CPPFLAGS=$(AM_CPPFLAGS) $(TEST_CPPFLAGS) $(AM_CPPFLAGS) $(CCTHRIFT_GEN_INCLUDE)\
	-DTOP_SRCDIR=\"$(abs_top_srcdir)/\" \
	-DTOP_BLDDIR=\"$(abs_top_builddir)/\"

if ENABLE_PYTHON_PARSER
check_PROGRAMS+=test/pythonservicetest
test_pythonservicetest_SOURCES=service/test/src/pythonservicetest.cpp
test_pythonservicetest_LDADD=$(TEST_LIBS) $(AM_LDADD) \
	$(top_builddir)/lib/libpythonparser.la \
	$(top_builddir)/lib/libgrockerutil.la \
	$(top_builddir)/lib/libmodel.la \
	$(top_builddir)/webplugin/pythonservice.la \
	$(top_builddir)/lib/libmongoose.la\
	-lboost_program_options
test_pythonservicetest_LDFLAGS=$(TEST_LDFLAGS) $(AM_LDFLAGS)
test_pythonservicetest_CPPFLAGS=$(AM_CPPFLAGS) $(TEST_CPPFLAGS) $(AM_CPPFLAGS) $(CCTHRIFT_GEN_INCLUDE)\
	-DTOP_SRCDIR=\"$(abs_top_srcdir)/\" \
	-DTOP_BLDDIR=\"$(abs_top_builddir)/\" 
endif

install-data-hook::
	mkdir -p ${thriftapidir}

uninstall-hook::
	rm -rf ${thriftapidir}


