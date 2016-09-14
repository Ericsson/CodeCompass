if ENABLE_USCS

bin_PROGRAMS+=bin/compass_uscs
bin_compass_uscs_ODBSOURCES= \
	service/uscs/src/model/server.h \
	service/uscs/src/model/entry.h
bin_compass_uscs_ODBGENSOURCES= \
  $(bin_compass_uscs_ODBSOURCES:.h=-odb.hxx) \
  $(bin_compass_uscs_ODBSOURCES:.h=-odb.cxx) \
  $(bin_compass_uscs_ODBSOURCES:.h=-odb.ixx)
bin_compass_uscs_SOURCES= \
	$(bin_compass_uscs_ODBSOURCES) \
	$(bin_compass_uscs_ODBGENSOURCES) \
	service/uscs/src/uscs.cpp \
	service/uscs/src/uscshandler.h \
	service/uscs/src/uscshandler.cpp \
	service/uscs/src/model/database.h \
	service/uscs/src/model/database.cpp
bin_compass_uscs_LDADD=$(AM_LDADD) \
	$(top_builddir)/lib/libuscs-api.la \
	-lboost_program_options \
	-lthrift \
	$(COMMON_LIBS)
bin_compass_uscs_LDFLAGS=$(AM_LDFLAGS) -static
bin_compass_uscs_CPPFLAGS=$(AM_CPPFLAGS)

$(bin_compass_uscs_SOURCES): uscs-api-rule

noinst_HEADERS+= $(bin_compass_uscs_ODBSOURCES)
GENCLEANFILES += $(bin_compass_uscs_ODBGENSOURCES)
BUILT_SOURCES += $(bin_compass_uscs_ODBGENSOURCES)

endif
