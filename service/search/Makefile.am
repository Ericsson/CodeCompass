webplugin_LTLIBRARIES+=webplugin/search.la
webplugin_search_la_CPPFLAGS=$(AM_CPPFLAGS) $(CCTHRIFT_GEN_INCLUDE)
webplugin_search_la_SOURCES=\
	service/search/src/plugin.cpp\
	service/search/src/searchservice.h\
	service/search/src/searchservice.cpp\
	service/search/src/serviceprocess.h

webplugin_search_la_LIBADD=\
  $(AM_LIBADD) \
	-lboost_regex \
	$(builddir)/lib/libsearch-api.la \
	-lthrift \
	$(builddir)/lib/libmodel.la \
	$(builddir)/lib/libcore-api.la \
	$(builddir)/lib/libgrockerutil.la
webplugin_search_la_LDFLAGS=-module -avoid-version -shared

check_PROGRAMS+=test/search_service_test
test_search_service_test_SOURCES=\
	service/search/test/search_service_test.cpp
test_search_service_test_LDADD=$(TEST_LIBS) $(AM_LDADD) \
	$(top_builddir)/webplugin/search.la \
	$(top_builddir)/lib/libgrockerutil.la \
	$(top_builddir)/lib/libmongoose.la \
	-lboost_program_options
test_search_service_test_LDFLAGS=\
	$(TEST_LDFLAGS) $(AM_LDFLAGS)
test_search_service_test_CPPFLAGS=\
	$(AM_CPPFLAGS) $(TEST_CPPFLAGS) $(CCTHRIFT_GEN_INCLUDE)

