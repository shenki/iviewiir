ACLOCAL_AMFLAGS = -I m4
lib_LTLIBRARIES = libiview.la

libiview_la_SOURCES = \
		   libiview/get_xml_buffer.c \
		   libiview/parse_config.c \
		   libiview/get_index.c \
		   libiview/parse_index.c \
		   libiview/get_series_items.c \
		   libiview/parse_series_items.c \
		   libiview/destroy_config.c \
		   libiview/destroy_index.c \
		   libiview/parse_auth.c \
		   libiview/generate_video_uri.c \
		   libiview/fetch_video.c \
		   libiview/destroy_series_items.c

libiview_la_CFLAGS = $(DEPS_CFLAGS)
libiview_la_LIBADD = $(DEPS_LIBS)
libiview_la_LDFLAGS = -version-info @LIBIVIEW_SO_VERSION@

library_includedir=$(includedir)/libiview-@LIBIVIEW_API_VERSION@/libiview
library_include_HEADERS = libiview/iview.h
pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libiview/libiview-@LIBIVIEW_API_VERSION@.pc

bin_PROGRAMS = iviewiir
iviewiir_SOURCES = \
		   iviewiir.c \
		   iviewiir.h \
		   xdg-user-dir-lookup.c

iviewiir_LDADD = libiview.la
INCLUDES = -Ilibiview
