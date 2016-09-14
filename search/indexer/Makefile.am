SEARCHINDEXER_JAVA_BASEDIR=search/indexer
SEARCHINDEXER_JAVA_SRC=\
	$(SEARCHINDEXER_JAVA_BASEDIR)/src/cc/search/analysis/MonoTokenizer.java \
	$(SEARCHINDEXER_JAVA_BASEDIR)/src/cc/search/analysis/SourceAnalyzer.java \
	$(SEARCHINDEXER_JAVA_BASEDIR)/src/cc/search/analysis/tags/CTags.java \
	$(SEARCHINDEXER_JAVA_BASEDIR)/src/cc/search/analysis/tags/TagStream.java \
	$(SEARCHINDEXER_JAVA_BASEDIR)/src/cc/search/analysis/tags/TagGenerator.java \
	$(SEARCHINDEXER_JAVA_BASEDIR)/src/cc/search/analysis/tags/TagGeneratorManager.java \
	$(SEARCHINDEXER_JAVA_BASEDIR)/src/cc/search/analysis/tags/SourceTagGenerator.java \
	$(SEARCHINDEXER_JAVA_BASEDIR)/src/cc/search/indexer/AbstractIndexer.java \
	$(SEARCHINDEXER_JAVA_BASEDIR)/src/cc/search/indexer/FileIndexer.java \
	$(SEARCHINDEXER_JAVA_BASEDIR)/src/cc/search/indexer/FieldReIndexer.java \
	$(SEARCHINDEXER_JAVA_BASEDIR)/src/cc/search/indexer/Context.java \
	$(SEARCHINDEXER_JAVA_BASEDIR)/src/cc/search/indexer/IndexerTask.java \
	$(SEARCHINDEXER_JAVA_BASEDIR)/src/cc/search/indexer/app/Options.java \
	$(SEARCHINDEXER_JAVA_BASEDIR)/src/cc/search/indexer/app/Indexer.java \
	$(SEARCHINDEXER_JAVA_BASEDIR)/src/cc/search/indexer/util/IOHelper.java \
	$(SEARCHINDEXER_JAVA_BASEDIR)/src/cc/search/suggestion/DatabaseBuilder.java \
	$(SEARCHINDEXER_JAVA_BASEDIR)/src/cc/search/suggestion/DocumentIterator.java \
	$(SEARCHINDEXER_JAVA_BASEDIR)/src/cc/search/suggestion/TagInputIterator.java \
	$(SEARCHINDEXER_JAVA_BASEDIR)/src/cc/search/suggestion/UniqueInputIterator.java

lib/java/searchindexer.jar: lib/java/searchcommon.jar $(SEARCHINDEXER_JAVA_SRC)
	mkdir -p $(SEARCHINDEXER_JAVA_BASEDIR)/bin
	$(JAVAC) $(JAVAC_FLAGS) -cp $(JAVA_BUILD_CP):lib/java/searchcommon.jar:$(SEARCHINDEXER_JAVA_BASEDIR)/bin \
	  -d $(SEARCHINDEXER_JAVA_BASEDIR)/bin $(SEARCH_INDEXER_API_JAVA_SRC) \
	  $(SEARCHINDEXER_JAVA_SRC)
	jar cfmv lib/java/searchindexer.jar \
	  $(SEARCHINDEXER_JAVA_BASEDIR)/src/META-INF/MANIFEST.MF \
	  -C $(SEARCHINDEXER_JAVA_BASEDIR)/bin cc

$(SEARCHINDEXER_JAVA_BASEDIR)/searchindexer: lib/java/searchindexer.jar
	cp $(SEARCHINDEXER_JAVA_BASEDIR)/searchindexer.in $(SEARCHINDEXER_JAVA_BASEDIR)/searchindexer

bin_SCRIPTS+=$(SEARCHINDEXER_JAVA_BASEDIR)/searchindexer

javalib_DATA+=lib/java/searchindexer.jar

CLEANFILES+=\
	$(SEARCHINDEXER_JAVA_BASEDIR)/searchindexer \
	lib/java/searchindexer.jar

clean-local::
	-rm -rf $(SEARCHINDEXER_JAVA_BASEDIR)/bin/cc