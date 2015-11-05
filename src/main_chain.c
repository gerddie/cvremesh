/***************************************************************************
 ** -*- mona-c++  -*-
 *            main.c
 *
 *  Copyright  2006-2015  Gert Wollny  gw.fossdev@gmail.com
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

 ****************************************************************************/

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>


#include <limits.h>
#include <gts.h>
#include <glib/gprintf.h>

#include <clusterface.h>
#include <edgepool.h>
#include <triangulate.h>
#include <approxvoronycluster.h>
#include <remeshcluster.h>
#include <cvenergy.h>
#include <cvcenergy.h>
#include <paenergy.h>

typedef struct {
	GLogFunc logger;
	GLogLevelFlags flags; 
} LogFilterData; 
	
void my_log_handler(const gchar *log_domain,
		    GLogLevelFlags log_level,
		    const gchar *message,
		    LogFilterData *lfd)
{
	if (log_level & lfd->flags) 
		lfd->logger(log_domain, log_level, message, lfd); 
}



int main(int argc, char **argv)
{
	gchar *in_filename=NULL;
	gchar *out_filename = NULL; 
	FILE *in_file; 
	FILE *out_file; 
	gint nclusters = 200; 
	gboolean debug = FALSE;
	gboolean message = FALSE; 
	gboolean opt_vertices = FALSE; 
	GtsSurface *in_mesh;
	GtsSurface *out_mesh; 
	gint errline; 
	
	
	GOptionEntry entries[] = {
		{ "in-mesh", 'i', 0, G_OPTION_ARG_STRING, &in_filename, "input mesh file", "mesh"},
		{ "out-mesh", 'o', 0, G_OPTION_ARG_STRING, &out_filename, "output mesh file", "mesh"},
		{ "nclusters", 'n', 0, G_OPTION_ARG_INT, &nclusters, "number of clusters", "clusters"},
		{ "vopt", 'v', 0, G_OPTION_ARG_NONE, &opt_vertices, "optimize vertex positions", NULL}, 
		{ "message", 'm', 0, G_OPTION_ARG_NONE, &message, "show message output", NULL },
		{ "debug", 'd', 0, G_OPTION_ARG_NONE, &debug, "show debugging output", NULL },
		{ NULL }
	};
	
	GError *error = NULL;
	GOptionContext *context;

	LogFilterData lfd; 
	lfd.logger = g_log_set_default_handler((GLogFunc)my_log_handler, &lfd); 
	lfd.flags = G_LOG_FLAG_RECURSION |G_LOG_FLAG_FATAL|G_LOG_LEVEL_ERROR|G_LOG_LEVEL_CRITICAL|G_LOG_LEVEL_WARNING; 
	g_log_set_always_fatal(lfd.flags); 
	
	context = g_option_context_new ("test functions");
	g_option_context_add_main_entries (context, entries, NULL);
	g_option_context_parse (context, &argc, &argv, &error);
	g_option_context_free(context); 
	context = NULL; 
	
	if (debug) 
		lfd.flags |= G_LOG_LEVEL_MESSAGE|G_LOG_LEVEL_INFO|G_LOG_LEVEL_DEBUG; 
	if (message)
		lfd.flags |= G_LOG_LEVEL_MESSAGE; 
	
	in_file = in_filename ? fopen(in_filename,"r"): stdin; 
	
	if (!in_file) {
		g_error("Unable to open input file %s for reading", in_filename); 
		return 1;
	}
	
	
	GtsFile *inf = gts_file_new(in_file); 
	if (!inf){
		g_error("Input file %s is not a GTS mesh", in_filename); 
		return 1;
	}
		
	in_mesh = gts_surface_new(gts_surface_class(), 
				  GTS_FACE_CLASS(gts_clusterface_class()), gts_edge_class(), gts_vertex_class()); 
	g_assert(in_mesh); 
	
	if ((errline = gts_surface_read(in_mesh, inf))) {
		g_error("Could not read mesh from %s; Error line %d", in_filename, errline);
		return 1;
	}
	gts_file_destroy(inf);
	
	
	GtsEnergy *energy1 = gts_energy_new(GTS_ENERGY_CLASS(gts_cvcenergy_class()));
	gts_surface_cluster(in_mesh, nclusters, energy1, TRUE, NULL, NULL);
	gts_cvenergy_destroy(energy1); 
	
	GtsEnergy *energy2 = gts_energy_new(GTS_ENERGY_CLASS(gts_paenergy_class()));
	gts_surface_cluster(in_mesh, nclusters, energy2, FALSE, NULL, NULL);
	gts_paenergy_destroy(energy2); 
	
	out_mesh = gts_surface_remesh_clusters(in_mesh, nclusters, opt_vertices);
	
	out_file = out_filename ? fopen(out_filename,"w"): stdout;
	if (!out_file) {
		g_error("Unable to open output file %s for writinf", in_filename);
		return 1;
	}
	gts_surface_write(out_mesh, out_file);
	gts_object_destroy(GTS_OBJECT(out_mesh)); 
	
	
	if (in_filename) 
		fclose(in_file); 
	
	if (out_filename) 
		fclose(out_file); 
	
	
	return 0; 
}
