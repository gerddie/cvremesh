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

#include <gts.h>
#include <glib/gprintf.h>
#include "clusterface.h"
#include "approxvoronycluster.h"
#include "helper.h"
#include <cvenergy.h>
#include <paenergy.h>


typedef struct {
	guint cluster; 
	gdouble area; 
	GtsVector x; 
} CenterAreaData;

static gint f_get_center_and_area(GtsClusterFace *f, CenterAreaData *cad)
{
	if (gts_clusterface_get_clusternr(f) == cad->cluster) {
		GtsVector c; 
		gdouble area = gts_clusterface_wcentroid(f, c); 
		cad->area += area;
		cad->x[0] += c[0]; 
		cad->x[1] += c[1];
		cad->x[2] += c[2]; 
	}
	return 0; 
}

gdouble gts_surface_center_of_cluster_area(GtsSurface *s, guint nr, GtsVector center)
{
	CenterAreaData cad = {nr, 0.0, {0.0, 0.0, 0.0}}; 
	gts_surface_foreach_face(s, (GtsFunc)f_get_center_and_area, &cad); 
	if (cad.area > 0) {
		center[0] = cad.x[0] / cad.area;
		center[1] = cad.x[1] / cad.area;
		center[2] = cad.x[2] / cad.area;
	}
	return cad.area; 

}
	

typedef struct  {
	gdouble step; 
	gdouble next; 
	gint cluster; 
	gint max_cluster; 
} ClusterProcInit; 



static gint f_cluster_init(GtsClusterFace *f, ClusterProcInit *cpi) 
{
	g_assert(GTS_IS_CLUSTERFACE(f)); 
	
	gts_clusterface_set_clusternr(f, -1); 
	gts_clusterface_set_generation(f, 0); 
	cpi->next += 1.0; 
	
	if (cpi->next >= cpi->step && cpi->cluster < cpi->max_cluster) {
		gts_clusterface_set_clusternr(f, cpi->cluster++); 
		cpi->next -= cpi->step;
		gts_clusterface_set_generation(f, 0);
	}
	
	return cpi->cluster >= cpi->max_cluster; 
}


static void surface_initialise_clustering(GtsSurface *s, gint nclusters)
{
	ClusterProcInit cpi= {0.0, 0.0, 0, nclusters};
	
	cpi.step = gts_surface_face_number(s) / nclusters;
	g_assert(cpi.step >= 2.0);
	
	gts_surface_foreach_face(s, (GtsFunc)f_cluster_init, &cpi);
}

typedef struct  {
	GtsSurface *master;
	guint updated;
	guint free_faces; 
	GtsEnergy *energy;
	guint *nfaces; 
	guint *change_generation; 
	guint generation; 
} ClusterPrepB;


static gint f_update_clusters(GtsClusterFace *f, ClusterPrepB *cp) 
{
	gint new_cn; 
	GSList *neighbours; 
	
		
	new_cn = gts_clusterface_get_clusternr(f); 
	
	// from the free cluster, no work needed 
	if (new_cn < 0 ) {
		++cp->free_faces; 
		return 0;
	}

	if (gts_clusterface_get_generation(f) >= cp->generation)
		return 0; 
	
	neighbours = gts_clusterface_get_noncluster_neighbours(f, cp->master);  
		
	while (neighbours) {
		GtsClusterFace *fi = GTS_CLUSTERFACE(neighbours->data);
		gint old_cn = gts_clusterface_get_clusternr(fi);
		
		if (old_cn != -1 && 
		    cp->generation - cp->change_generation[new_cn] > 2 &&
		    cp->generation - cp->change_generation[old_cn] > 2){
			neighbours = g_slist_next(neighbours);
			continue; 
		}
		if ((old_cn == -1 || cp->nfaces[old_cn] > 1) && 
		    gts_energy_move_face(cp->energy, fi, old_cn, new_cn)) {
			
			gts_clusterface_set_clusternr(fi, new_cn);
			gts_clusterface_set_generation(fi, cp->generation); 
			
			cp->change_generation[new_cn] = cp->generation;
			  
			
			++cp->nfaces[new_cn];
			++cp->updated;
			
			if (old_cn > -1) {
				--cp->nfaces[old_cn];
				cp->change_generation[old_cn] = cp->generation;
			}
		} 
		neighbours = g_slist_next(neighbours);
	}
	g_slist_free(neighbours);
	return 0;
}



static gint f_prepare_value_cache(GtsClusterFace *f, ClusterPrepB *cp) 
{
	gint cn = gts_clusterface_get_clusternr(f); 
	if (cn >= 0) 
		++cp->nfaces[cn]; 
	
	return 0; 
}


typedef struct {
	gdouble  *max_area; 
	GSList  **clusters; 
	guint free_faces; 
	GtsSurface *surface; 
	guint *nfaces;
} ConnVisitData; 


static gint f_scan_cluster(GtsClusterFace *f, ConnVisitData *cvd)
{
	gdouble area = 0.0; 
	gint cn; 
	
	if (f->reserved) 
		return 0; 
	
	f->reserved = 1; 
	
	cn = gts_clusterface_get_clusternr(f); 
	
	if (cn < 0) 
		return 0;
	
	area = gts_triangle_area(GTS_TRIANGLE(f)); 
	
	GSList *cluster = g_slist_prepend(NULL, f);  
	GSList *real_cluster = g_slist_prepend(NULL, f);
	
	while (cluster) {
		GSList* neighbors = gts_face_neighbors(GTS_FACE(cluster->data), cvd->surface);
		cluster = g_slist_delete_link(cluster, cluster); 
		while (neighbors) {
			GtsClusterFace *fi = GTS_CLUSTERFACE(neighbors->data); 
			if (gts_clusterface_get_clusternr(fi) == cn && !fi->reserved) {
				real_cluster = g_slist_prepend(real_cluster, fi);
				cluster = g_slist_prepend(cluster, fi); 
				fi->reserved = 1; 
				area += gts_clusterface_area(fi);
			}
			neighbors = g_slist_delete_link(neighbors, neighbors);
		}
	}
	
	if (cvd->clusters[cn]) {
		g_message("Found disconnected cluster %d, area %f vs. %f", cn, area, cvd->max_area[cn]);  

		if (area > cvd->max_area[cn]) {
			GSList *help = cvd->clusters[cn];
			cvd->clusters[cn] = real_cluster; 
			real_cluster = help;
			area = cvd->max_area[cn]; 
		}
		while (real_cluster) {
			gts_clusterface_set_clusternr(GTS_CLUSTERFACE(real_cluster->data), -1); 
			++cvd->free_faces; 
			--cvd->nfaces[cn]; 
			real_cluster = g_slist_delete_link(real_cluster,real_cluster); 
		}
	}else{
		cvd->clusters[cn] = real_cluster; 
		cvd->max_area[cn] = area; 
	}
	return 0; 
}


static gint f_clean_face_visited_flags(GtsClusterFace *f, gpointer data) 
{
	g_assert(GTS_IS_CLUSTERFACE(f));
	f->reserved = 0; 
	return 0; 
}


static guint check_cluster_connectivity(GtsSurface *surface, guint n, guint *nfaces)
{
	gint i; 
	ConnVisitData cvd;
	cvd.max_area = g_new0(gdouble, n);
	cvd.clusters = g_new0(GSList *, n); 
	cvd.free_faces = 0; 
	cvd.surface = surface; 
	cvd.nfaces = nfaces; 
	
	gts_surface_foreach_face(surface, (GtsFunc)f_clean_face_visited_flags, NULL); 
	
	gts_surface_foreach_face(surface, (GtsFunc)f_scan_cluster, &cvd); 
	
	for (i = 0; i < n; ++i) 
		g_slist_free(cvd.clusters[i]); 
	
		
	g_free(cvd.clusters); 
	g_free(cvd.max_area); 
	
	return cvd.free_faces; 
	
}

static gint f_clean_reserved(GtsObject *obj, gpointer data)
{
	obj->reserved = NULL; 
	return 0; 
}


static gint f_clean_reserved2(GtsObject *obj, gpointer data)
{
	g_free(obj->reserved);
	return 0; 
}

static gint f_reset_generation(GtsClusterFace *face, gpointer data)
{
	gts_clusterface_set_generation(face, 0); 
	return 0; 
}
     

void gts_surface_cluster(GtsSurface *surface, guint n, GtsEnergy *energy, gboolean init, 
			 DisplayCallback display, gpointer userdata)
{
	ClusterPrepB cp;
	
	g_assert(GTS_IS_ENERGY(energy)); 
	g_assert(GTS_CLUSTERFACE_CLASS(surface->face_class) == gts_clusterface_class());
	
	if (init)
		surface_initialise_clustering(surface, n);
	else
		gts_surface_foreach_face(surface, (GtsFunc)f_reset_generation, NULL); 
	
	cp.energy = energy; 
	cp.master = surface;
	cp.nfaces = g_new0(guint, n); 
	cp.change_generation = g_new0(guint, n);
	cp.generation = 0; 
	
	gts_surface_foreach_face(surface, (GtsFunc)f_prepare_value_cache, &cp);  
	gts_surface_foreach_edge(surface, (GtsFunc)f_clean_reserved, NULL); 

		
	gts_energy_init_data(energy, surface, n); 

	
	do {
		do {
			++cp.generation;
			cp.free_faces = 0; 
			cp.updated = 0;
			gts_surface_foreach_face(surface, (GtsFunc)f_update_clusters, &cp);
			g_message("Generation: %3d Updated: %7d Free %7d ", cp.generation, cp.updated, cp.free_faces);  
			if (display) {
				display(surface, userdata); 
			}
			 
		} while (cp.updated > 0 || cp.free_faces); 
		
		cp.free_faces = check_cluster_connectivity(surface, n, cp.nfaces); 
		if (cp.free_faces) {
			g_message("Found %d disconnected faces.", cp.free_faces); 
		}
	} while (cp.free_faces > 0);

	gts_surface_foreach_edge(surface, (GtsFunc)f_clean_reserved2, NULL); 
	

	g_free(cp.change_generation); 
	g_free(cp.nfaces);
}

GtsSurface *gts_surface_cluster_copy(GtsSurface *s, guint n, GtsEnergy *energy, DisplayCallback display, gpointer userdata)
{
	GtsSurface *result = gts_surface_new(gts_surface_class(), 
					     GTS_FACE_CLASS(gts_clusterface_class()), 
					     gts_edge_class(), 
					     gts_vertex_class());
	
	
	gts_surface_copy(result, s);
	gts_surface_cluster(result, n, energy, TRUE, display, userdata); 
	return result; 
}


	


void gts_cluster_connect_test()
{
#define  nVtx   5
#define  nEdge  7 
#define  nFace  3

	guint nfaces[2] = {2, 1};  
	double v[nVtx][3] = { { 0.0, 0.0, 0.0},
			       {-1.0, 0.0, 0.0},
			       {-1.5, 1.0, 0.0},
			       {-0.5, 1.5, 0.0},
			       { 0.5, 1.5, 0.0}
	}; 
	
	int e[nEdge][2] = {{ 0, 1}, // 0 
			   { 0, 2}, // 1
			   { 0, 3}, // 2
			   { 0, 4}, // 3
			   { 1, 2}, // 4
			   { 2, 3}, // 5 
			   { 3, 4}}; // 6 
	
	int f[nFace][4] = { {0, 4, 1, 0}, { 1, 5, 2, 1}, { 2, 6, 3, 0} }; 
	
	
	
	GtsSurface *surface = create_clusterface_surface(nVtx, v, nEdge, e, nFace, f); 
	guint free_faces = check_cluster_connectivity(surface, 2, nfaces); 

	g_message("free faces = %d", free_faces);  
	g_assert( free_faces == 1);

	gts_object_destroy(GTS_OBJECT(surface)); 
#undef  nVtx   
#undef  nEdge   
#undef  nFace  

}
void 	gts_approx_vorony_clustering_test()
{
#define save_files 1
#define  nVtx   10
#define  nEdge  19 
#define  nFace  10
	
	gdouble v[nVtx][3] = { { -40,  30, 0}, 
			       { -50,   0, 0}, 
			       { -40, -30, 0}, 
			       {  0,   20, 0}, 
			       {  0,    5, 0}, 
			       {  0,   -5, 0},
			       {  0,  -20, 0},
			       {  40,  30, 0}, 
			       {  50,   0, 0}, 
			       {  40, -30, 0},
			          
	}; 
	gint e[nEdge][2] = { 
		{ 0, 1}, { 1, 2}, { 0, 3}, { 0, 4}, { 1, 4}, 
		{ 1, 5}, { 2, 5}, { 2, 6}, { 3, 4}, { 4, 5}, 
		{ 5, 6}, { 3, 7}, { 4, 7}, { 4, 8}, { 5, 8}, 
		{ 5, 9}, { 6, 9}, { 7, 8}, { 8, 9}
		};
	
	gint f[nFace][4] = { 
		{ 0,  3,  4, -1}, 
		{ 1,  5,  6, -1}, 
		{ 2,  8,  3,  0}, 
		{ 4,  9,  5, -1}, 
		{ 7,  6, 10, -1},
		{ 8, 11, 12, -1}, 
		{ 9, 13, 14,  1}, 
		{10, 15, 16, -1}, 
		{13, 12, 17, -1}, 
		{14, 18, 15, -1}
	};
	
	
	GtsVertex *vtx[nVtx]; 
	GtsEdge   *edge[nEdge]; 
	GtsFace   *face[nFace];
	GtsSurface *s, *c1, *c2; 
	GtsEnergy *energy; 
	gdouble area_c1, area_c2, area_s; 
	GtsVector c_c1, c_c2, c_s, c_r1, c_r2;


	int i; 
	FILE *file; 

	gts_cluster_connect_test();
	
	for (i = 0; i < nVtx; ++i) {
		vtx[i] = gts_vertex_new(gts_vertex_class(), v[i][0], v[i][1],v[i][2]); 
	}
	
	for (i = 0; i < nEdge; ++i) {
		edge[i] = gts_edge_new(gts_edge_class(), vtx[e[i][0]], vtx[e[i][1]]); 
	}
	
	for (i = 0; i < nFace; ++i) {
		face[i] = GTS_FACE(gts_clusterface_new(gts_clusterface_class(),
				       edge[f[i][0]], edge[f[i][1]], edge[f[i][2]], f[i][3])); 
	}
	
	s = gts_surface_new(gts_surface_class(), 
			    GTS_FACE_CLASS(gts_clusterface_class()), 
			    gts_edge_class(), 
			    gts_vertex_class());
	
	
	c1 = create_std_surface();
	c2 = create_std_surface();
	
	

	for (i = 0; i < nFace; ++i) {
		gts_surface_add_face(s, face[i]); 
	}

	for (i = 0; i < nFace/2; ++i) {
		gts_surface_add_face(c2, face[i]); 
	}

	for (i = nFace/2; i < nFace; ++i) {
		gts_surface_add_face(c1, face[i]); 
	}


	if (save_files) {
		file = fopen("testsurface.gts", "w"); 
		gts_surface_write(s, file); 
		fclose(file);
		file = fopen("testsurface_c1.gts", "w"); 
		gts_surface_write(c1, file); 
		fclose(file);
		file = fopen("testsurface_c2.gts", "w"); 
		gts_surface_write(c2, file); 
		fclose(file); 
	}

	area_c1 = gts_surface_center_of_area(c1, c_c1); 
	area_c2 = gts_surface_center_of_area(c2, c_c2); 
	area_s  = gts_surface_center_of_area(s, c_s);
	
	g_assert(area_s == area_c1 + area_c2); 

	energy = GTS_ENERGY(gts_cvenergy_new(gts_cvenergy_class()));
	gts_surface_cluster(s, 2, energy, TRUE, NULL,NULL);
	
	
	
	gdouble area_r1 = gts_surface_center_of_cluster_area(s, 0, c_r1); 
	gdouble area_r2 = gts_surface_center_of_cluster_area(s, 1, c_r2); 
	

	g_message(" area_c1 = %f, area_r1 = %f, area_c2= %f, area_r2= %f\n", 
		 area_c1, area_r1, area_c2, area_r2); 
	
	
	g_message("c1: %e %e %e, r1: %e %e %e\nc1: %e %e %e, r1: %e %e %e\n", 
		 c_c1[0], c_c1[1], c_c1[2],
		 c_r1[0], c_r1[1], c_r1[2],
		 c_c2[0], c_c2[1], c_c2[2],
		 c_r2[0], c_r2[1], c_r2[2]); 
		
	if (!((area_c1 == area_r1 && area_c2 == area_r2 && 
	       gts_vector_equal(c_c2, c_r2) && 
	       gts_vector_equal(c_c1, c_r1)) ||
	      (area_c1 == area_r2 && area_c2 == area_r1 && 
	       gts_vector_equal(c_c2, c_r1) && 
	       gts_vector_equal(c_c1, c_r2)))) {
		g_assert(!"result not as expected"); 
		
	}

	gts_energy_destroy(energy); 
	gts_object_destroy(GTS_OBJECT(c2));
	gts_object_destroy(GTS_OBJECT(c1));
	gts_object_destroy(GTS_OBJECT(s));

	 
#undef save_files 
#undef  nVtx   
#undef  nEdge   
#undef  nFace  
	
	
}
