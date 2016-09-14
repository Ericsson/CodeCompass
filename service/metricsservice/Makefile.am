webplugin_LTLIBRARIES+=webplugin/metrics.la
webplugin_metrics_la_CPPFLAGS=$(AM_CPPFLAGS) $(CCTHRIFT_GEN_INCLUDE) -I$(srcdir)/service/codechecker/include/
webplugin_metrics_la_SOURCES=\
	service/metricsservice/src/metricsservice.cpp\
	service/metricsservice/src/metricsservice.h\
	service/metricsservice/src/plugin.cpp

webplugin_metrics_la_LIBADD=$(AM_LIBADD) -lthrift $(builddir)/lib/libmodel.la $(builddir)/lib/libcodechecker-api.la $(builddir)/lib/libmetrics-api.la $(builddir)/lib/libgrockerutil.la
webplugin_metrics_la_LDFLAGS=-module -avoid-version -shared
