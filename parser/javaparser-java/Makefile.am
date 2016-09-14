JAVAPARSERBASEDIR=parser/javaparser-java

JAVAPARSER_SRC=\
	$(JAVAPARSERBASEDIR)/src/parser/ArgParser.java \
	$(JAVAPARSERBASEDIR)/src/parser/AstVisitor.java \
	$(JAVAPARSERBASEDIR)/src/parser/BuildAction.java \
	$(JAVAPARSERBASEDIR)/src/parser/BuildLog.java \
	$(JAVAPARSERBASEDIR)/src/parser/BuildParameter.java \
	$(JAVAPARSERBASEDIR)/src/parser/BuildSource.java \
	$(JAVAPARSERBASEDIR)/src/parser/BuildTarget.java \
	$(JAVAPARSERBASEDIR)/src/parser/ConnStringParser.java \
	$(JAVAPARSERBASEDIR)/src/parser/DatabasePlatform.java \
	$(JAVAPARSERBASEDIR)/src/parser/FileContent.java \
	$(JAVAPARSERBASEDIR)/src/parser/File.java \
	$(JAVAPARSERBASEDIR)/src/parser/Frame.java \
	$(JAVAPARSERBASEDIR)/src/parser/JavaAstNode.java \
	$(JAVAPARSERBASEDIR)/src/parser/JavaEnumConstant.java \
	$(JAVAPARSERBASEDIR)/src/parser/JavaFunction.java \
	$(JAVAPARSERBASEDIR)/src/parser/JavaImport.java \
	$(JAVAPARSERBASEDIR)/src/parser/JavaMember.java \
	$(JAVAPARSERBASEDIR)/src/parser/JavaType.java \
	$(JAVAPARSERBASEDIR)/src/parser/JavaVariable.java \
	$(JAVAPARSERBASEDIR)/src/parser/JavaDocComment.java \
	$(JAVAPARSERBASEDIR)/src/parser/JavaAnnotation.java \
	$(JAVAPARSERBASEDIR)/src/parser/LogConfigurator.java \
	$(JAVAPARSERBASEDIR)/src/parser/NameExtractor.java \
	$(JAVAPARSERBASEDIR)/src/parser/NameMangler.java \
	$(JAVAPARSERBASEDIR)/src/parser/Option.java \
	$(JAVAPARSERBASEDIR)/src/parser/ParseException.java \
	$(JAVAPARSERBASEDIR)/src/parser/Parser.java \
	$(JAVAPARSERBASEDIR)/src/parser/PositionInfo.java \
	$(JAVAPARSERBASEDIR)/src/parser/ProblemHandler.java \
	$(JAVAPARSERBASEDIR)/src/parser/Project.java \
	$(JAVAPARSERBASEDIR)/src/parser/HtmlBuilder.java \
	$(JAVAPARSERBASEDIR)/src/parser/JavaParserServiceImpl.java \
	$(JAVAPARSERBASEDIR)/src/parser/Utils.java

lib/java/javaparser.jar: $(JAVAPARSER_SRC) $(JAVA_PARSER_API_JAVA_SRC)
	mkdir -p $(JAVAPARSERBASEDIR)/bin
	$(JAVAC) $(JAVAC_FLAGS) -cp $(JAVA_BUILD_CP):$(JAVAPARSERBASEDIR)/bin:$(JAVAPARSERBASEDIR)/src \
	  -d $(JAVAPARSERBASEDIR)/bin $(JAVAPARSER_SRC) $(JAVA_PARSER_API_JAVA_SRC)
	jar cfmv lib/java/javaparser.jar \
	     $(JAVAPARSERBASEDIR)/src/META-INF/MANIFEST.MF \
	  -C $(JAVAPARSERBASEDIR)/src META-INF/persistence.xml \
	  -C $(JAVAPARSERBASEDIR)/src META-INF/services/java.sql.Driver \
	  -C $(JAVAPARSERBASEDIR)/bin parser \
	  -C $(JAVAPARSERBASEDIR)/bin cc

javalib_DATA+=lib/java/javaparser.jar

CLEANFILES+=\
	lib/java/javaparser.jar

clean-local::
	-rm -rf $(JAVAPARSERBASEDIR)/bin/parser

