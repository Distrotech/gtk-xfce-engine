@SET_MAKE@

SUBDIRS = 	\
	themes	\
	src
	
distclean-local:
	rm -rf *.cache *~

EXTRA_DIST = autogen.sh gtk-xfce-engine-2.spec gtk-xfce-engine-2.spec.in

rpm : dist
	rpmbuild -ta $(PACKAGE)-$(VERSION).tar.gz
	@rm -f $(PACKAGE)-$(VERSION).tar.gz

dist-bz2: dist
	zcat $(PACKAGE)-$(VERSION).tar.gz | bzip2 --best -c > $(PACKAGE)-$(VERSION).tar.bz2

distcheck-bz2: distcheck
	zcat $(PACKAGE)-$(VERSION).tar.gz | bzip2 --best -c > $(PACKAGE)-$(VERSION).tar.bz2
