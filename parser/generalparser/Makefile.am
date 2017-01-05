parserplugin_libgeneralparserdir=
parserplugin_LTLIBRARIES+=parserplugin/libgeneralparser.la
parserplugin_libgeneralparser_la_CPPFLAGS=$(AM_CPPFLAGS) \
  -I$(top_srcdir)/parser/generalparser/include \
  -I$(top_srcdir)/parser/parser/include

parserplugin_libgeneralparser_la_SOURCES=\
	parser/generalparser/src/generalparser.cpp \
	parser/generalparser/src/includepath.cpp \
	parser/parser/src/projectparser.cpp
	
parserplugin_libgeneralparser_HEADERS= \
    parser/generalparser/include/generalparser/generalparser.h \
    parser/generalparser/include/generalparser/includepath.h \
    parser/parser/include/parser/abstract_parser.h \
    parser/parser/include/parser/projectparser.h

parserplugin_libgeneralparser_la_LIBADD=$(AM_LIBADD)

parserplugin_libgeneralparser_la_LDFLAGS=-module -avoid-version -shared
