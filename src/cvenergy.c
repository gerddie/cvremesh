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

#include <cvenergy.h>
#include <helper.h>

static void cvenergy_destroy(GtsObject *object)
{
	g_assert(GTS_IS_CVENERGY(object));
	g_free(GTS_CVENERGY(object)->area); 
	g_free(GTS_CVENERGY(object)->wcentroid);
	(* GTS_OBJECT_CLASS (gts_cvenergy_class ())->parent_class->destroy) (object);
}

static gboolean cvenergy_move_face(GtsEnergy *energy, GtsClusterFace *f, gint old_cn, gint new_cn); 
static void cvenergy_init_data(GtsEnergy *energy, GtsSurface *s, guint nclusters); 

static void cvenergy_class_init(GtsCVEnergyClass *klass)
{
	GTS_OBJECT_CLASS (klass)->destroy = cvenergy_destroy;
	GTS_ENERGY_CLASS(klass)->move_face = cvenergy_move_face;  
	GTS_ENERGY_CLASS(klass)->init_data = cvenergy_init_data; 
}

static void cvenergy_init(GtsCVEnergy *cvenergy)
{
	cvenergy->area = NULL;  
	cvenergy->wcentroid = NULL;
	cvenergy->n_clusters = 0; 
}

GtsCVEnergyClass * gts_cvenergy_class(void)
{
	static GtsCVEnergyClass * klass = NULL;
	
	if (klass == NULL) {
		GtsObjectClassInfo cvenergy_info = {
			"GtsCVEnergy",
			sizeof (GtsCVEnergy),
			sizeof (GtsCVEnergyClass),
			(GtsObjectClassInitFunc) cvenergy_class_init,
			(GtsObjectInitFunc) cvenergy_init,
			(GtsArgSetFunc) NULL,
			(GtsArgGetFunc) NULL
		};
		klass = gts_object_class_new (GTS_OBJECT_CLASS(gts_energy_class ()), 
					      &cvenergy_info);
	}
	
	return klass;

}

GtsCVEnergy *    gts_cvenergy_new(GtsCVEnergyClass *klass)
{
	return GTS_CVENERGY (gts_energy_new (GTS_ENERGY_CLASS (klass)));
}

static guint f_init_data(GtsClusterFace *f, GtsCVEnergy *energy) 
{
	g_assert(GTS_IS_CLUSTERFACE(f)); 
	gint cn = gts_clusterface_get_clusternr(f); 
	
	if (cn < 0) 
		return 0; 
	if (cn < energy->n_clusters) {
		GtsVector c; 
		gdouble area = gts_clusterface_wcentroid(f, c); 
		energy->area[cn] += area; 
		energy->wcentroid[cn][0] += c[0];
		energy->wcentroid[cn][1] += c[1];
		energy->wcentroid[cn][2] += c[2];
	}else
		g_warning("Got clusternr %d not less then %d", cn, energy->n_clusters); 
	
	return 0;
}

static void cvenergy_init_data(GtsEnergy *energy, GtsSurface *s, guint nclusters)
{
	g_assert(GTS_IS_CVENERGY(energy)); 
	GtsCVEnergy *e = GTS_CVENERGY(energy); 
	
	e->n_clusters = nclusters; 
	e->area = g_new0(gdouble, nclusters); 
	e->wcentroid = g_new0(GtsVector, nclusters); 
	
	gts_surface_foreach_face(s, (GtsFunc)f_init_data, e); 
	
}

inline static gdouble eval_cluster_enery(int n, const GtsCVEnergy *e) 
{
	return gts_vector_scalar(e->wcentroid[n], e->wcentroid[n]) / e->area[n]; 
} 


static void add_face(GtsCVEnergy *e, gdouble area, GtsVector c, gint c_nr)
{
	e->area[c_nr] += area;
	e->wcentroid[c_nr][0] +=  c[0]; 
	e->wcentroid[c_nr][1] +=  c[1]; 
	e->wcentroid[c_nr][2] +=  c[2];
}

static void del_face(GtsCVEnergy *e, gdouble area, GtsVector c, gint c_nr)
{ 
	e->area[c_nr] -= area;
	e->wcentroid[c_nr][0] -=  c[0]; 
	e->wcentroid[c_nr][1] -=  c[1]; 
	e->wcentroid[c_nr][2] -=  c[2];
}

static gboolean cvenergy_move_face(GtsEnergy *energy, GtsClusterFace *f, gint old_cn, gint new_cn)
{
	g_assert(GTS_IS_CVENERGY(energy));
	g_assert(GTS_IS_CLUSTERFACE(f));
	g_assert(new_cn >= 0); 
	
	gdouble energy_0; 
	gdouble energy_1;
	gdouble area; 
	
	GtsCVEnergy *e = GTS_CVENERGY(energy); 
	
	
	GtsVector c;
	GtsVector cfd, cfa;
	
	
	area = gts_clusterface_wcentroid(f, c);
	
	// if the face is in the free cluster, allow transformation
	if (old_cn == -1) {
		add_face(e, area, c, new_cn); 
		return TRUE;
	}
	
	gts_vector_add(cfa, e->wcentroid[new_cn], c); 
	gts_vector_sub(cfd, e->wcentroid[old_cn], c);
	
	// energy term before face move 
	energy_0 = - eval_cluster_enery(old_cn, e) - eval_cluster_enery(new_cn, e);
	
	energy_1 = -gts_vector_scalar(cfd, cfd)/ (e->area[old_cn] - area) - 
		gts_vector_scalar(cfa, cfa)/ (e->area[new_cn] + area); 
	
	if (energy_0 > energy_1) {
		add_face(e, area, c, new_cn);
		del_face(e, area, c, old_cn);
		return TRUE; 
	}
	return FALSE;
}

void gts_cvenergy_test()    
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
	
	
	GtsCVEnergy *cvenergy = gts_cvenergy_new(gts_cvenergy_class()); 
	g_assert(GTS_IS_CVENERGY(cvenergy));

	gts_energy_init_data(GTS_ENERGY(cvenergy), s, nclusters);
	
	gts_cvenergy_destroy(cvenergy);	
}


