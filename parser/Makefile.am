include $(srcdir)/parser/parser/Makefile.am
include $(srcdir)/parser/fileparser/Makefile.am
include $(srcdir)/parser/projectparser/Makefile.am
include $(srcdir)/parser/cxxparser/Makefile.am
include $(srcdir)/parser/logger/Makefile.am
include $(srcdir)/parser/searchparser/Makefile.am
include $(srcdir)/parser/scripts/Makefile.am
include $(srcdir)/parser/javaparser/Makefile.am
include $(srcdir)/parser/javaparser-api/Makefile.am
include $(srcdir)/parser/javaparser-java/Makefile.am
include $(srcdir)/parser/gitparser/Makefile.am
include $(srcdir)/parser/reparser/Makefile.am
include $(srcdir)/parser/metricsparser/Makefile.am

# It's ugly but I don't have a better idea
PYTHONPARSERBASEDIR=parser/pythonparser-java
clean-local::
	-rm -rf $(PYTHONPARSERBASEDIR)/bin

if ENABLE_PYTHON_PARSER
include $(srcdir)/parser/pythonparser-api/Makefile.am
include $(srcdir)/parser/pythonparser/Makefile.am
include $(srcdir)/parser/pythonparser-java/Makefile.am
endif
