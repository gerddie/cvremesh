## Process this file with automake to produce Makefile.in

## Created by Anjuta - will be overwritten
## If you don't want it to overwrite it,
## 	Please disable it in the Anjuta project configuration

INCLUDES = @GTS_CFLAGS@  -DG_LOG_DOMAIN=\"ACV\"

AM_CFLAGS =\
	 -Wall\
	 -g

bin_PROGRAMS = centroidvorony\
	centroidvorony_a

check_PROGRAMS = run_tests

lib_LTLIBRARIES = libacv.la


libacv_la_SOURCES = \
	approxvoronycluster.c approxvoronycluster.h \
	clist.c clist.h	\
	clusterface.c clusterface.h \
	csurface.c csurface.h \
	cvenergy.c cvenergy.h \
	cvcenergy.c cvcenergy.h \
	edgepool.c edgepool.h \
	energy.h energy.c \
	helper.h helper.c \
	paenergy.c paenergy.h \
	remeshcluster.c remeshcluster.h \
	triangulate.c triangulate.h


libacv_la_LDFLAGS = @GTS_LIBS@ -lm

centroidvorony_SOURCES = \
	main.c

centroidvorony_LDFLAGS = @GTS_LIBS@ -lm 
centroidvorony_LDADD = libacv.la

centroidvorony_a_SOURCES = \
	main_chain.c
centroidvorony_a_LDFLAGS = @GTS_LIBS@ -lm 
centroidvorony_a_LDADD = libacv.la


run_tests_SOURCES = \
	run_tests.c

run_tests_LDFLAGS = @GTS_LIBS@ -lm
run_tests_LDADD = libacv.la 

#check: 
#	./run_tests
