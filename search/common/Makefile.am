SEARCHCOMMON_JAVA_BASEDIR=search/common
SEARCHCOMMON_JAVA_SRC=\
        $(SEARCHCOMMON_JAVA_BASEDIR)/src/cc/search/analysis/tags/Tag.java \
        $(SEARCHCOMMON_JAVA_BASEDIR)/src/cc/search/analysis/tags/Tags.java \
        $(SEARCHCOMMON_JAVA_BASEDIR)/src/cc/search/analysis/LineInformations.java \
        $(SEARCHCOMMON_JAVA_BASEDIR)/src/cc/search/analysis/Location.java \
        $(SEARCHCOMMON_JAVA_BASEDIR)/src/cc/search/analysis/SourceTextAnalyzer.java \
        $(SEARCHCOMMON_JAVA_BASEDIR)/src/cc/search/analysis/SourceTextTokenizer.java \
        $(SEARCHCOMMON_JAVA_BASEDIR)/src/cc/search/common/IndexFields.java \
        $(SEARCHCOMMON_JAVA_BASEDIR)/src/cc/search/common/SuggestionDatabase.java \
        $(SEARCHCOMMON_JAVA_BASEDIR)/src/cc/search/common/NFSFriendlyLockFactory.java \
        $(SEARCHCOMMON_JAVA_BASEDIR)/src/cc/search/common/config/UnknownArgumentException.java \
        $(SEARCHCOMMON_JAVA_BASEDIR)/src/cc/search/common/config/InvalidValueException.java \
        $(SEARCHCOMMON_JAVA_BASEDIR)/src/cc/search/common/config/CommonOptions.java \
        $(SEARCHCOMMON_JAVA_BASEDIR)/src/cc/search/common/config/LogConfigurator.java \
        $(SEARCHCOMMON_JAVA_BASEDIR)/src/cc/search/common/ipc/IPCProcessor.java

lib/java/searchcommon.jar: $(SEARCHCOMMON_JAVA_SRC)
	mkdir -p $(SEARCHCOMMON_JAVA_BASEDIR)/bin
	$(JAVAC) $(JAVAC_FLAGS) -cp $(JAVA_BUILD_CP):$(SEARCHCOMMON_JAVA_BASEDIR)/bin \
	  -d $(SEARCHCOMMON_JAVA_BASEDIR)/bin $(SEARCHCOMMON_JAVA_SRC)
	jar cfmv lib/java/searchcommon.jar \
	  $(SEARCHCOMMON_JAVA_BASEDIR)/src/META-INF/MANIFEST.MF \
	  -C $(SEARCHCOMMON_JAVA_BASEDIR)/bin cc

javalib_DATA+=lib/java/searchcommon.jar

CLEANFILES+=\
	lib/java/searchcommon.jar

clean-local::
	-rm -rf $(SEARCHCOMMON_JAVA_BASEDIR)/bin/cc
