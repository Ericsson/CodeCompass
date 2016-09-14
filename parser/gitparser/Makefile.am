AM_CPPFLAGS+=-I$(top_srcdir)/parser/gitparser/include
#AM_LDFLAGS=$(COVERAGE_LDFLAGS)

gitparserdir=$(pkgincludedir)/parser/gitparser
gitparser_HEADERS= \
    parser/gitparser/include/gitparser/gitblame.h \
    parser/gitparser/include/gitparser/gitblamehunk.h \
    parser/gitparser/include/gitparser/gitblameoptions.h \
    parser/gitparser/include/gitparser/gitblob.h \
    parser/gitparser/include/gitparser/gitclone.h \
    parser/gitparser/include/gitparser/gitcommit.h \
    parser/gitparser/include/gitparser/gitdiff.h \
    parser/gitparser/include/gitparser/gitexception.h \
    parser/gitparser/include/gitparser/githexoid.h \
    parser/gitparser/include/gitparser/gitobject.h \
    parser/gitparser/include/gitparser/gitoid.h \
    parser/gitparser/include/gitparser/gitparser.h \
    parser/gitparser/include/gitparser/gitpatch.h \
    parser/gitparser/include/gitparser/gitreference.h \
    parser/gitparser/include/gitparser/gitrepository.h \
    parser/gitparser/include/gitparser/gitrevwalk.h \
    parser/gitparser/include/gitparser/gitsignature.h \
    parser/gitparser/include/gitparser/gittag.h \
    parser/gitparser/include/gitparser/gittime.h \
    parser/gitparser/include/gitparser/gittree.h \
    parser/gitparser/include/gitparser/gittreeentry.h


#gitparser_HEADERS+=\
#    parser/gitparser/include/gitparser/nullparser.h\
#    parser/gitparser/include/gitparser/gitparser.h\
#    parser/gitparser/include/gitparser/internal/filelocutilbase.h
# noinst_HEADERS+=

lib_LTLIBRARIES+=lib/libgitparser.la
lib_libgitparser_la_SOURCES=\
	parser/gitparser/src/gitblame.cpp \
	parser/gitparser/src/gitblameoptions.cpp \
	parser/gitparser/src/gitblob.cpp \
	parser/gitparser/src/gitclone.cpp \
	parser/gitparser/src/gitcommit.cpp \
	parser/gitparser/src/gitdiff.cpp \
	parser/gitparser/src/gitexception.cpp \
	parser/gitparser/src/githexoid.cpp \
	parser/gitparser/src/gitinit.h \
	parser/gitparser/src/gitinit.cpp \
	parser/gitparser/src/gitobject.cpp \
	parser/gitparser/src/gitoid.cpp \
	parser/gitparser/src/gitparser.cpp \
	parser/gitparser/src/gitpatch.cpp \
	parser/gitparser/src/gitreference.cpp \
	parser/gitparser/src/gitrepository.cpp \
	parser/gitparser/src/gitrevwalk.cpp \
	parser/gitparser/src/gitsignature.cpp \
	parser/gitparser/src/gittag.cpp \
	parser/gitparser/src/gittime.cpp \
	parser/gitparser/src/gittree.cpp \
	parser/gitparser/src/gittreeentry.cpp
	
lib_libgitparser_la_LIBADD=$(AM_LIBADD) \
	$(top_builddir)/lib/libgrockerutil.la \
	$(top_builddir)/lib/libmodel.la \
	-lgit2 -lz -lssl -lcrypto -lboost_filesystem

lib_libgitparser_la_CPPFLAGS=$(AM_CPPFLAGS) $(LLVM_CPPFLAGS)
lib_libgitparser_la_CXXFLAGS=$(AM_CXXFLAGS) -fPIC
lib_libgitparser_la_LDFLAGS=-static $(AM_LDFLAGS)

#	$(top_builddir)/lib/libparser.la

check_PROGRAMS+=test/gitparsertest
test_gitparsertest_SOURCES= \
	parser/gitparser/test/gitparserblametest.cpp \
	parser/gitparser/test/gitparserblobtest.cpp \
	parser/gitparser/test/gitparserclonetest.cpp \
	parser/gitparser/test/gitparsercommittest.cpp \
	parser/gitparser/test/gitparserreferencetest.cpp \
	parser/gitparser/test/gitparserrevwalktest.cpp \
	parser/gitparser/test/gitparsertreetest.cpp \
	parser/gitparser/test/gitparsertest.cpp
test_gitparsertest_LDADD=$(LLVM_LIBS) $(TEST_LIBS) $(AM_LDADD) \
	$(top_builddir)/lib/libgrockerutil.la \
	$(top_builddir)/lib/libgitparser.la \
	$(top_builddir)/lib/libmodel.la \
	-lgit2 -lz -lssl -lcrypto
test_gitparsertest_LDFLAGS=$(TEST_LDFLAGS) $(AM_LDFLAGS) $(LLVM_LDFLAGS)
test_gitparsertest_CPPFLAGS=$(AM_CPPFLAGS) $(TEST_CPPFLAGS) \
	-I$(top_srcdir)/parser/gitparser/include \
	-I$(top_srcdir)/parser/gitparser/src \
	-DTOP_SRCDIR=\"$(abs_top_srcdir)/\"
	
	
