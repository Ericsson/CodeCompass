webplugin_LTLIBRARIES+=webplugin/version.la
webplugin_version_la_CPPFLAGS=$(AM_CPPFLAGS) $(CCTHRIFT_GEN_INCLUDE)
webplugin_version_la_SOURCES=\
	service/version/src/plugin.cpp\
	service/version/src/versionservice.h\
	service/version/src/versionservice.cpp\
	service/version/src/serviceprocess.h

webplugin_version_la_LIBADD=\
	$(AM_LIBADD) \
	-lboost_regex \
	$(builddir)/lib/libversion-api.la \
	$(builddir)/lib/libcore-api.la \
	$(builddir)/lib/libmodel.la  \
	$(builddir)/lib/libgrockerutil.la \
	$(builddir)/lib/libgitparser.la \
	-lodb \
	-lthrift

webplugin_version_la_LDFLAGS=-module -avoid-version -shared


check_PROGRAMS+=test/version_service_test
test_version_service_test_SOURCES=\
	service/version/test/version_service_test.cpp
test_version_service_test_LDADD=$(TEST_LIBS) $(AM_LDADD) \
	$(top_builddir)/webplugin/version.la \
	$(top_builddir)/lib/libversion-api.la \
	$(top_builddir)/lib/libcore-api.la \
	$(top_builddir)/lib/libmodel.la  \
	$(top_builddir)/lib/libgitparser.la \
	$(top_builddir)/lib/libgrockerutil.la \
	$(top_builddir)/lib/libmongoose.la \
	-lboost_program_options
test_version_service_test_LDFLAGS=\
	$(TEST_LDFLAGS) $(AM_LDFLAGS)
test_version_service_test_CPPFLAGS=\
	$(AM_CPPFLAGS) $(TEST_CPPFLAGS) $(CCTHRIFT_GEN_INCLUDE)
