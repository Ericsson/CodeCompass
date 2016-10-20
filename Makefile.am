# SUBDIRS=libgthreads
#SUBDIRS=. benchmarks

if ENV_SH_USED
 ifndef grocker_env_set
 $(error No env.sh loaded!)
 endif
endif

ACLOCAL_AMFLAGS=-I m4

# I know it's dirty, but -Wno-unknown-pragmas must be after -Wall to supress
# these type of warnings.
CXXFLAGS+= -Wno-unknown-pragmas

# global
AM_CXXFLAGS=$(COVERAGE_CXXFLAGS) -Wno-unknown-pragmas
AM_CPPFLAGS=$(DATABASE_CPPFLAGS)
#we would like LDFLAGS that we pass to configure to be included first at linking
AM_LDFLAGS=$(LDFLAGS) $(COVERAGE_LDFLAGS)
EXTRA_DIST=test-wrapper
MOSTLYCLEANFILES=db.sqlite
MAINTAINERCLEANFILES=
GENCLEANFILES=
DISTCLEANFILES=
CLEANFILES=
BUILT_SOURCES=

webrootdir = $(datadir)/$(PACKAGE)/html
thriftapidir = $(datadir)/$(PACKAGE)/thrift-api
jsdir=$(webrootdir)/scripts
javaparserinstalldir = $(datadir)/$(PACKAGE)/java
clidir=$(datadir)/$(PACKAGE)/cli

LLVM_LIBS= \
  -lclangFrontend \
  -lclangParse \
  -lclangSema \
  -lclangStaticAnalyzerFrontend \
  -lclangStaticAnalyzerCheckers \
  -lclangStaticAnalyzerCore \
  -lclangDriver \
  -lclangEdit \
  -lclangAnalysis \
  -lclangSerialization \
  -lclangAST \
  -lclangLex \
  -lclangBasic\
  -lclangTooling \
  @LLVM_LIBS@

lib_LTLIBRARIES =
noinst_LTLIBRARIES =
noinst_HEADERS =
bin_PROGRAMS =
bin_SCRIPTS =
pkglib_LTLIBRARIES=

AM_TESTS_ENVIRONMENT=
check_PROGRAMS =
TESTS = $(check_PROGRAMS)
LOG_COMPILER=$(top_srcdir)/test-wrapper

include_HEADERS=

#JAVAC_FLAGS="-g"
JAVAC_FLAGS=
# Java libraries:
# Put your library here
CC_EXTERNAL_JAVA_LIBS=\
    $(abs_srcdir)/lib/java/lucene-analyzers-common-4.9.0.jar \
    $(abs_srcdir)/lib/java/lucene-core-4.9.0.jar \
    $(abs_srcdir)/lib/java/lucene-queryparser-4.9.0.jar \
    $(abs_srcdir)/lib/java/lucene-highlighter-4.9.0.jar \
    $(abs_srcdir)/lib/java/lucene-queries-4.9.0.jar \
    $(abs_srcdir)/lib/java/lucene-memory-4.9.0.jar \
    $(abs_srcdir)/lib/java/lucene-misc-4.9.0.jar \
    $(abs_srcdir)/lib/java/lucene-suggest-4.9.0.jar \
    $(abs_srcdir)/lib/java/commons-codec-1.6.jar \
    $(abs_srcdir)/lib/java/commons-logging-1.1.1.jar \
    $(abs_srcdir)/lib/java/commons-lang3-3.1.jar \
    $(abs_srcdir)/lib/java/commons-io-2.4.jar \
    $(abs_srcdir)/lib/java/httpcore-4.2.4.jar \
    $(abs_srcdir)/lib/java/httpclient-4.2.5.jar \
    $(abs_srcdir)/lib/java/libthrift-0.9.2.jar \
    $(abs_srcdir)/lib/java/servlet-api-2.5.jar \
    $(abs_srcdir)/lib/java/log4j-1.2.14.jar \
    $(abs_srcdir)/lib/java/slf4j-log4j12-1.5.8.jar \
    $(abs_srcdir)/lib/java/slf4j-api-1.5.8.jar \
    $(abs_srcdir)/lib/java/eclipselink.jar \
    $(abs_srcdir)/lib/java/javax.persistence_2.1.0.v201304241213.jar \
    $(abs_srcdir)/lib/java/org.eclipse.core.contenttype_3.4.200.v20130326-1255.jar \
    $(abs_srcdir)/lib/java/org.eclipse.core.jobs_3.5.300.v20130429-1813.jar \
    $(abs_srcdir)/lib/java/org.eclipse.core.resources_3.8.101.v20130717-0806.jar \
    $(abs_srcdir)/lib/java/org.eclipse.core.runtime_3.9.0.v20130326-1255.jar \
    $(abs_srcdir)/lib/java/org.eclipse.equinox.common_3.6.200.v20130402-1505.jar \
    $(abs_srcdir)/lib/java/org.eclipse.equinox.preferences_3.5.100.v20130422-1538.jar \
    $(abs_srcdir)/lib/java/org.eclipse.jdt.core_3.9.1.v20130905-0837.jar \
    $(abs_srcdir)/lib/java/org.eclipse.osgi_3.9.1.v20130814-1242.jar \
    $(abs_srcdir)/lib/java/sqlite.jar \
    $(abs_srcdir)/lib/java/postgresql-9.3-1101.jdbc41.jar \
    $(abs_srcdir)/lib/java/simplemagic-1.6.jar \
    $(abs_srcdir)/lib/java/annotations-5.1.jar \
    $(abs_srcdir)/lib/java/gson-2.2.4.jar \
    $(abs_srcdir)/lib/java/guava-15.0.jar \
    $(abs_srcdir)/lib/java/jackson-annotations-2.2.3.jar \
    $(abs_srcdir)/lib/java/jackson-core-2.2.3.jar \
    $(abs_srcdir)/lib/java/jackson-databind-2.2.3.jar
JAVA_BUILD_CP=$(shell echo "$(CC_EXTERNAL_JAVA_LIBS)" | sed -e 's/[[:space:]]\{1,\}/:/g')

javalibdir=$(datadir)/$(PACKAGE)/java
javalib_DATA=$(CC_EXTERNAL_JAVA_LIBS)

include $(srcdir)/model/Makefile.am
include $(srcdir)/util/Makefile.am
include $(srcdir)/service/Makefile.am
include $(srcdir)/search/Makefile.am
include $(srcdir)/parser/Makefile.am
include $(srcdir)/webserver/Makefile.am
include $(srcdir)/webgui2/Makefile.am
include $(srcdir)/dox/Makefile.am
include $(srcdir)/scripts/Makefile.am
#include $(srcdir)/Makefile.am.coverage
include $(srcdir)/Makefile.am.doxygen


mostlyclean-local: mostlyclean-servicetests
	-rm -f $(TESTS:=.xml)

genclean:
	-rm -f $(GENCLEANFILES)

MAINTAINERCLEANFILES+=$(GENCLEANFILES)
