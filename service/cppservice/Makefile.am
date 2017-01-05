webplugin_LTLIBRARIES+=webplugin/cppservice.la
webplugin_cppservice_la_CPPFLAGS=$(AM_CPPFLAGS) $(CCTHRIFT_GEN_INCLUDE)
webplugin_cppservice_la_SOURCES=\
	service/cppservice/src/plugin.cpp\
	service/cppservice/src/cppservice.h\
	service/cppservice/src/cppservice.cpp

webplugin_cppservice_la_LIBADD=$(AM_LIBADD) $(builddir)/lib/liblanguage-api.la\
  -lthrift $(builddir)/lib/libcore-api.la $(builddir)/lib/libcppservicehelper.la $(builddir)/lib/libgrockerutil.la
webplugin_cppservice_la_LDFLAGS=-module -avoid-version -shared
