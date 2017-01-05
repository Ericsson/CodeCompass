AM_CPPFLAGS+=-I$(top_srcdir)/service/pythonservice/include

webplugin_LTLIBRARIES+=webplugin/pythonservice.la
webplugin_pythonservice_la_CPPFLAGS=$(AM_CPPFLAGS) $(CCTHRIFT_GEN_INCLUDE)
webplugin_pythonservice_la_SOURCES=\
	service/pythonservice/src/plugin.cpp \
	service/pythonservice/include/pythonservice/pythonservice.h \
	service/pythonservice/src/pythonservice.cpp \
	service/pythonservice/include/pythonservice/treehandler.h \
	service/pythonservice/src/treehandler.cpp \
	service/pythonservice/include/pythonservice/pythonqueryhelper.h \
	service/pythonservice/src/pythonqueryhelper.cpp \
	service/pythonservice/include/pythonservice/pythonpossibles.h \
	service/pythonservice/src/pythonpossibles.cpp 

webplugin_pythonservice_la_LIBADD=$(AM_LIBADD) $(builddir)/lib/liblanguage-api.la\
  -lthrift $(builddir)/lib/libcore-api.la $(builddir)/lib/libgrockerutil.la \
  $(top_builddir)/lib/liblangservicelib.la
webplugin_pythonservice_la_LDFLAGS=-module -avoid-version -shared
