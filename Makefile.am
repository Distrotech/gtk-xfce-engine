INCLUDES = $(XFCE_CFLAGS)

enginedir = $(libdir)/gtk-2.0/$(GTK_VERSION)/engines

engine_LTLIBRARIES = libxfce.la

libxfce_la_SOURCES = \
	xfce_theme_draw.c	\
	xfce_theme_main.c

libxfce_la_LDFLAGS = -module -avoid-version

libxfce_la_LIBADD = 

EXTRA_DIST = autogen.sh
