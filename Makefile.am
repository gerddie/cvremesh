## Process this file with automake to produce Makefile.in
## Created by Anjuta - will be overwritten
## If you don't want it to overwrite it,
## 	Please disable it in the Anjuta project configuration

SUBDIRS = src

centroidvoronydocdir = ${prefix}/doc/centroidvorony
centroidvoronydoc_DATA = \
	README.md\
	COPYING\
	AUTHORS\
	ChangeLog\
	INSTALL


EXTRA_DIST = $(centroidvoronydoc_DATA)



# Copy all the spec files. Of cource, only one is actually used.
dist-hook:
	for specfile in *.spec; do \
		if test -f $$specfile; then \
			cp -p $$specfile $(distdir); \
		fi \
	done

