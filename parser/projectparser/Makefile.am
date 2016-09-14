#ACLOCAL_AMFLAGS=-I m4
#include $(top_srcdir)/Makefile.am.coverage

#AM_CXXFLAGS=-std=gnu++0x -W -Wall $(COVERAGE_CXXFLAGS)
AM_CPPFLAGS+=-I$(top_srcdir)/parser/projectparser/include
#AM_LDFLAGS=$(COVERAGE_LDFLAGS)

projectparserdir=$(pkgincludedir)/projectparser
projectparser_HEADERS=
#noinst_LTLIBRARIES=
#check_PROGRAMS=
#TESTS=$(check_PROGRAMS)

projectparser_HEADERS+=\
    parser/projectparser/include/projectparser/generalprojectparser.h \
    parser/projectparser/include/projectparser/xmlparser.h \
    parser/projectparser/include/projectparser/xmlprojectparser.h \
    parser/projectparser/include/projectparser/jsonprojectparser.h
# noinst_HEADERS+=

lib_LTLIBRARIES+=lib/libprojectparser.la
lib_libprojectparser_la_SOURCES=\
  parser/projectparser/src/generalprojectparser.cpp\
  parser/projectparser/src/includepath.h\
  parser/projectparser/src/includepath.cpp \
  parser/projectparser/src/xmlparser.cc \
  parser/projectparser/src/xmlprojectparser.cc \
  parser/projectparser/src/jsonprojectparser.cpp
  
lib_libprojectparser_la_LIBADD=\
    $(AM_LIBADD) \
    $(XERCES_LIBS) \
    $(LLVM_LIBS) \
    $(top_builddir)/lib/libmodel.la \
    $(top_builddir)/lib/libgrockerutil.la

lib_libprojectparser_la_CPPFLAGS=\
    $(AM_CPPFLAGS)\
    $(XERCES_CPPFLAGS) \
    $(LLVM_CPPFLAGS)

lib_libprojectparser_la_LDFLAGS=\
    $(AM_LDFLAGS)\
    $(LLVM_LDFLAGS)\
    $(XERCES_LDFLAGS)
