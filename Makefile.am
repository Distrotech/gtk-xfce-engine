@SET_MAKE@

INCLUDES = $(XFCE_CFLAGS)

enginedir = $(libdir)/gtk-2.0/$(GTK_VERSION)/engines

engine_LTLIBRARIES = libxfce.la

libxfce_la_SOURCES = \
	xfce_theme_draw.c	\
	xfce_theme_main.c	\
	xfce_rc_style.c	\
	xfce_rc_style.h	\
	xfce_style.h

libxfce_la_LDFLAGS = -module -avoid-version

libxfce_la_LIBADD = 

EXTRA_DIST = autogen.sh gtk-xfce-engine-2.spec gtk-xfce-engine-2.spec.in

rpm : dist
	rpm -ta $(PACKAGE)-$(VERSION).tar.gz
	@rm -f $(PACKAGE)-$(VERSION).tar.gz
