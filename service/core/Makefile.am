webplugin_LTLIBRARIES+=webplugin/core.la
webplugin_core_la_CPPFLAGS=$(AM_CPPFLAGS) $(CCTHRIFT_GEN_INCLUDE)
webplugin_core_la_SOURCES=\
	service/core/src/projectservice.cpp\
	service/core/src/projectservice.h\
	service/core/src/plugin.cpp

webplugin_core_la_LIBADD=$(AM_LIBADD) -lthrift $(builddir)/lib/libmodel.la $(builddir)/lib/libcore-api.la $(builddir)/lib/libgrockerutil.la
webplugin_core_la_LDFLAGS=-module -avoid-version -shared




