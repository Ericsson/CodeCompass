webplugin_LTLIBRARIES+=webplugin/plugin.la
webplugin_plugin_la_CPPFLAGS=$(AM_CPPFLAGS) $(CCTHRIFT_GEN_INCLUDE)
webplugin_plugin_la_SOURCES=\
	service/plugin/src/plugin.cpp \
	service/plugin/src/pluginservice.h \
	service/plugin/src/pluginservice.cpp

webplugin_plugin_la_LIBADD=\
	$(AM_LIBADD) $(builddir)/lib/libplugin-api.la -lthrift \
	$(top_builddir)/lib/libgrockerutil.la
webplugin_plugin_la_LDFLAGS=\
	-module -avoid-version -shared
