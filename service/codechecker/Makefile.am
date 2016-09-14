webplugin_LTLIBRARIES+=webplugin/codechecker.la
webplugin_codechecker_la_CPPFLAGS=$(AM_CPPFLAGS) $(CCTHRIFT_GEN_INCLUDE)
webplugin_codechecker_la_SOURCES=\
	service/codechecker/src/plugin.cpp\
	service/codechecker/src/codecheckerproxy.h\
	service/codechecker/src/codecheckerproxy.cpp

webplugin_codechecker_la_LIBADD=\
  $(AM_LIBADD) \
	-lboost_regex \
	$(builddir)/lib/libcodechecker-api.la \
	-lthrift \
	$(builddir)/lib/libgrockerutil.la
webplugin_codechecker_la_LDFLAGS=-module -avoid-version -shared

# check_PROGRAMS+=test/codechecker_service_test
# test_codechecker_service_test_SOURCES=\
# 	service/codechecker/test/codechecker_service_test.cpp
# test_codechecker_service_test_LDADD=$(TEST_LIBS) $(AM_LDADD) \
# 	$(top_builddir)/webplugin/codechecker.la \
# 	$(top_builddir)/lib/libgrockerutil.la \
# 	$(top_builddir)/lib/libmongoose.la \
# 	-lboost_program_options
# test_codechecker_service_test_LDFLAGS=\
# 	$(TEST_LDFLAGS) $(AM_LDFLAGS)
# test_codechecker_service_test_CPPFLAGS=\
# 	$(AM_CPPFLAGS) $(TEST_CPPFLAGS) $(CCTHRIFT_GEN_INCLUDE)

