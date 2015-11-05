/***************************************************************************
 *            helper.c
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

/* test helper functions */
#include "helper.h"
#include "clusterface.h"


GtsSurface *create_std_surface() 
{
	return  gts_surface_new(gts_surface_class(), 
				gts_face_class(), 
				gts_edge_class(), 
				gts_vertex_class());
}

gboolean gts_vector_equal(GtsVector a, GtsVector b)
{
	GtsVector diff; 
	gts_vector_sub(diff, a, b)
	return gts_vector_scalar(diff,diff) < 1e-20; 
}


GtsSurface *create_clusterface_surface(guint nvtx, double vtx[][3], guint nedge, int edge[][2], guint nface, int face[][4])
{
	int i; 
	GtsVertex **v = g_new0(GtsVertex *, nvtx); 
	GtsEdge **e   = g_new0(GtsEdge *, nedge);
	GtsSurface *s = gts_surface_new(gts_surface_class(), GTS_FACE_CLASS(gts_clusterface_class()), 
					gts_edge_class(), gts_vertex_class());
	
	

	g_debug("Create test face with %d vertices, %d edges, and %d faces", 
		nvtx, nedge, nface); 
	for (i = 0; i < nvtx; ++i) 
		v[i] = gts_vertex_new(gts_vertex_class(), vtx[i][0], vtx[i][1], vtx[i][2]);
	
	for (i = 0; i < nedge; ++i)
		e[i] = gts_edge_new(gts_edge_class(), v[edge[i][0]], v[edge[i][1]]); 
	
	for (i = 0; i < nface; ++i)
		gts_surface_add_face(s, GTS_FACE(gts_clusterface_new(gts_clusterface_class(), 
								     e[face[i][0]], e[face[i][1]], 
								     e[face[i][2]], face[i][3]))); 
	g_free(v); 
	g_free(e); 

	g_debug("done");  
		

	return s; 
}

gint f_get_clusters(GtsClusterFace *f, SurfaceClusterParams *scp)
{
	g_assert(GTS_IS_CLUSTERFACE(f)); 
	gint cn = gts_clusterface_get_clusternr(f); 
	if (cn < 0)
		return 0; 
	if (cn < scp->nclusters)
		gts_surface_add_face(scp->clusters[cn], GTS_FACE(f));
	else
		g_warning("gts_surface_get_cluster_params: cluster no. %d out of given range", cn);
	return 0; 
}

void gts_surface_get_clusters(GtsSurface *s, SurfaceClusterParams *scp)
{
	gts_surface_foreach_face(s, (GtsFunc)f_get_clusters, scp);
}
