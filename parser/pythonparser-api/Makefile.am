AM_CPPFLAGS+=-I$(srcdir)/parser/pythonparser-api/include

#include here the exported thrift headers
CCTHRIFT_GEN_INCLUDE+=-I$(srcdir)/parser/pythonparser-api/include/pythonparser-api

lib_LTLIBRARIES+=lib/libpythonparser-api.la
lib_libpythonparser_api_la_LDFLAGS= $(AM_LDFLAGS)
lib_libpythonparser_api_la_CPPFLAGS=$(AM_CPPFLAGS) $(CCTHRIFT_GEN_INCLUDE)
lib_libpythonparser_api_la_LIBADD= $(AM_LIBADD) -lthrift

lib_libpythonparser_api_la_SOURCES= \
	parser/pythonparser-api/src/pythonparser_constants.cpp \
	parser/pythonparser-api/include/pythonparser-api/pythonparser_constants.h \
	parser/pythonparser-api/src/pythonparser_types.cpp \
	parser/pythonparser-api/include/pythonparser-api/pythonparser_types.h \
	parser/pythonparser-api/src/PythonPersisterService.cpp \
	parser/pythonparser-api/include/pythonparser-api/PythonPersisterService.h \
	parser/pythonparser-api/include/pythonparser-api/pythonparserprocess.h \
	parser/pythonparser-api/src/pythonparserprocess.cpp

PYTHON_PARSER_API_JAVA_SRC=\
	parser/pythonparser-api/gen-java/cc/parser/PythonPersisterService.java \
	parser/pythonparser-api/gen-java/cc/parser/TFileLoc.java \
	parser/pythonparser-api/gen-java/cc/parser/TPythonAstNode.java \
	parser/pythonparser-api/gen-java/cc/parser/TPythonBinding.java \
	parser/pythonparser-api/gen-java/cc/parser/TPythonAttribute.java \
	parser/pythonparser-api/gen-java/cc/parser/TPythonClassDef.java \
	parser/pythonparser-api/gen-java/cc/parser/TPythonDecorator.java \
	parser/pythonparser-api/gen-java/cc/parser/TPythonFunctionDef.java \
	parser/pythonparser-api/gen-java/cc/parser/TPythonFunctionCall.java \
	parser/pythonparser-api/gen-java/cc/parser/TPythonFunctionParam.java \
	parser/pythonparser-api/gen-java/cc/parser/TPythonInheritance.java \
	parser/pythonparser-api/gen-java/cc/parser/TPythonReference.java \
	parser/pythonparser-api/gen-java/cc/parser/TPythonVariable.java \
	parser/pythonparser-api/gen-java/cc/parser/TPythonVariableRef.java \
	parser/pythonparser-api/gen-java/cc/parser/TPythonVariableRefKind.java \
	parser/pythonparser-api/gen-java/cc/parser/TPythonUnknown.java

$(lib_libpythonparser_api_la_SOURCES): pythonparser-api-rule
$(PYTHON_PARSER_API_JAVA_SRC): pythonparser-api-rule

$(srcdir)/pythonparser-api-rule: parser/pythonparser-api/pythonparser.thrift
	thrift --gen cpp -I $(CCTHRIFT_INCLUDE) -o parser/pythonparser-api $<
	thrift --gen java -I $(CCTHRIFT_INCLUDE) -o parser/pythonparser-api $<
	rm  $(srcdir)/parser/pythonparser-api/gen-cpp/PythonPersisterService_server.skeleton.cpp
	cp -f $(srcdir)/parser/pythonparser-api/gen-cpp/*.h $(srcdir)/parser/pythonparser-api/include/pythonparser-api
	cp -f $(srcdir)/parser/pythonparser-api/gen-cpp/*.cpp $(srcdir)/parser/pythonparser-api/src
	touch pythonparser-api-rule

EXTRA_DIST+=parser/pythonparser-api/pythonparser.thrift pythonparser-api-rule
GENCLEANFILES+= pythonparser-api-rule $(PYTHON_PARSER_API_JAVA_SRC)

