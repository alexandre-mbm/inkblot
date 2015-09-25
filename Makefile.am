## Process this file with automake to produce Makefile.in

SUBDIRS = src icons glade po

EXTRA_DIST =  mkinstalldirs NEWS README AUTHORS TODO ChangeLog \
	      inkblot.schemas.in inkblot.desktop.in inkblot.spec.in \
	      intltool-update.in intltool-merge.in intltool-extract.in

ACLOCAL_AMFLAGS = -I m4

@INTLTOOL_DESKTOP_RULE@

desktopdir = $(datadir)/applications
desktop_in_files=inkblot.desktop.in
desktop_files = $(desktop_in_files:.desktop.in=.desktop)
desktop_DATA = $(desktop_files)

@INTLTOOL_SCHEMAS_RULE@

schemadir = @GCONF_SCHEMA_FILE_DIR@
schema_in_files = inkblot.schemas.in
schema_DATA = $(schema_in_files:.schemas.in=.schemas)

CLEAN_FILES = $(schema_DATA) $(desktop_DATA)

install-data-local:
	if test -z "$(DESTDIR)" ; then \
	for p in $(schema_DATA) ; do \
	GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) $(GCONFTOOL) --makefile-install-rule $(top_builddir)/$$p; \
	done \
	fi

