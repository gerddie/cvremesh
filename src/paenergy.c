/***************************************************************************
 *            cvenergy.h
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


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <paenergy.h>
#include <helper.h>

static void paenergy_destroy(GtsObject *object)
{
	g_assert(GTS_IS_PAENERGY(object));
	g_free(GTS_PAENERGY(object)->area); 
	g_free(GTS_PAENERGY(object)->perimeter);
	(* GTS_OBJECT_CLASS (gts_paenergy_class ())->parent_class->destroy) (object);
}

static gboolean paenergy_move_face(GtsEnergy *energy, GtsClusterFace *f, gint old_cn, gint new_cn); 
static void paenergy_init_data(GtsEnergy *energy, GtsSurface *s, guint nclusters); 

static void paenergy_class_init(GtsPAEnergyClass *klass)
{
	GTS_OBJECT_CLASS (klass)->destroy = paenergy_destroy;
	GTS_ENERGY_CLASS(klass)->move_face = paenergy_move_face;  
	GTS_ENERGY_CLASS(klass)->init_data = paenergy_init_data; 
}

static void paenergy_init(GtsPAEnergy *paenergy)
{
	paenergy->area = NULL;  
	paenergy->perimeter = NULL;
	paenergy->n_clusters = 0; 
}

GtsPAEnergyClass * gts_paenergy_class(void)
{
	static GtsPAEnergyClass * klass = NULL;
	
	if (klass == NULL) {
		GtsObjectClassInfo paenergy_info = {
			"GtsPAEnergy",
			sizeof (GtsPAEnergy),
			sizeof (GtsPAEnergyClass),
			(GtsObjectClassInitFunc) paenergy_class_init,
			(GtsObjectInitFunc) paenergy_init,
			(GtsArgSetFunc) NULL,
			(GtsArgGetFunc) NULL
		};
		klass = gts_object_class_new (GTS_OBJECT_CLASS(gts_energy_class ()), 
					      &paenergy_info);
	}
	
	return klass;

}

GtsPAEnergy *    gts_paenergy_new(GtsPAEnergyClass *klass)
{
	return GTS_PAENERGY (gts_energy_new (GTS_ENERGY_CLASS (klass)));
}

static guint f_init_data(GtsClusterFace *f, GtsPAEnergy *energy) 
{
	g_assert(GTS_IS_CLUSTERFACE(f)); 
	gint cn = gts_clusterface_get_clusternr(f); 
	
	if (cn < 0) 
		return 0; 
	if (cn < energy->n_clusters) {
		energy->area[cn] = gts_clusterface_area(f);
		energy->perimeter[cn] = gts_triangle_perimeter(GTS_TRIANGLE(f)); 
	}else
		g_warning("Got clusternr %d not less then %d", cn, energy->n_clusters); 
	
	return 0;
}



static gdouble gts_edge_length(GtsSegment *s)
{
	if (!GTS_OBJECT(s)->reserved) {
		GTS_OBJECT(s)->reserved = g_new(gdouble, 1); 
		*((gdouble *)GTS_OBJECT(s)->reserved) = gts_point_distance(GTS_POINT(s->v1), GTS_POINT(s->v2));
	}
	return *((gdouble *)GTS_OBJECT(s)->reserved); 
}

static void paenergy_init_data(GtsEnergy *energy, GtsSurface *s, guint nclusters)
{
	g_assert(GTS_IS_PAENERGY(energy)); 
	GtsPAEnergy *e = GTS_PAENERGY(energy); 
	
	e->surface = s; 
	e->n_clusters = nclusters; 
	e->area = g_new0(gdouble, nclusters); 
	e->perimeter = g_new0(gdouble, nclusters);

	// evaluate the boundary length of each cluster
	SurfaceClusterParams scp; 
	scp.clusters = g_new(GtsSurface *, nclusters); 
	scp.nclusters = nclusters; 
	
	int i; 
	for (i = 0; i < nclusters; ++i) {
		scp.clusters[i] = gts_surface_new(gts_surface_class(), 
						  gts_face_class(), 
						  gts_edge_class(), 
						  gts_vertex_class()); 
	}
	
	gts_surface_get_clusters(s, &scp); 

	for (i = 0; i < nclusters; ++i) {
		GSList *b = gts_surface_boundary(scp.clusters[i]); 
		double l = 0.0;
		
		while (b) {
			l += gts_edge_length(GTS_SEGMENT(b->data)); 
			b = g_slist_next(b); 
		}
		e->area[i] = gts_surface_area(scp.clusters[i]); 
		e->perimeter[i] = l; 
		gts_object_destroy(GTS_OBJECT(scp.clusters[i]));
	}
	g_free(scp.clusters); 
}

inline static gdouble eval_cluster_enery(int n, const GtsPAEnergy *e) 
{
	return e->perimeter[n] / e->area[n]; 
} 


static void add_face(GtsPAEnergy *e, gdouble area, gdouble dperimeter, gint c_nr)
{
	e->area[c_nr] += area;
	e->perimeter[c_nr] += dperimeter;
}

static void del_face(GtsPAEnergy *e, gdouble area, gdouble dperimeter, gint c_nr)
{ 
	e->area[c_nr] -= area;
	e->perimeter[c_nr] +=  dperimeter;
}


static double get_signed_edge_length(GtsFace *f, gint cnf, GtsEdge *e,  GtsSurface *s)
{
	GtsFace *f1, *f2;
	if (!gts_edge_manifold_faces(e, s, &f1, &f2)) 
		return -gts_edge_length(GTS_SEGMENT(e));
	
	gint cn1 = gts_clusterface_get_clusternr(GTS_CLUSTERFACE(f1)); 
	gint cn2 = gts_clusterface_get_clusternr(GTS_CLUSTERFACE(f2));
	
	if ( ( (f == f1) && (cn2 == cnf) ) ||
	     ( (f == f2) && (cn1 == cnf) ) ) 
		return gts_edge_length(GTS_SEGMENT(e));
	else
		return -gts_edge_length(GTS_SEGMENT(e));
		
}

static double get_edge_old_old(GtsFace *f, gint old_cn, GtsSurface *s)
{
	gdouble result = 0.0;
	
	result += get_signed_edge_length(f, old_cn, GTS_TRIANGLE(f)->e1, s); 
	result += get_signed_edge_length(f, old_cn, GTS_TRIANGLE(f)->e2, s); 
	result += get_signed_edge_length(f, old_cn, GTS_TRIANGLE(f)->e3, s);
	
	return result; 
}

static double get_edge_old_new(GtsFace *f, gint new_cn, GtsSurface *s)
{
	gdouble result = 0.0;
	
	result -= get_signed_edge_length(f, new_cn, GTS_TRIANGLE(f)->e1, s); 
	result -= get_signed_edge_length(f, new_cn, GTS_TRIANGLE(f)->e2, s); 
	result -= get_signed_edge_length(f, new_cn, GTS_TRIANGLE(f)->e3, s);
	
	return result; 
}


static gboolean paenergy_move_face(GtsEnergy *energy, GtsClusterFace *f, gint old_cn, gint new_cn)
{
	g_assert(GTS_IS_PAENERGY(energy));
	g_assert(GTS_IS_CLUSTERFACE(f));
	g_assert(new_cn >= 0); 
	
	gdouble energy_0; 
	gdouble energy_1;
	gdouble area; 
	
	GtsPAEnergy *e = GTS_PAENERGY(energy);
	
	gdouble delta_old_new = get_edge_old_new(GTS_FACE(f), new_cn, e->surface); 
	area = gts_clusterface_area(f); 
	
	// if the face is in the free cluster, allow transformation
	if (old_cn == -1) {
		add_face(e, area, delta_old_new, new_cn); 
		return TRUE;
	}
	
	gdouble delta_old_old = get_edge_old_old(GTS_FACE(f), old_cn, e->surface);
	
	
	// energy term before face move 
	energy_0 = eval_cluster_enery(old_cn, e) + eval_cluster_enery(new_cn, e);
	
	energy_1 = (e->perimeter[old_cn] + delta_old_old )/ (e->area[old_cn] - area) + 
		(e->perimeter[new_cn] + delta_old_new)/ (e->area[new_cn] + area); 
	
	
	if (energy_0 > energy_1) {
		add_face(e, area, delta_old_new, new_cn);
		del_face(e, area, delta_old_old, old_cn);
		return TRUE; 
	}
	return FALSE;
}




void gts_paenergy_test()    
{
	
#define  nVtx   10
#define  nEdge  19 
#define  nFace  4

	int i; 
	gdouble v[nVtx][3] = { { -10,  20, 0}, 
			       { -10,   0, 0}, 
			       { -20, -20, 0}, 
			       {  0,   20, 0}, 
			       {  0,   10, 0}, 
			       {  0,  -10, 0},
			       {  0,  -20, 0},
			       {  20,  20, 0}, 
			       {  40,   0, 0}, 
			       {  20, -20, 0},
			          
	}; 
	gint e[nEdge][2] = { 
		{ 0, 1}, { 1, 2}, { 0, 3}, { 0, 4}, { 1, 4}, 
		{ 1, 5}, { 2, 5}, { 2, 6}, { 3, 4}, { 4, 5}, 
		{ 5, 6}, { 3, 7}, { 4, 7}, { 4, 8}, { 5, 8}, 
		{ 5, 9}, { 6, 9}, { 7, 8}, { 8, 9}
		};
	
	gint f[nFace][4] = { 
		{ 0,  3,  4, 0},
		{ 4,  9,  5, 1}, 
		{ 5,  6,  1, 1}, 
		{ 9, 13, 14, 0}
	};
	GtsVertex *vtx[nVtx]; 
	GtsEdge   *edge[nEdge]; 
	GtsFace   *face[nFace]; 
	
	const guint nclusters = 3; 
	
	for (i = 0; i < nVtx; ++i) 
		vtx[i] = gts_vertex_new(gts_vertex_class(), v[i][0], v[i][1], v[i][2]);
	
	for (i = 0; i < nEdge; ++i)
		edge[i] = gts_edge_new(gts_edge_class(), vtx[e[i][0]], vtx[e[i][1]]); 
	
	
	GtsSurface *s = gts_surface_new(gts_surface_class(), GTS_FACE_CLASS(gts_clusterface_class()), 
					gts_edge_class(), gts_vertex_class());
	for (i = 0; i < nFace; ++i) {
		g_message("create face %d", i); 
		face[i] = GTS_FACE(gts_clusterface_new(gts_clusterface_class(), 
						       edge[f[i][0]], edge[f[i][1]], 
						       edge[f[i][2]], f[i][3])); 
		
		gts_surface_add_face(s, GTS_FACE(face[i])); 
	}
	
	GtsPAEnergy *paenergy = gts_paenergy_new(gts_paenergy_class()); 
	g_assert(GTS_IS_PAENERGY(paenergy));

	gts_energy_init_data(GTS_ENERGY(paenergy), s, nclusters);

	g_debug("get_edge_old_old 1 = %f", get_edge_old_old(face[1], 1, s)); 
	g_assert( get_edge_old_old(face[1], 1, s) == -20); 

	g_debug("get_edge_old_new 0 = %f", get_edge_old_new(face[1], 0, s)); 
	g_assert( get_edge_old_new(face[1], 0, s) == -20); 


	g_assert( get_edge_old_new(face[0], 1, s) == 20); 

	
	gts_paenergy_destroy(paenergy);	
}


