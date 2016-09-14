AM_CPPFLAGS+=-I$(srcdir)/service/workspace-api/include
#include here the exported thrift headers
CCTHRIFT_GEN_INCLUDE+=-I$(srcdir)/service/workspace-api/include/workspace-api
lib_LTLIBRARIES+=lib/libworkspace-api.la
lib_libworkspace_api_la_CPPFLAGS=$(AM_CPPFLAGS) $(CCTHRIFT_GEN_INCLUDE)

lib_libworkspace_api_la_SOURCES=\
	service/workspace-api/src/workspace_constants.cpp\
	service/workspace-api/include/workspace-api/workspace_constants.h\
	service/workspace-api/src/workspace_types.cpp\
	service/workspace-api/include/workspace-api/workspace_types.h\
	service/workspace-api/src/WorkspaceService.cpp\
	service/workspace-api/include/workspace-api/WorkspaceService.h

$(lib_libworkspace_api_la_SOURCES): workspace-api-rule 

$(srcdir)/workspace-api-rule: service/workspace-api/workspace.thrift $(CCTHRIFT_INCLUDE)/core-api/common.thrift
	mkdir --parents $(srcdir)/service/workspace-api/include/workspace-api
	mkdir --parents $(srcdir)/service/workspace-api/src
	thrift --gen cpp -I $(CCTHRIFT_INCLUDE) -o service/workspace-api $<
	thrift --gen js -I $(CCTHRIFT_INCLUDE) -o service/workspace-api $<
	cp -f $(srcdir)/service/workspace-api/gen-cpp/*.h $(srcdir)/service/workspace-api/include/workspace-api
	cp -f $(srcdir)/service/workspace-api/gen-cpp/*.cpp $(srcdir)/service/workspace-api/src
	rm  $(srcdir)/service/workspace-api/gen-cpp/WorkspaceService_server.skeleton.cpp
	touch workspace-api-rule
#copying javascripts file into webroot
workspaceapijsdir=$(jsdir)/workspace-api
workspaceapijs_DATA=\
	service/workspace-api/gen-js/WorkspaceService.js\
	service/workspace-api/gen-js/workspace_types.js

$(workspaceapijs_DATA) : workspace-api-rule

EXTRA_DIST+=service/workspace-api/workspace.thrift workspace-api-rule\
  $(workspaceapijs_DATA)

CLEANFILES+=service/workspace-api/gen-cpp/WorkspaceService_server.skeleton.cpp

GENCLEANFILES+=$(lib_libworkspace_api_la_SOURCES) workspace-api-rule \
  $(workspaceapijs_DATA)


install-data-hook::
	mkdir -p ${thriftapidir}/workspace-api
	cp -R $(srcdir)/service/workspace-api/*.thrift ${thriftapidir}/workspace-api

