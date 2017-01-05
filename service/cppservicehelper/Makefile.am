AM_CPPFLAGS+=-I$(top_srcdir)/service/cppservicehelper/include -I$(top_srcdir)/service/diagram/include

lib_LTLIBRARIES+=lib/libcppservicehelper.la

lib_libcppservicehelper_la_SOURCES=\
	service/cppservicehelper/include/cppservicehelper/cppservicehelper.h \
	service/cppservicehelper/include/cppservicehelper/symbolhandler.h \
	service/cppservicehelper/include/cppservicehelper/odbquery.h \
	service/cppservicehelper/src/slicer/slicerhelper.h \
	service/cppservicehelper/src/slicer/slicerhelper.cpp \
	service/cppservicehelper/src/slicer/pdg.h \
	service/cppservicehelper/src/slicer/pdg.cpp \
	service/cppservicehelper/src/slicer/pdg_builder.h \
	service/cppservicehelper/src/slicer/multistack.h \
	service/cppservicehelper/src/slicer/variablevisitor.h \
	service/cppservicehelper/src/utils.h \
	service/cppservicehelper/src/utils.cpp \
	service/cppservicehelper/src/symbolhandler.cpp \
	service/cppservicehelper/src/cppservicehelper.cpp \
	service/cppservicehelper/src/diagram/diagram.h \
	service/cppservicehelper/src/diagram/diagram.cpp \
	service/cppservicehelper/src/diagram/cppgraphdecorator.h \
	service/cppservicehelper/src/diagram/includedependency.h \
	service/cppservicehelper/src/diagram/includedependency.cpp \
	service/cppservicehelper/src/diagram/usedcomponents.h \
	service/cppservicehelper/src/diagram/usedcomponents.cpp \
	service/cppservicehelper/src/diagram/componentusers.h \
	service/cppservicehelper/src/diagram/componentusers.cpp \
	service/cppservicehelper/src/diagram/interface.h \
	service/cppservicehelper/src/diagram/interface.cpp \
	service/cppservicehelper/src/diagram/subsystemdependency.h \
	service/cppservicehelper/src/diagram/subsystemdependency.cpp \
	service/cppservicehelper/src/diagram/externaldependency.h \
	service/cppservicehelper/src/diagram/externaldependency.cpp \
	service/cppservicehelper/src/diagram/externalusers.h \
	service/cppservicehelper/src/diagram/externalusers.cpp

lib_libcppservicehelper_la_LIBADD=$(AM_LIBADD) $(LLVM_LIBS) $(builddir)/lib/liblanguage-api.la \
	-lthrift $(builddir)/lib/libcore-api.la -lodb $(top_builddir)/lib/libmodel.la $(builddir)/lib/libgrockerutil.la \
	$(top_builddir)/lib/liblangservicelib.la \
	$(top_builddir)/lib/libreparser.la
lib_libcppservicehelper_la_CPPFLAGS=$(AM_CPPFLAGS) $(CCTHRIFT_GEN_INCLUDE) $(LLVM_CPPFLAGS)
lib_libcppservicehelper_la_LDFLAGS=$(AM_LDFLAGS) $(LLVM_LDFLAGS)
