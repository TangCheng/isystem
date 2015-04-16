## Process this file with automake to produce Makefile.in


AM_CPPFLAGS = \
	-DPACKAGE_LOCALE_DIR=\""$(localedir)"\" \
	-DPACKAGE_SRC_DIR=\""$(srcdir)"\" \
	-DPACKAGE_DATA_DIR=\""$(pkgdatadir)"\" \
	$(ISYSTEM_CFLAGS)


bin_PROGRAMS =  \
	vtoold

vtoold_SOURCES =  \
	main.c \
	sysutils.c \
	sysutils.h

vtoold_LDFLAGS = 

vtoold_LDADD = $(ISYSTEM_LIBS)
