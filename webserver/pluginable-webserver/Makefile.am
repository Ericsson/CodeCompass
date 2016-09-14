AM_CPPFLAGS+=-I$(top_srcdir)/webserver/pluginable-webserver/include

bin_pluginablewebserverdir=$(includedir)/webserver/pluginable-webserver

bin_PROGRAMS+=bin/pluginablewebserver

bin_pluginablewebserver_HEADERS=\
  webserver/pluginable-webserver/include/mongoose/plugin.h\
  webserver/pluginable-webserver/include/mongoose/thrifthandler.h

bin_pluginablewebserver_SOURCES=\
  webserver/pluginable-webserver/src/authentication.h\
  webserver/pluginable-webserver/src/authentication.cpp\
  webserver/pluginable-webserver/src/userstat.h\
  webserver/pluginable-webserver/src/userstat.cpp\
  webserver/pluginable-webserver/src/mongoose_main.cpp\
  webserver/pluginable-webserver/src/threadedmongoose.h\
  webserver/pluginable-webserver/src/threadedmongoose.cpp\
  webserver/pluginable-webserver/src/mongoose_utility.h\
  webserver/pluginable-webserver/src/mongoose_utility.cpp

bin_pluginablewebserver_CXXFLAGS=$(AM_CXXFLAGS) -Wno-strict-aliasing
bin_pluginablewebserver_CPPFLAGS=$(AM_CPPFLAGS)
bin_pluginablewebserver_LDADD=$(AM_LDADD) \
 $(top_builddir)/lib/libgrockerutil.la \
 $(top_builddir)/lib/libuscs-api.la \
 $(top_builddir)/lib/libmongoose.la \
 -ldl -lboost_system -lboost_filesystem -lboost_program_options -lldap -lthrift

