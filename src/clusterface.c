/***************************************************************************
 *            clusterface.c
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

#include <assert.h>
#include <string.h>

#include <glib/gprintf.h>
#include "clusterface.h"

static void clusterface_destroy (GtsObject * object)
{
	(* GTS_OBJECT_CLASS (gts_clusterface_class ())->parent_class->destroy) (object);
}

static void clusterface_clone (GtsObject * clone, GtsObject * object)
{
	(* GTS_OBJECT_CLASS (gts_clusterface_class ())->parent_class->clone) (clone, object);
	GTS_CLUSTERFACE (clone)->cluster = GTS_CLUSTERFACE (object)->cluster;
}

static void clusterface_class_init (GtsClusterFaceClass * klass)
{
	GTS_OBJECT_CLASS (klass)->clone = clusterface_clone;
	GTS_OBJECT_CLASS (klass)->destroy = clusterface_destroy;
}

static void clusterface_init (GtsClusterFace * face)
{
	face->cluster = -1;
	face->centroid_valid = 0; 
}

/**
 * gts_clusterface_class:
 *
 * Returns: the #GtsClusterFaceClass.
 */
GtsClusterFaceClass * gts_clusterface_class (void)
{
	static GtsClusterFaceClass * klass = NULL;
	
	if (klass == NULL) {
		GtsObjectClassInfo clusterface_info = {
			"GtsClusterFace",
			sizeof (GtsClusterFace),
			sizeof (GtsClusterFaceClass),
			(GtsObjectClassInitFunc) clusterface_class_init,
			(GtsObjectInitFunc) clusterface_init,
			(GtsArgSetFunc) NULL,
			(GtsArgGetFunc) NULL
		};
		klass = gts_object_class_new (GTS_OBJECT_CLASS (gts_face_class ()), 
					      &clusterface_info);
	}
	
	return klass;
}

/**
 * gts_clusterface_new:
 * @klass: a #GtsClusterFaceClass.
 * @e1: a #GtsEdge.
 * @e2: a #GtsEdge.
 * @e3: a #GtsEdge.
 * @c: the cluster number 
 *
 * Returns: a new #GtsClusterFace using @e1, @e2 and @e3 as edges.
 */
GtsClusterFace * gts_clusterface_new (GtsClusterFaceClass * klass,
			GtsEdge * e1, GtsEdge * e2, GtsEdge * e3, gint c)
{
	GtsClusterFace * f;
	
	f = GTS_CLUSTERFACE (gts_face_new (GTS_FACE_CLASS(klass), e1, e2, e3));
	f->cluster = c; 
	f->centroid_valid = 0; 
	return f;
}

gint gts_clusterface_get_clusternr(GtsClusterFace *f)
{
	g_assert(GTS_IS_CLUSTERFACE(f)); 
	return f->cluster; 
}

void gts_clusterface_set_clusternr(GtsClusterFace *f, gint nr)
{
	g_assert(GTS_IS_CLUSTERFACE(f));
	f->cluster = nr; 
}

gint gts_clusterface_get_generation(GtsClusterFace *f)
{
	g_assert(GTS_IS_CLUSTERFACE(f)); 
	return f->generation;
}

void gts_clusterface_set_generation(GtsClusterFace *f, gint nr)
{
	g_assert(GTS_IS_CLUSTERFACE(f)); 
	f->generation = nr;
}



struct _NCNData {
	GSList *result; 
	gint cluster; 
}; 
typedef struct _NCNData NCNData;

static gint f_get_ncn(GtsClusterFace *face, NCNData *ncn)
{
	if (face->cluster != ncn->cluster) {
		ncn->result = g_slist_prepend(ncn->result, face);
	}
	return 0; 
}

GSList *gts_clusterface_get_noncluster_neighbours(GtsClusterFace *f, GtsSurface *s) 
{
	NCNData data = {0, f->cluster};
	gts_face_foreach_neighbor(GTS_FACE(f), s, (GtsFunc)f_get_ncn, &data);
	return data.result;
}

static void clusterface_eval_wcentroid_and_area(GtsClusterFace *f)
{
	GtsTriangle *t = GTS_TRIANGLE(f); 
	GtsVertex *v1=0, *v2=0, *v3=0;
	gts_triangle_vertices(t, &v1, &v2, &v3); 
	GtsPoint *p1 = GTS_POINT(v1); 
	GtsPoint *p2 = GTS_POINT(v2); 
	GtsPoint *p3 = GTS_POINT(v3); 
	assert(p1 && p2 && p3); 
	
	f->area =  gts_triangle_area(t); 
	
	f->centroid[0] = f->area * (p1->x + p2->x + p3->x) / 3.0; 
	f->centroid[1] = f->area * (p1->y + p2->y + p3->y) / 3.0; 
	f->centroid[2] = f->area * (p1->z + p2->z + p3->z) / 3.0;
	
	f->area =  gts_triangle_area(t); 
	f->centroid_valid = 1;
}

gdouble gts_clusterface_area(GtsClusterFace *f)
{
	assert(GTS_IS_CLUSTERFACE(f)); 
	if (!f->centroid_valid)
		clusterface_eval_wcentroid_and_area(f); 
	return f->area;
}

gdouble gts_clusterface_wcentroid(GtsClusterFace *f, GtsVector c)
{
	assert(GTS_IS_CLUSTERFACE(f)); 
	if (!f->centroid_valid)
		clusterface_eval_wcentroid_and_area(f); 
	
	memcpy(c, f->centroid, 3 * sizeof(gdouble)); 
	return f->area; 
}

void gts_clusterface_test()
{
	const gdouble one_sixth = 1.0/6.0; 

	GtsVector v; 
	GtsVertex *v1 = gts_vertex_new(gts_vertex_class(), 0.0, 0.0, 2.0); 
	GtsVertex *v2 = gts_vertex_new(gts_vertex_class(), 0.0, 1.0, 1.0); 
	GtsVertex *v3 = gts_vertex_new(gts_vertex_class(), 0.0, 0.0, 3.0); 
	
	GtsEdge *e1 = gts_edge_new(gts_edge_class(), v1, v2); 
	GtsEdge *e2 = gts_edge_new(gts_edge_class(), v2, v3); 
	GtsEdge *e3 = gts_edge_new(gts_edge_class(), v3, v1); 
	
	GtsClusterFace *f = gts_clusterface_new(gts_clusterface_class(), e1, e2, e3, 1);
	assert(GTS_IS_CLUSTERFACE(f)); 
	assert(GTS_IS_FACE(f));
	
	assert(gts_clusterface_wcentroid(f, v) == 0.5);
	assert(v[0] == 0.0 && v[1] == one_sixth && v[2] == 1.0); 
	
	gts_object_destroy(GTS_OBJECT(f)); 
	
}

