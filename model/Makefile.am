AM_CPPFLAGS+=-I$(top_srcdir)/model/include


lib_LTLIBRARIES += lib/libmodel.la

lib_libmodel_la_ODBSOURCES=\
  model/include/model/project.h \
  model/include/model/filecontent.h \
  model/include/model/file.h \
  model/include/model/fileposinfo.h \
  model/include/model/fileloc.h \
  model/include/model/statistics.h \
  model/include/model/cxx/cppastnode.h \
  model/include/model/cxx/cppentity.h \
  model/include/model/cxx/cppfunction.h \
  model/include/model/cxx/cppfunctionpointer.h \
  model/include/model/cxx/cppmacro.h \
  model/include/model/cxx/cppinheritance.h \
  model/include/model/cxx/cppfriendship.h \
  model/include/model/cxx/cpptype.h \
  model/include/model/cxx/cppvariable.h \
  model/include/model/cxx/cppmacroexpansion.h \
  model/include/model/cxx/cppnamespace.h \
  model/include/model/cxx/cppheaderinclusion.h \
  model/include/model/cxx/cppenum.h \
  model/include/model/cxx/cpptypedef.h \
  model/include/model/cxx/cpprelation.h \
  model/include/model/cxx/cpppointeranalysis.h \
  model/include/model/cxx/cppproviderelation.h \
  model/include/model/cxx/cppimplicit.h \
  model/include/model/buildtarget.h \
  model/include/model/buildsource.h \
  model/include/model/buildparameter.h \
  model/include/model/buildlogmessage.h \
  model/include/model/buildlog.h \
  model/include/model/buildaction.h \
  model/include/model/metrics.h \
  model/include/model/position.h \
  model/include/model/option.h \
  model/include/model/comment/doccomment.h \
  model/include/model/java/javaastnode.h \
  model/include/model/java/javatype.h \
  model/include/model/java/javatype_interfaces.h \
  model/include/model/java/javatype_typeparams.h \
  model/include/model/java/javaenumconstant.h \
  model/include/model/java/javavariable.h \
  model/include/model/java/javafunction.h \
  model/include/model/java/javafunction_typeparams.h \
  model/include/model/java/javamember.h \
  model/include/model/java/javaimport.h \
  model/include/model/java/javadoccomment.h \
  model/include/model/java/javaannotation.h \
  model/include/model/version/repository.h \
  model/include/model/diagram/node.h \
  model/include/model/diagram/edge.h \
  model/include/model/diagram/nodeattribute.h \
  model/include/model/diagram/edgeattribute.h

if ENABLE_PYTHON_PARSER
lib_libmodel_la_ODBSOURCES+= \
  model/include/model/python/pythonastnode.h \
  model/include/model/python/pythonattribute.h \
  model/include/model/python/pythonbinding.h \
  model/include/model/python/pythonclassdef.h \
  model/include/model/python/pythondecorator.h \
  model/include/model/python/pythonfunctiondef.h \
  model/include/model/python/pythonfunctioncall.h \
  model/include/model/python/pythonfunctionparam.h \
  model/include/model/python/pythoninheritance.h \
  model/include/model/python/pythonreference.h \
  model/include/model/python/pythonunknown.h \
  model/include/model/python/pythonvariable.h \
  model/include/model/python/pythonviews.h \
  model/include/model/python/pythonvariableref.h
endif

noinst_HEADERS+=$(lib_libmodel_la_ODBSOURCES)
lib_libmodel_la_ALLSOURCES=\
  $(lib_libmodel_la_ODBSOURCES:.h=-odb.hxx) \
  $(lib_libmodel_la_ODBSOURCES:.h=-odb.cxx) \
  $(lib_libmodel_la_ODBSOURCES:.h=-odb.ixx)
GENCLEANFILES += $(lib_libmodel_la_ALLSOURCES)

BUILT_SOURCES += $(lib_libmodel_la_ALLSOURCES)

lib_libmodel_la_SOURCES=\
  $(lib_libmodel_la_ALLSOURCES)\
  model/src/asttype.cpp\
  model/src/cxx/cppastnode.cpp\
  model/include/model/workspace.h

lib_libmodel_la_CPPFLAGS=$(AM_CPPFLAGS) -I$(top_srcdir)/libgthreads/include
lib_libmodel_la_LIBADD=$(COMMON_LIBS)

check_PROGRAMS+=test/featuretest
test_featuretest_SOURCES=\
  model/test/featuretest.cpp

test_featuretest_LDADD=$(TEST_LIBS) $(AM_LDADD) lib/libmodel.la
test_featuretest_LDFLAGS=$(TEST_LDFLAGS) $(AM_LDFLAGS)
test_featuretest_CPPFLAGS=$(AM_CPPFLAGS) $(TEST_CPPFLAGS)

ODB = @ODB@
ODBFLAGS = @ODBFLAGS@
ODBCPPFLAGS = @ODBCPPFLAGS@

if HAVE_CXX11
ODBFLAGS += --std c++11
endif

%-odb.ixx %-odb.cxx %-odb.hxx: %.h
	$(ODB) $(ODBCPPFLAGS) $(CPPFLAGS) $(AM_CPPFLAGS) $(ODBFLAGS)  --database @database@ \
	--generate-query --generate-schema --default-pointer "std::shared_ptr" \
 	-o $(top_srcdir)/$(<D)/ $<


AM_DISTCHECK_CONFIGURE_FLAGS=--with-database=@database@

install-data-hook::
	mkdir -p ${prefix}/usr/share/codecompass/sql
	find $(top_srcdir)/model -name "*.sql" -print0 | xargs -0 -I {} cp {} ${prefix}/usr/share/codecompass/sql

uninstall-hook::
	rm -rf ${prefix}/usr/share/codecompass/sql
