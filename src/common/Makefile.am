AM_CFLAGS = @LIBLO_CFLAGS@ @DSSI_CFLAGS@ @LIBGLADEMM_CFLAGS@ -D_XOPEN_SOURCE
AM_CXXFLAGS = $(AM_CFLAGS)

dssidir = @dssidir@

noinst_LTLIBRARIES = libdssi_common.la libdssi_plugin.la libdssi_ui.la
libdssi_plugin_la_SOURCES = dssiplugin.cpp dssiplugin.hpp
libdssi_common_la_SOURCES = dssi_shm.c dssi_shm.h ringbuffer.c ringbuffer.h
libdssi_ui_la_SOURCES = dssiuiclient.cpp dssiuiclient.hpp
