install-data-hook::
	$(DX_ENV) $(DX_DOXYGEN) $(srcdir)/dox/Doxyfile
	mkdir -p $(webrootdir)/userguide
	mkdir -p $(webrootdir)/images
	cp -R $(srcdir)/dox/html/html/* $(webrootdir)/userguide
	cp -R $(srcdir)/dox/images/* $(webrootdir)/images

uninstall-hook::
	rm -rf $(webrootdir)/userguide
	rm -rf $(webrootdir)/images/userguide

DISTCLEANFILES += $(top_builddir)/db.options
