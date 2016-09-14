AM_CPPFLAGS+=-I$(srcdir)/service/metrics-api/include
#include here the exported thrift headers
CCTHRIFT_GEN_INCLUDE+=-I$(srcdir)/service/metrics-api/include/metrics-api
lib_libmetrics_api_la_CPPFLAGS=$(AM_CPPFLAGS) $(CCTHRIFT_GEN_INCLUDE)
lib_LTLIBRARIES+=lib/libmetrics-api.la

CCTHRIFT_METRICS_SOURCES=\
    service/metrics-api/src/metrics_constants.cpp \
    service/metrics-api/include/metrics-api/metrics_constants.h \
    service/metrics-api/src/metrics_types.cpp \
    service/metrics-api/include/metrics-api/metrics_types.h \
    service/metrics-api/src/MetricsService.cpp \
    service/metrics-api/include/MetricsService.h

lib_libmetrics_api_la_SOURCES=$(CCTHRIFT_METRICS_SOURCES)

lib_libmetrics_api_la_LIBADD=$(builddir)/lib/libcore-api.la

$(CCTHRIFT_METRICS_SOURCES): $(srcdir)/metrics-api-metrics-rule

$(srcdir)/metrics-api-metrics-rule: service/metrics-api/metrics.thrift $(CCTHRIFT_INCLUDE)/core-api/common.thrift
	mkdir --parents $(srcdir)/service/metrics-api/include/metrics-api
	mkdir --parents $(srcdir)/service/metrics-api/src
	thrift --gen cpp -I $(CCTHRIFT_INCLUDE) -o service/metrics-api $<
	thrift --gen js -I $(CCTHRIFT_INCLUDE) -o service/metrics-api $<
	thrift --gen java -I $(CCTHRIFT_INCLUDE) -o service/metrics-api $<
	cp -f $(srcdir)/service/metrics-api/gen-cpp/metrics_*.h $(srcdir)/service/metrics-api/include/metrics-api
	cp -f $(srcdir)/service/metrics-api/gen-cpp/metrics_*.cpp $(srcdir)/service/metrics-api/src
	cp -f $(srcdir)/service/metrics-api/gen-cpp/[mM]etrics*.h $(srcdir)/service/metrics-api/include/metrics-api
	cp -f $(srcdir)/service/metrics-api/gen-cpp/[mM]etrics*.cpp $(srcdir)/service/metrics-api/src
	touch $@

EXTRA_DIST+=service/metrics-api/metrics.thrift metrics-api-metrics-rule

#copying javascripts file into webroot
metricsjsdir=$(jsdir)/metrics-api
metricsjs_DATA=\
	service/metrics-api/gen-js/MetricsService.js\
	service/metrics-api/gen-js/metrics_types.js

$(metricsjs_DATA): $(srcdir)/metrics-api-metrics-rule

METRICS_API_JAVA_SRC=

$(METRICS_API_JAVA_SRC): $(srcdir)/metrics-api-metrics-rule

EXTRA_DIST+=$(metricsjs_DATA)

GENCLEANFILES+= $(lib_libmetrics_api_la_SOURCES)\
	$(srcdir)/metrics-api-metrics-rule \
	$(METRICS_API_JAVA_SRC)\
	$(metricsjs_DATA)

install-data-hook::
	mkdir -p $(thriftapidir)/metrics-api
	cp -R $(srcdir)/service/metrics-api/*.thrift $(thriftapidir)/metrics-api
