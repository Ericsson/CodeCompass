AM_CPPFLAGS+=-I$(srcdir)/service/core-api/include
#include here the exported thrift headers
CCTHRIFT_GEN_INCLUDE+=-I$(srcdir)/service/core-api/include/core-api
lib_libcore_api_la_CPPFLAGS=$(AM_CPPFLAGS) $(CCTHRIFT_GEN_INCLUDE)
lib_LTLIBRARIES+=lib/libcore-api.la

#CCTHRIFT_CORECOMMON_GENERATED=\
#service/core-api/gen-cpp/common_constants.cpp \
#service/core-api/gen-cpp/common_constants.h \
#service/core-api/gen-cpp/common_types.cpp \
#service/core-api/gen-cpp/common_types.h

CCTHRIFT_CORECOMMON_SOURCES=\
    service/core-api/src/common_constants.cpp \
    service/core-api/include/core-api/common_constants.h \
    service/core-api/src/common_types.cpp \
    service/core-api/include/core-api/common_types.h

CCTHRIFT_COREPROJECT_SOURCES=\
    service/core-api/src/project_constants.cpp\
    service/core-api/include/core-api/project_constants.h\
    service/core-api/src/project_types.cpp\
    service/core-api/include/core-api/project_types.h\
    service/core-api/src/ProjectService.cpp\
    service/core-api/include/core-api/ProjectService.h

CCTHRIFT_CORERESULT_SOURCES=\
    service/core-api/src/result_constants.cpp \
    service/core-api/include/core-api/result_constants.h \
    service/core-api/src/result_types.cpp \
    service/core-api/include/core-api/result_types.h


lib_libcore_api_la_SOURCES=$(CCTHRIFT_CORECOMMON_SOURCES)\
    $(CCTHRIFT_COREPROJECT_SOURCES)\
    $(CCTHRIFT_CORERESULT_SOURCES)

$(CCTHRIFT_CORECOMMON_SOURCES): $(srcdir)/core-api-common-rule

$(srcdir)/core-api-common-rule: service/core-api/common.thrift
	mkdir --parents $(srcdir)/service/core-api/include/core-api
	mkdir --parents $(srcdir)/service/core-api/include/core-api/src
	thrift --gen cpp -o service/core-api $<
	thrift --gen js -o service/core-api $<
	thrift --gen java -o service/core-api $<
	cp -f $(srcdir)/service/core-api/gen-cpp/common_*.h $(srcdir)/service/core-api/include/core-api
	cp -f $(srcdir)/service/core-api/gen-cpp/common_*.cpp $(srcdir)/service/core-api/src
	touch $@

EXTRA_DIST+=service/core-api/common.thrift core-api-common-rule

$(CCTHRIFT_COREPROJECT_SOURCES) :  $(srcdir)/core-api-project-rule

$(srcdir)/core-api-project-rule: service/core-api/project.thrift service/core-api/common.thrift
	mkdir --parents $(srcdir)/service/core-api/include/core-api
	mkdir --parents $(srcdir)/service/core-api/include/core-api/src
	thrift --gen cpp -o service/core-api $<
	thrift --gen js -o service/core-api $<
	thrift --gen java -o service/core-api $<
	cp -f $(srcdir)/service/core-api/gen-cpp/[pP]roject*.h $(srcdir)/service/core-api/include/core-api
	cp -f $(srcdir)/service/core-api/gen-cpp/[pP]roject*.cpp $(srcdir)/service/core-api/src
	touch $@

EXTRA_DIST+=service/core-api/project.thrift core-api-project-rule
CLEANFILES+=service/core-api/src/ProjectService_server.skeleton.cpp

$(CCTHRIFT_CORERESULT_SOURCES):  $(srcdir)/core-api-result-rule

$(srcdir)/core-api-result-rule: service/core-api/result.thrift service/core-api/common.thrift service/core-api/project.thrift
	mkdir --parents $(srcdir)/service/core-api/include/core-api
	mkdir --parents $(srcdir)/service/core-api/include/core-api/src
	thrift --gen cpp -o service/core-api $<
	thrift --gen js -o service/core-api $<	
	thrift --gen java -o service/core-api $<
	cp -f $(srcdir)/service/core-api/gen-cpp/[rR]esult*.h $(srcdir)/service/core-api/include/core-api
	cp -f $(srcdir)/service/core-api/gen-cpp/[rR]esult*.cpp $(srcdir)/service/core-api/src
	touch $@

EXTRA_DIST+=service/core-api/result.thrift core-api-result-rule

#copying javascripts file into webroot
corejsdir=$(jsdir)/core-api
corejs_DATA=\
	service/core-api/gen-js/ProjectService.js\
	service/core-api/gen-js/project_types.js\
	service/core-api/gen-js/common_types.js\
	service/core-api/gen-js/result_types.js

$(corejs_DATA): $(srcdir)/core-api-project-rule $(srcdir)/core-api-common-rule $(srcdir)/core-api-project-rule $(srcdir)/core-api-result-rule

CORE_API_JAVA_SRC=\
	service/core-api/gen-java/cc/service/core/AstNodeId.java \
	service/core-api/gen-java/cc/service/core/BasicFileInfo.java \
	service/core-api/gen-java/cc/service/core/BuildLog.java \
	service/core-api/gen-java/cc/service/core/Category.java \
	service/core-api/gen-java/cc/service/core/DatasourceError.java \
	service/core-api/gen-java/cc/service/core/DiagramId.java \
	service/core-api/gen-java/cc/service/core/DiagramType.java \
	service/core-api/gen-java/cc/service/core/FileId.java \
	service/core-api/gen-java/cc/service/core/FileInfo.java \
	service/core-api/gen-java/cc/service/core/FileParseStatus.java \
	service/core-api/gen-java/cc/service/core/FilePosition.java \
	service/core-api/gen-java/cc/service/core/FileRange.java \
	service/core-api/gen-java/cc/service/core/FileSearchResult.java \
	service/core-api/gen-java/cc/service/core/FileStat.java \
	service/core-api/gen-java/cc/service/core/FileType.java \
	service/core-api/gen-java/cc/service/core/HitCountResult.java \
	service/core-api/gen-java/cc/service/core/InvalidId.java \
	service/core-api/gen-java/cc/service/core/InvalidInput.java \
	service/core-api/gen-java/cc/service/core/InvalidPosition.java \
	service/core-api/gen-java/cc/service/core/InvalidRange.java \
	service/core-api/gen-java/cc/service/core/LineMatch.java \
	service/core-api/gen-java/cc/service/core/MenuType.java \
	service/core-api/gen-java/cc/service/core/MessageType.java \
	service/core-api/gen-java/cc/service/core/Position.java \
	service/core-api/gen-java/cc/service/core/ProjectId.java \
	service/core-api/gen-java/cc/service/core/ProjectInfo.java \
	service/core-api/gen-java/cc/service/core/ProjectService.java \
	service/core-api/gen-java/cc/service/core/Range.java \
	service/core-api/gen-java/cc/service/core/RangedHitCountResult.java \
	service/core-api/gen-java/cc/service/core/ReferenceType.java \
	service/core-api/gen-java/cc/service/core/SearchResult.java \
	service/core-api/gen-java/cc/service/core/SearchResultEntry.java \
	service/core-api/gen-java/cc/service/core/StatisticsInfo.java

$(CORE_API_JAVA_SRC): $(srcdir)/core-api-project-rule $(srcdir)/core-api-common-rule $(srcdir)/core-api-result-rule

EXTRA_DIST+=$(corejs_DATA)

GENCLEANFILES+= $(lib_libcore_api_la_SOURCES)\
	$(srcdir)/core-api-common-rule \
	$(srcdir)/core-api-project-rule \
	$(srcdir)/core-api-result-rule \
	$(CORE_API_JAVA_SRC)\
	$(corejs_DATA)


install-data-hook::
	mkdir -p ${thriftapidir}/core-api
	cp -R $(srcdir)/service/core-api/*.thrift ${thriftapidir}/core-api


	
