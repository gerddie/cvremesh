
/***************************************************************************
 *            run_tests.c
 *
 *  Copyright  2006-2015  Gert Wollny  gw.fossdev@gmail.com
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser Public License as published by
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


#include <stdlib.h>
#include <clusterface.h>
#include <edgepool.h>
#include <triangulate.h>
#include <approxvoronycluster.h>
#include <remeshcluster.h>
#include <clist.h>
#include <cvenergy.h>
#include <cvcenergy.h>
#include <paenergy.h>
#include <csurface.h>

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
	gboolean sysmalloc = FALSE;
	gboolean debug = FALSE;
	gboolean message = FALSE; 
	
	GOptionEntry entries[] = {
		{ "sysmalloc", 's', 0, G_OPTION_ARG_NONE, &sysmalloc, "Use the system malloc functions", NULL},
		{ "message", 'm', 0, G_OPTION_ARG_NONE, &message, "show message output", NULL },
		{ "debug", 'd', 0, G_OPTION_ARG_NONE, &debug, "show debugging output", NULL },
		
		{ NULL }
	};
	
	GError *error = NULL;
	GOptionContext *context;

	LogFilterData lfd; 
	lfd.logger = g_log_set_default_handler((GLogFunc)my_log_handler, &lfd); 
	lfd.flags = G_LOG_FLAG_RECURSION |G_LOG_FLAG_FATAL|G_LOG_LEVEL_ERROR|G_LOG_LEVEL_CRITICAL|G_LOG_LEVEL_WARNING; 
	
	
	context = g_option_context_new ("test functions");
	g_option_context_add_main_entries (context, entries, NULL);
	g_option_context_parse (context, &argc, &argv, &error);
	g_option_context_free(context); 
	//	context = NULL; 
	
	if (debug) 
		lfd.flags |= G_LOG_LEVEL_MESSAGE|G_LOG_LEVEL_INFO|G_LOG_LEVEL_DEBUG; 
	if (message)
		lfd.flags |= G_LOG_LEVEL_MESSAGE; 
		
	if (sysmalloc) {
		GMemVTable sysmem = {
			malloc, 
			realloc,   
			free, 
			calloc,
			malloc,
			realloc
		};
	
		g_mem_set_vtable(&sysmem);
	}

	g_clist_test(); 
	gts_energy_test(); 
	gts_cvenergy_test();
	gts_cvcenergy_test();
	gts_paenergy_test(); 
	
	
	gts_edge_pool_test();
	gts_polygon_triangulate_test();
	gts_triangulate_convex_polygon_test();
	gts_clusterface_test();
	gts_csurface_test();   
	gts_approx_vorony_clustering_test();

	gts_vertex_face_sort_test(); 
	
	gts_surface_remesh_clusters_test(); 
	return 0;
}
