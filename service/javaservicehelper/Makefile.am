AM_CPPFLAGS+=-I$(top_srcdir)/service/javaservicehelper/include

lib_LTLIBRARIES+=lib/libjavaservicehelper.la

lib_libjavaservicehelper_la_CPPFLAGS=$(AM_CPPFLAGS) $(CCTHRIFT_GEN_INCLUDE)
lib_libjavaservicehelper_la_SOURCES=\
	service/javaservicehelper/include/javaservicehelper/javaservicehelper.h \
	service/javaservicehelper/include/javaservicehelper/symbolhandler.h \
	service/javaservicehelper/include/javaservicehelper/odbquery.h \
	service/javaservicehelper/src/utils.h \
	service/javaservicehelper/src/utils.cpp \
	service/javaservicehelper/src/symbolhandler.cpp \
	service/javaservicehelper/src/javaservicehelper.cpp

lib_libjavaservicehelper_la_LIBADD=$(AM_LIBADD) $(builddir)/lib/liblanguage-api.la \
	-lthrift $(builddir)/lib/libcore-api.la -lodb $(top_builddir)/lib/libmodel.la $(builddir)/lib/libgrockerutil.la \
	$(top_builddir)/lib/liblangservicelib.la

