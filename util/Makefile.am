AM_CPPFLAGS+=-I$(top_srcdir)/util/include
lib_LTLIBRARIES+=lib/libgrockerutil.la
lib_libgrockerutil_la_SOURCES=\
	util/src/util.cpp \
	util/src/filesystem.cpp \
    util/src/standarderrorlogstrategy.cpp \
    util/src/filelogstrategy.cpp \
    util/src/logutil.cpp \
    util/src/streamlog.cpp \
    util/src/graph.cpp \
    util/src/graphpimpl.h \
    util/src/pipedprocess.cpp \
    util/src/thriftprocess.cpp \
    util/src/environment.cpp \
    util/src/legendbuilder.cpp \
    util/src/json.cpp

libgrockerutildir=$(includedir)/util
libgrockerutil_HEADERS=\
	util/include/util/util.h \
	util/include/util/filesystem.h \
	util/include/util/sourcetextrange.h \
    util/include/util/streamlog.h \
    util/include/util/standarderrorlogstrategy.h \
    util/include/util/logstrategy.h \
    util/include/util/filelogstrategy.h \
    util/include/util/odbtransaction.h\
    util/include/util/logutil.h \
    util/include/util/graph.h \
    util/include/util/graphbuilder.h \
    util/include/util/odbobjectcache.h \
    util/include/util/cache.h \
    util/include/util/tsqueue.h \
    util/include/util/threadpool.h \
    util/include/util/exceptions.h \
    util/include/util/pipedprocess.h \
    util/include/util/thriftprocess.h \
    util/include/util/environment.h \
    util/include/util/json.h \
    util/include/util/diagram/bfs.h \
    util/include/util/diagram/graphdecorator.h \
    util/include/util/diagram/legendbuilder.h

lib_libgrockerutil_la_LIBADD=$(AM_LIBADD) $(GRAPHVIZ_LIBADD) -lgvc -lcgraph -lboost_regex $(builddir)/lib/libmodel.la
lib_libgrockerutil_la_CPPFLAGS=$(AM_CPPFLAGS) $(GRAPHVIZ_CPPFLAGS)
lib_libgrockerutil_la_CXXFLAGS=$(AM_CXXFLAGS) -fPIC
lib_libgrockerutil_la_LDFLAGS=$(AM_LDFLAGS) $(GRAPHVIZ_LIBS)


