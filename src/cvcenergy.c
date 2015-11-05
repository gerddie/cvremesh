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

#include <cvcenergy.h>
#include <helper.h>

static void cvcenergy_destroy(GtsObject *object)
{
	g_assert(GTS_IS_CVCENERGY(object));
	g_free(GTS_CVCENERGY(object)->area);
	g_free(GTS_CVCENERGY(object)->angle);
	g_free(GTS_CVCENERGY(object)->wcentroid);
	(* GTS_OBJECT_CLASS (gts_cvcenergy_class ())->parent_class->destroy) (object);
}

static gboolean cvcenergy_move_face(GtsEnergy *energy, GtsClusterFace *f, gint old_cn, gint new_cn); 
static void cvcenergy_init_data(GtsEnergy *energy, GtsSurface *s, guint nclusters); 

static void cvcenergy_class_init(GtsCVCEnergyClass *klass)
{
	GTS_OBJECT_CLASS (klass)->destroy = cvcenergy_destroy;
	GTS_ENERGY_CLASS(klass)->move_face = cvcenergy_move_face;  
	GTS_ENERGY_CLASS(klass)->init_data = cvcenergy_init_data; 
}

static void cvcenergy_init(GtsCVCEnergy *cvcenergy)
{
	cvcenergy->area = NULL;  
	cvcenergy->surface = NULL; 
	cvcenergy->angle = NULL;  
	cvcenergy->wcentroid = NULL;
	cvcenergy->n_clusters = 0; 
}

GtsCVCEnergyClass * gts_cvcenergy_class(void)
{
	static GtsCVCEnergyClass * klass = NULL;
	
	if (klass == NULL) {
		GtsObjectClassInfo cvcenergy_info = {
			"GtsCVCEnergy",
			sizeof (GtsCVCEnergy),
			sizeof (GtsCVCEnergyClass),
			(GtsObjectClassInitFunc) cvcenergy_class_init,
			(GtsObjectInitFunc) cvcenergy_init,
			(GtsArgSetFunc) NULL,
			(GtsArgGetFunc) NULL
		};
		klass = gts_object_class_new (GTS_OBJECT_CLASS(gts_energy_class ()), 
					      &cvcenergy_info);
	}
	
	return klass;

}

GtsCVCEnergy *    gts_cvcenergy_new(GtsCVCEnergyClass *klass)
{
	return GTS_CVCENERGY (gts_energy_new (GTS_ENERGY_CLASS (klass)));
}

static guint f_init_data(GtsClusterFace *f, GtsCVCEnergy *energy) 
{
	g_assert(GTS_IS_CLUSTERFACE(f)); 
	gint cn = gts_clusterface_get_clusternr(f); 
	
	if (cn < 0) 
		return 0; 
	if (cn < energy->n_clusters) {
		GtsVector c; 
		gdouble area = gts_clusterface_wcentroid(f, c); 
		energy->area[cn] += area; 
		energy->angle[cn] = 0;  
		energy->wcentroid[cn][0] += c[0];
		energy->wcentroid[cn][1] += c[1];
		energy->wcentroid[cn][2] += c[2];
	}else
		g_warning("Got clusternr %d not less then %d", cn, energy->n_clusters); 
	
	return 0;
}

static void cvcenergy_init_data(GtsEnergy *energy, GtsSurface *s, guint nclusters)
{
	g_assert(GTS_IS_CVCENERGY(energy)); 
	GtsCVCEnergy *e = GTS_CVCENERGY(energy); 
	
	e->n_clusters = nclusters; 
	e->area = g_new0(gdouble, nclusters);
	e->surface = s; 
	e->angle = g_new0(gdouble, nclusters);
	e->wcentroid = g_new0(GtsVector, nclusters); 
	
	gts_surface_foreach_face(s, (GtsFunc)f_init_data, e); 
	
}

inline static gdouble eval_cluster_enery(int n, const GtsCVCEnergy *e) 
{
	return (e->angle[n] - gts_vector_scalar(e->wcentroid[n], e->wcentroid[n])) / e->area[n]; 
} 

static gdouble get_signed_angle(GtsClusterFace *f, GtsClusterFace *fi, gint cn)
{
	gint cf = gts_clusterface_get_clusternr(f); 
	gint cfi = gts_clusterface_get_clusternr(fi);
	
	if (cf == cfi && cf == cn)
		return gts_triangles_angle(GTS_TRIANGLE(f), GTS_TRIANGLE(fi));
	else
		return 0.0;
}

static double get_angle_old_old(GtsClusterFace *f, gint old_cn, gdouble area_f, GtsSurface *s)
{
	gdouble result = 0.0;
	GSList *n = gts_face_neighbors(GTS_FACE(f), s); 
	while (n) {
		GtsClusterFace *fi = GTS_CLUSTERFACE(n->data);
		result += get_signed_angle(f, fi, old_cn) * area_f * gts_clusterface_area(fi); 
		n = g_slist_next(n); 
	}	
	return result; 
}

static double get_angle_old_new(GtsClusterFace *f, gint new_cn, gdouble area_f, GtsSurface *s)
{
	gdouble result = 0.0;
	
	GSList *n = gts_face_neighbors(GTS_FACE(f), s); 
	while (n) {
		
		GtsClusterFace *fi = GTS_CLUSTERFACE(n->data);
		result -= get_signed_angle(f, fi, new_cn) * area_f * gts_clusterface_area(fi); 
		n = g_slist_next(n); 
	}	
	return result; 
}

static void add_face(GtsCVCEnergy *e, gdouble area, gdouble angle, GtsVector c, gint c_nr)
{
	e->area[c_nr] += area;
	e->angle[c_nr] += angle;
	e->wcentroid[c_nr][0] +=  c[0]; 
	e->wcentroid[c_nr][1] +=  c[1]; 
	e->wcentroid[c_nr][2] +=  c[2];
}

static void del_face(GtsCVCEnergy *e, gdouble area, gdouble angle, GtsVector c, gint c_nr)
{ 
	e->area[c_nr] -= area;
	e->angle[c_nr] -= angle;
	e->wcentroid[c_nr][0] -=  c[0]; 
	e->wcentroid[c_nr][1] -=  c[1]; 
	e->wcentroid[c_nr][2] -=  c[2];
}

static gboolean cvcenergy_move_face(GtsEnergy *energy, GtsClusterFace *f, gint old_cn, gint new_cn)
{
	g_assert(GTS_IS_CVCENERGY(energy));
	g_assert(GTS_IS_CLUSTERFACE(f));
	g_assert(new_cn >= 0); 
	
	gdouble energy_0; 
	gdouble energy_1;
	gdouble area;
	GtsVector c;
	GtsVector cfd, cfa;

	
	GtsCVCEnergy *e = GTS_CVCENERGY(energy); 

	area = gts_clusterface_wcentroid(f, c);	
	
	gdouble old_angles = get_angle_old_old(f, old_cn, area, e->surface); 
	gdouble new_angles = get_angle_old_new(f, new_cn, area, e->surface);
	
	
	// if the face is in the free cluster, allow transformation
	if (old_cn == -1) {
		add_face(e, area, new_angles, c, new_cn); 
		return TRUE;
	}
	
	gts_vector_add(cfa, e->wcentroid[new_cn], c); 
	gts_vector_sub(cfd, e->wcentroid[old_cn], c);
	
	// energy term before face move 
	energy_0 =  eval_cluster_enery(old_cn, e) + eval_cluster_enery(new_cn, e);
	
	energy_1 =
		(e->angle[old_cn] + gts_vector_scalar(cfd, cfd)) / (e->area[old_cn] - area) + 
		( e->angle[new_cn]  + gts_vector_scalar(cfa, cfa))/ (e->area[new_cn] + area); 
	
	if (energy_0 > energy_1) {
		add_face(e, area, new_angles, c, new_cn);
		del_face(e, area, old_angles, c, old_cn);
		return TRUE; 
	}
	return FALSE;
}

void gts_cvcenergy_test()    
{
	
#define  nVtx   10
#define  nEdge  19 
#define  nFace  10
	
	gdouble v[nVtx][3] = { { -20,  10, 0}, 
			       { -20,   0, 0}, 
			       { -20, -10, 0}, 
			       {  0,   10, 0}, 
			       {  0,    5, 0}, 
			       {  0,   -5, 0},
			       {  0,   -10, 0},
			       {  20,  10, 0}, 
			       {  20,   0, 0}, 
			       {  20, -10, 0},
			          
	}; 
	gint e[nEdge][2] = { 
		{ 0, 1}, { 1, 2}, { 0, 3}, { 0, 4}, { 1, 4}, 
		{ 1, 5}, { 2, 5}, { 2, 6}, { 3, 4}, { 4, 5}, 
		{ 5, 6}, { 3, 7}, { 4, 7}, { 4, 8}, { 5, 8}, 
		{ 5, 9}, { 6, 9}, { 7, 8}, { 8, 9}
		};
	
	gint f[nFace][4] = { 
		{ 0,  3,  4, 0}, 
		{ 1,  5,  6, 0}, 
		{ 2,  8,  3, 0}, 
		{ 4,  9,  5, 0}, 
		{ 7,  6, 10, 0},
		{ 8, 11, 12, 1}, 
		{ 9, 13, 14, 1}, 
		{10, 15, 16, 1}, 
		{13, 12, 17, 1}, 
		{14, 18, 15, 1}
	};

	const guint nclusters = 2; 
	
	GtsSurface *s = create_clusterface_surface(nVtx, v, nEdge, e, nFace, f); 
	
	
	GtsCVCEnergy *cvcenergy = gts_cvcenergy_new(gts_cvcenergy_class()); 
	g_assert(GTS_IS_CVCENERGY(cvcenergy));

	gts_energy_init_data(GTS_ENERGY(cvcenergy), s, nclusters);
	
	gts_cvcenergy_destroy(cvcenergy);	
}


