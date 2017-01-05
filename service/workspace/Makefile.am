webplugin_LTLIBRARIES+=webplugin/workspace.la
webplugin_workspace_la_CPPFLAGS=$(AM_CPPFLAGS) $(CCTHRIFT_GEN_INCLUDE)
webplugin_workspace_la_SOURCES=\
	service/workspace/src/plugin.cpp \
	service/workspace/src/workspaceservice.h \
	service/workspace/src/workspaceservice.cpp

webplugin_workspace_la_LIBADD=\
	$(AM_LIBADD) $(builddir)/lib/libworkspace-api.la -lthrift \
	$(top_builddir)/lib/libgrockerutil.la
webplugin_workspace_la_LDFLAGS=\
	-module -avoid-version -shared
