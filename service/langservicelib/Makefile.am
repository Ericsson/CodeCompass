AM_CPPFLAGS+=-I$(top_srcdir)/service/langservicelib/include

lib_LTLIBRARIES+=lib/liblangservicelib.la

lib_liblangservicelib_la_CPPFLAGS=$(AM_CPPFLAGS) $(CCTHRIFT_GEN_INCLUDE)
lib_liblangservicelib_la_SOURCES=\
	service/langservicelib/include/langservicelib/odbquery.h \
	service/langservicelib/include/langservicelib/utils.h \
	service/langservicelib/src/utils.cpp

