
AM_CPPFLAGS = \
	-I$(top_srcdir) \
	-DG_LOG_DOMAIN=\"gtk-xfce-engine\" \
	$(PLATFORM_CPPFLAGS)

enginedir = $(libdir)/gtk-3.0/$(GTK3_BINARY_VERSION)/theming-engines

engine_LTLIBRARIES = libxfce.la

BUILT_SOURCES = \
	xfce_typebuiltin.c \
	xfce_typebuiltin.h

libxfce_la_SOURCES = \
	xfce_theme_main.c \
	xfce_engine.c \
	xfce_engine.h \
	xfce_style_types.h \
	xfce_typebuiltin.c \
	xfce_typebuiltin.h

libxfce_la_CFLAGS = \
	$(GTK3_CFLAGS) \
	$(GLIB_CFLAGS) \
	$(GMODULE_CFLAGS) \
	$(PLATFORM_CFLAGS)

libxfce_la_LIBADD = \
	$(GTK3_LIBS) \
	$(GLIB_LIBS) \
	$(GMODULE_LIBS) \
	$(LIBM)

libxfce_la_LDFLAGS = \
	-module \
	-avoid-version \
	-no-undefined \
	$(PLATFORM_LDFLAGS)

CLEANFILES= $(BUILT_SOURCES)

EXTRA_DIST= \
	xfce_typebuiltin.c.template \
	xfce_typebuiltin.h.template

xfce_typebuiltin.h: xfce_style_types.h Makefile xfce_typebuiltin.h.template
	@( cd $(srcdir) && $(GLIB_MKENUMS) --template ./xfce_typebuiltin.h.template \
	./xfce_style_types.h ) >> xgen-gtbh \
	&& (cmp -s xgen-gtbh $@ || (echo "Creating $@..."; cp xgen-gtbh $@) ) \
	&& rm -f xgen-gtbh

xfce_typebuiltin.c: xfce_style_types.h Makefile xfce_typebuiltin.c.template
	@( cd $(srcdir) && $(GLIB_MKENUMS) --template ./xfce_typebuiltin.c.template \
	./xfce_style_types.h ) >> xgen-gtbc \
	&& (cmp -s xgen-gtbc $@ || (echo "Creating $@..."; cp xgen-gtbc $@) ) \
	&& rm -f xgen-gtbc

