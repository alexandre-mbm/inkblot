## Process this file with automake to produce Makefile.in

INCLUDES = \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	@PACKAGE_CFLAGS@

bin_PROGRAMS = inkblot

inkblot_SOURCES = \
	main.c inkblot.h detect.c \
	menu.c eggtrayicon.c eggtrayicon.h

inkblot_LDADD = @PACKAGE_LIBS@

