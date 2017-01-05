AM_CPPFLAGS+=-I$(srcdir)/service/uscs-api/include

#include here the exported thrift headers
CCTHRIFT_GEN_INCLUDE+=-I$(srcdir)/service/uscs-api/include/uscs-api

lib_LTLIBRARIES+=lib/libuscs-api.la
lib_libuscs_api_la_CPPFLAGS=$(AM_CPPFLAGS) $(CCTHRIFT_GEN_INCLUDE)
lib_libuscs_api_la_SOURCES=\
	service/uscs-api/src/uscs_constants.cpp \
	service/uscs-api/include/uscs-api/uscs_constants.h \
	service/uscs-api/src/uscs_types.cpp \
	service/uscs-api/include/uscs-api/uscs_types.h \
	service/uscs-api/src/UsageStatisticsService.cpp \
	service/uscs-api/include/uscs-api/UsageStatisticsService.h

$(lib_libuscs_api_la_SOURCES): uscs-api-rule

$(srcdir)/uscs-api-rule: service/uscs-api/uscs.thrift
	mkdir --parents $(srcdir)/service/uscs-api/include/uscs-api
	mkdir --parents $(srcdir)/service/uscs-api/src
	thrift --gen cpp -I $(CCTHRIFT_INCLUDE) -o service/uscs-api $<
	cp -f $(srcdir)/service/uscs-api/gen-cpp/*.h $(srcdir)/service/uscs-api/include/uscs-api
	cp -f $(srcdir)/service/uscs-api/gen-cpp/*.cpp $(srcdir)/service/uscs-api/src
	rm  $(srcdir)/service/uscs-api/gen-cpp/UsageStatisticsService_server.skeleton.cpp
	touch uscs-api-rule

EXTRA_DIST+=service/uscs-api/uscs.thrift uscs-api-rule
GENCLEANFILES+=$(lib_libuscs_api_la_SOURCES) uscs-api-rule
