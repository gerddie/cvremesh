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


/****************************************************************************
 *
 * This file implements the "ear-cutting" triangulation of flat polygons
 * it's complexity is O(n^2).
 *
 ****************************************************************************/

#include <gts.h>
#include <glib/gprintf.h>
#include <math.h>
#include <string.h>


#include "triangulate.h" 


gint gts_polygon_orientation(GCList *polygon, GtsVector retval) 
{
	GtsVector a,b; 
	
	retval[0] = retval[1] = retval[2] = 0.0; 
	
	if (polygon->next == polygon->prev) /* only two points */ 
		return 0;
		
	gts_vector_init(a, GTS_POINT(polygon->data), GTS_POINT(polygon->prev->data)); 
	gts_vector_init(b, GTS_POINT(polygon->data), GTS_POINT(polygon->next->data)); 
	
	gts_vector_cross(retval, a, b);
	gts_vector_normalize(retval); 
	
	return 1; 
}

static gboolean is_convex(GtsPoint *p0, GtsPoint *p1, GtsPoint *p2, GtsVector orientation)
{
	g_assert(GTS_IS_POINT(p0)); 
	g_assert(GTS_IS_POINT(p1)); 
	g_assert(GTS_IS_POINT(p2)); 
	
	GtsVector a; 
	GtsVector b;
	GtsVector c; 
	
	gts_vector_init(a, p1, p0); 
	gts_vector_init(b, p1, p2); 
	
	gts_vector_cross(c, a, b);
	
	return gts_vector_scalar(c, orientation) >= 0; 
	
}

static gdouble get_area(GtsPoint *v1, GtsPoint *v2, GtsPoint *v3)
{
	GtsVector l1, l2, c; 
	
	gts_vector_init(l1, v2, v1); 
	gts_vector_init(l2, v2, v3);
	gts_vector_cross(c, l1, l2);
	
	return gts_vector_norm(c); 
}

static gint is_inside(GtsPoint *v1, GtsPoint *v2, GtsPoint *v3, GtsPoint *p)
{
	
	gdouble a_all, a1, a2, a3;
	
	a_all = get_area(v1, v2, v3); 
	a1 = get_area(p, v2, v3);
	a2 = get_area(v1, p, v3);
	a3 = get_area(v1, v2, p);
	
	return (a_all >= a1 + a2 + a3);
}

static void  add_face_from_polygon_corner(GtsSurface *s, GtsEdgePool *pool, 
					  GtsVertex *v1, GtsVertex *v2, GtsVertex *v3)
{
	GtsEdge *e1 = gts_edge_pool_new_edge(pool, s->edge_class, v1, v2); 
	GtsEdge *e2 = gts_edge_pool_new_edge(pool, s->edge_class, v2, v3); 
	GtsEdge *e3 = gts_edge_pool_new_edge(pool, s->edge_class, v3, v1); 
	GtsFace *f = gts_face_new(s->face_class, e1, e2, e3); 
	gts_surface_add_face(s,f); 
}

static gboolean add_ear(GtsSurface *s, GtsEdgePool *pool, GCList *p, GCList *polygon, GtsVector orientation)
{
	GtsPoint *v1, *v2, *v3;  
	GCList *i; 
	// ears are convex
	GCList *pp = p->prev;
	GCList *pn = p->next;
	
	v1 = GTS_POINT(pp->data);
	v2 = GTS_POINT(p->data);
	v3 = GTS_POINT(pn->data);
	GTS_IS_POINT(v1); 
	GTS_IS_POINT(v2); 
	GTS_IS_POINT(v3);

	if ( !is_convex(v1, v2, v3, orientation) ) {
		g_debug("concave face (%6.2f %6.2f %6.2f) (%6.2f %6.2f %6.2f) (%6.2f %6.2f %6.2f)",
			v1->x, v1->y, v1->z, v2->x, v2->y, v2->z, v3->x, v3->y, v3->z); 
		return FALSE;
	}
	
	i = polygon; 
	
	do {
		GtsPoint *p1 = GTS_POINT(i->data);
		if (p1 != v1 && p1 != v2 && p1 != v3) {
			GtsPoint *p0 = GTS_POINT(i->prev->data);
			GtsPoint *p2 = GTS_POINT(i->next->data);
			
			if (!is_convex(p0, p1, p2, orientation) && is_inside(v1, v2, v3, p1)) {
				g_debug("reject face\n"); 
				return FALSE;
			}
		}
		i = g_clist_next(i); 
	} while (i != polygon); 

	add_face_from_polygon_corner(s, pool, GTS_VERTEX(v1), GTS_VERTEX(v2), GTS_VERTEX(v3)); 
	return TRUE; 
}

GCList *gts_surface_add_polygon(GtsSurface *s, GtsEdgePool *pool, GCList *polygon, GtsVector orientation) 
{
	GtsVertex *v1, *v2, *v3; 
	g_assert(GTS_IS_SURFACE(s));
	g_assert(GTS_IS_EDGE_POOL(pool)); 

	guint loop = 0; 
	gint poly_size = g_clist_length(polygon);
	g_debug("triangulate a  polygon with %d vertices", poly_size);
	
	// no triangles -> no 
	if (poly_size < 3) 
		return polygon;
	

	if (gts_vector_norm(orientation) == 0.0)
		gts_polygon_orientation(polygon, orientation); 
		
	
	while (poly_size > 3) {
		if (loop > 2) {// that is essentially rape
			polygon = g_clist_reverse(polygon); 
			g_warning("reversed the polygon!"); 
		}
			
		GCList *p_i = g_clist_next(polygon);
		while (p_i != polygon && poly_size > 3) {
			if (add_ear(s, pool, p_i, polygon, orientation)) {
				GCList *p_i_old = p_i;
				p_i = p_i->prev != polygon ? p_i->prev : p_i->next;
				polygon = g_clist_delete_link(polygon, p_i_old);
				--poly_size;
			}else
				p_i = g_clist_next(p_i); 
	
		}
		++loop; 
	}
	v1 = GTS_VERTEX(polygon->prev->data); 
	v2 = GTS_VERTEX(polygon->data); 
	v3 = GTS_VERTEX(polygon->next->data); 
	
	add_face_from_polygon_corner(s, pool, v1, v2, v3);
	return polygon; 
}




void gts_triangulate_convex_polygon(GtsSurface *s, GtsEdgePool *pool, GCList *p)
{
	guint poly_length = g_clist_length(p);
	
	while (poly_length > 2) {
		while (poly_length > 3) {
			
			GtsVector e1, e2; 
			GtsPoint *v1 = GTS_POINT(p->prev->data);
			GtsPoint *v2 = GTS_POINT(p->data);
			GtsPoint *v3 = GTS_POINT(p->next->data);
			GtsPoint *v4 = GTS_POINT(p->next->next->data);
			
			if (v1 == v3 || v2 == v4) {
				g_debug("kill degenerated triangle"); 
				GCList *p1 = p;
				GCList *p2 = p->next;
				p=g_clist_delete_link(p,p1);
				p=g_clist_delete_link(p,p2);
				poly_length -= 2; 
				continue; 
			}
			
			gts_vector_init(e1, v1, v3);
			gts_vector_init(e2, v2, v4);
			
			if (gts_vector_scalar(e1, e1) < gts_vector_scalar(e2, e2)) {
				add_face_from_polygon_corner(s, pool, 
							     GTS_VERTEX(v1), 
							     GTS_VERTEX(v2),
							     GTS_VERTEX(v3)); 
				p = g_clist_delete_link(p, p);
			} else {
				add_face_from_polygon_corner(s, pool, 
							     GTS_VERTEX(v2), 
							     GTS_VERTEX(v3),
							     GTS_VERTEX(v4)); 
				p = g_clist_delete_link(p, p->next);
			}
			--poly_length; 
			
		}
		if (g_clist_length(p) > 2) {
			add_face_from_polygon_corner(s, pool, 
						     GTS_VERTEX(p->prev->data), 
						     GTS_VERTEX(p->data),
						     GTS_VERTEX(p->next->data));
			p = g_clist_delete_link(p, p);
			--poly_length; 
		}
		
	}
	g_clist_free(p);
}


static void gts_triangulate_test_stuff()
{
	gdouble v[4][3] = {
		{0.0, 0.0, 0.0},
		{0.0, 3.0, 0.0},
		{3.0, 3.0, 0.0},
		{1.0, 1.0, 0.0}
	};
	
	int i; 
	GtsVertex *vtx[4];
	GCList *polygon = NULL;
	GtsVector orient; 
	GCList *p; 
	
	for (i = 0; i < 4; ++i) {
		vtx[i] = gts_vertex_new(gts_vertex_class(), v[i][0],v[i][1],v[i][2]); 
		polygon = g_clist_append(polygon, vtx[i]); 
	}
	g_assert(g_clist_length(polygon));
	g_assert(gts_polygon_orientation(polygon, orient));	

	g_assert(orient[0] == 0 && orient[1] == 0 && orient[2] < 0); 

	p = g_clist_next(polygon); 
	g_assert(is_convex(GTS_POINT(p->prev->data), GTS_POINT(p->data),GTS_POINT(p->next->data), orient));
	g_assert(is_inside(GTS_POINT(vtx[0]), GTS_POINT(vtx[1]), GTS_POINT(vtx[2]), GTS_POINT(vtx[3])));
	g_assert(!is_inside(GTS_POINT(vtx[0]), GTS_POINT(vtx[1]), GTS_POINT(vtx[3]), GTS_POINT(vtx[2])));

	g_clist_free(p); 
	for (i = 0; i < 4; ++i) 
		gts_object_destroy(GTS_OBJECT(vtx[i])); 
		
}

void gts_polygon_triangulate_test() 
{
	int i; 
	gdouble v[12][3] = {
		{0.0, 0.0, 0.0},
		{0.0, 3.0, 0.0},
		{3.0, 3.0, 0.0},
		{3.0, 1.0, 0.0},
		{4.0, 1.0, 0.0},
		{4.0, 3.0, 0.0},
		{5.0, 3.0, 0.0},
		{5.0, 0.0, 0.0},
		{2.0, 0.0, 0.0},
		{2.0, 2.0, 0.0},
		{1.0, 2.0, 0.0},
		{1.0, 0.0, 0.0},
	};

	GtsVector normal = {0,0,0}; 
	GtsVertex *vtx[12]; 
	GCList *polygon = NULL;
	GCList *triangle = NULL;
	GtsSurface *s, *s2; 
	
	GtsEdgePool *pool = gts_edge_pool_new(gts_edge_pool_class());
	
	gts_triangulate_test_stuff(); 
		
	for (i = 0; i < 12; ++i) {
		vtx[i] = gts_vertex_new(gts_vertex_class(), v[i][0],v[i][1],v[i][2]); 
		polygon = g_clist_append(polygon, vtx[i]);
	}

	for (i = 0; i < 3; ++i) {
		triangle = g_clist_prepend(triangle, vtx[i]); 
	}
	
	
	s = gts_surface_new(gts_surface_class(), 
			    gts_face_class(), 
			    gts_edge_class(),
			    gts_vertex_class());
	
	s2 = gts_surface_new(gts_surface_class(), 
			     gts_face_class(), 
			     gts_edge_class(),
			     gts_vertex_class());
	
	
	
	// triangulate the polygon by using its own orientation 
	polygon = gts_surface_add_polygon(s, pool, polygon, normal);
	// do some test ?
	g_assert(gts_surface_face_number(s) == 10); 
	g_debug("area  = %f", gts_surface_area(s)); 
	g_assert(gts_surface_area(s) == 11.0);
	
	
	
	gts_object_destroy(GTS_OBJECT(pool)); 
	// cleanup

	g_clist_free(triangle); 
	g_clist_free(polygon); 
	gts_object_destroy(GTS_OBJECT(s));
	gts_object_destroy(GTS_OBJECT(s2));
	g_message("Triangulate PASSED"); 
}

void gts_triangulate_convex_polygon_test()
{
	int i; 
	gdouble v[5][3] = {
		{0.0, 0.0, 1.0},
		{0.0, 2.0, 2.0},
		{2.0, 3.0, 1.0},
		{4.0, 1.0, 1.0},
		{2.0, 0.0, 0.0}
	};
	
	GtsEdgePool *pool = gts_edge_pool_new(gts_edge_pool_class());
	GtsVertex *vtx[5]; 
	GCList *polygon = NULL;
	GCList *wired = NULL; 
	GtsSurfaceStats sstats; 
	
	for (i = 0; i < 5; ++i) {
		vtx[i] = gts_vertex_new(gts_vertex_class(), v[i][0],v[i][1],v[i][2]); 
		polygon = g_clist_append(polygon, vtx[i]);
	}
	polygon = g_clist_append(polygon, vtx[0]);
	polygon = g_clist_append(polygon, vtx[1]);
	
	wired = g_clist_append(wired, vtx[0]);
	wired = g_clist_append(wired, vtx[1]);
	wired = g_clist_append(wired, vtx[0]);
	wired = g_clist_append(wired, vtx[2]);
	
	
		
	GtsSurface *s = gts_surface_new(gts_surface_class(), 
					gts_face_class(), 
					gts_edge_class(),
					gts_vertex_class());
	
	gts_triangulate_convex_polygon(s, pool, wired);
	g_assert(gts_surface_face_number(s) == 0);

	gts_triangulate_convex_polygon(s, pool, polygon);  

	g_message("gts_surface_face_number = %d", gts_surface_face_number(s));  
	g_assert(gts_surface_face_number(s) == 3);
	gts_surface_stats(s, &sstats);
	
	g_assert(sstats.n_incompatible_faces == 0); 
	g_assert(sstats.n_duplicate_faces == 0);	
	g_assert(sstats.n_duplicate_edges == 0);	
	g_assert(sstats.n_boundary_edges == 5);	
	g_assert(sstats.n_non_manifold_edges == 0);	
	g_assert(sstats.faces_per_edge.min == 1); 
	g_assert(sstats.faces_per_edge.max == 2);	
	
	g_message("Triangulate convex polygon PASS"); 

	gts_object_destroy(GTS_OBJECT(pool)); 
}


