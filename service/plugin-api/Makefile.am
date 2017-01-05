AM_CPPFLAGS+=-I$(srcdir)/service/plugin-api/include
#include here the exported thrift headers
CCTHRIFT_GEN_INCLUDE+=-I$(srcdir)/service/plugin-api/include/plugin-api
lib_LTLIBRARIES+=lib/libplugin-api.la
lib_libplugin_api_la_CPPFLAGS=$(AM_CPPFLAGS) $(CCTHRIFT_GEN_INCLUDE)

lib_libplugin_api_la_SOURCES=\
	service/plugin-api/src/plugin_constants.cpp\
	service/plugin-api/include/plugin-api/plugin_constants.h\
	service/plugin-api/src/plugin_types.cpp\
	service/plugin-api/include/plugin-api/plugin_types.h\
	service/plugin-api/src/PluginService.cpp\
	service/plugin-api/include/plugin-api/PluginService.h

$(lib_libplugin_api_la_SOURCES): plugin-api-rule

$(srcdir)/plugin-api-rule: service/plugin-api/plugin.thrift $(CCTHRIFT_INCLUDE)/core-api/common.thrift
	mkdir --parents $(srcdir)/service/plugin-api/include/plugin-api
	mkdir --parents $(srcdir)/service/plugin-api/src
	thrift --gen cpp -I $(CCTHRIFT_INCLUDE) -o service/plugin-api $<
	thrift --gen js -I $(CCTHRIFT_INCLUDE) -o service/plugin-api $<
	cp -f $(srcdir)/service/plugin-api/gen-cpp/*.h $(srcdir)/service/plugin-api/include/plugin-api
	cp -f $(srcdir)/service/plugin-api/gen-cpp/*.cpp $(srcdir)/service/plugin-api/src
	rm  $(srcdir)/service/plugin-api/gen-cpp/PluginService_server.skeleton.cpp
	touch plugin-api-rule
#copying javascripts file into webroot
pluginapijsdir=$(jsdir)/plugin-api
pluginapijs_DATA=\
	service/plugin-api/gen-js/PluginService.js\
	service/plugin-api/gen-js/plugin_types.js

$(pluginapijs_DATA) : plugin-api-rule

EXTRA_DIST+=service/plugin-api/plugin.thrift plugin-api-rule\
  $(pluginapijs_DATA)

CLEANFILES+=service/plugin-api/gen-cpp/PluginService_server.skeleton.cpp

GENCLEANFILES+=$(lib_libplugin_api_la_SOURCES) plugin-api-rule \
  $(pluginapijs_DATA)
  

install-data-hook::
	mkdir -p ${thriftapidir}/plugin-api
	cp -R $(srcdir)/service/plugin-api/*.thrift ${thriftapidir}/plugin-api

