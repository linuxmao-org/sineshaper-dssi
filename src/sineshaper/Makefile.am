dssidir = @dssidir@
dssi_LTLIBRARIES = sineshaper.la
sineshaper_la_SOURCES = sineshaper.hpp sineshaper.cpp sineshaperports.hpp presetmanager.hpp presetmanager.cpp
sineshaper_la_LDFLAGS = -module -avoid-version
sineshaper_la_LIBADD = ../common/libdssi_plugin.la
sineshaper_la_CXXFLAGS = @DSSI_CFLAGS@ -I../common -I../components -DINSTALL_DIR=\"$(dssidir)\"


guidir = $(dssidir)/sineshaper/
gui_PROGRAMS = sineshaper_gtk
sineshaper_gtk_SOURCES = \
	main.cpp \
	presetmanager.cpp presetmanager.hpp \
	skindial_gtkmm.cpp skindial_gtkmm.hpp \
	sineshapergui.hpp sineshapergui.cpp \
	sineshaperports.hpp
sineshaper_gtk_LDADD = @DSSI_LIBS@ @LIBGLADEMM_LIBS@ @LIBLO_LIBS@ ../common/libdssi_ui.la ../common/libdssi_common.la
sineshaper_gtk_CXXFLAGS = @LIBLO_CFLAGS@ @DSSI_CFLAGS@ -D_XOPEN_SOURCE @LIBGLADEMM_CFLAGS@ -DINSTALL_DIR=\"$(dssidir)\" -I../common/ -I../gui-components

dist_bin_SCRIPTS = sineshaper.sh

gui_DATA = sineshaper.glade dial.png pixmaps/dummydial.png pixmaps/sineshaper.png presets
EXTRA_DIST = $(gui_DATA) README.sineshaper

# Stolen from swh-plugins, makes stupid libtool versions go away
install-dssiLTLIBRARIES: $(dssi_LTLIBRARIES)
	mkdir -p $(DESTDIR)/$(dssidir)
	list='$(dssi_LTLIBRARIES)'; \
	for file in $$list; do \
	  sofile=`basename $$file .la`.so; \
	  $(INSTALL_PROGRAM) .libs/$$sofile $(DESTDIR)/$(dssidir); \
	done
