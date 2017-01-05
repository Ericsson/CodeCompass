webplugin_LTLIBRARIES+=webplugin/javaservice.la
webplugin_javaservice_la_CPPFLAGS=$(AM_CPPFLAGS) $(CCTHRIFT_GEN_INCLUDE)
webplugin_javaservice_la_SOURCES=\
	service/javaservice/src/plugin.cpp\
	service/javaservice/src/javaservice.h\
	service/javaservice/src/javaservice.cpp

webplugin_javaservice_la_LIBADD=$(AM_LIBADD) $(builddir)/lib/liblanguage-api.la\
  -lthrift $(builddir)/lib/libcore-api.la $(builddir)/lib/libjavaservicehelper.la $(builddir)/lib/libgrockerutil.la
webplugin_javaservice_la_LDFLAGS=-module -avoid-version -shared
