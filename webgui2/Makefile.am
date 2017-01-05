install-data-hook::
	$(MKDIR_P) $(webrootdir)
	chmod a+w $(webrootdir)
	cp -R $(top_srcdir)/webgui2/html/* $(webrootdir)
	echo "var CC_GIT_VERSION=\"$(CC_GIT_VERSION)\";" \
	  > $(webrootdir)/scripts/codecompass/codecompass_rev.js
	chmod -R a+w $(webrootdir)
	find $(webrootdir) -name '.*' -and -not -name '.' -print0 | xargs -0 rm -rf

EXTRA_DIST += webgui2/html

uninstall-hook::
	rm -rf $(webrootdir)/images/
	rm -rf $(webrootdir)/scripts/
	rm -rf $(webrootdir)/style/
	rm -rf $(webrootdir)/index.html
	rm -rf $(webrootdir)/credits.html
	rm -rf $(webrootdir)/settings.html
	rm -rf $(webrootdir)/startpage.html
	rm -rf $(webrootdir)/userguide.html
	rm -rf $(top_builddir)/test/tmp_servicetest/
	
DISTCLEANFILES += $(top_builddir)/db.options
