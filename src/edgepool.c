/***************************************************************************
 *            edgepool.h
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

#include <math.h>


#include "edgepool.h"

static void edge_pool_destroy (GtsObject * object)
{
	g_assert(GTS_IS_EDGE_POOL(object)); 
	g_hash_table_destroy(GTS_EDGE_POOL(object)->edges); 
	(* GTS_OBJECT_CLASS (gts_edge_pool_class ())->parent_class->destroy) (object);
}

static void edge_pool_class_init(GtsEdgePoolClass *klass)
{
	GTS_OBJECT_CLASS (klass)->destroy = edge_pool_destroy;
}


typedef struct {
	GtsVertex *v1; 
	GtsVertex *v2;
} VertexPair; 

static guint gts_vertex_pair_hash(VertexPair *s)
{
	return GPOINTER_TO_UINT(s->v1) + GPOINTER_TO_UINT(s->v2);  
}


static gboolean gts_vertex_pair_equal(VertexPair *s1, VertexPair *s2)
{
	return ((s1->v1 == s2->v1 && s1->v2 == s2->v2) ||
		(s1->v1 == s2->v2 && s1->v2 == s2->v1)) ? TRUE : FALSE; 
}


static void edge_pool_init(GtsEdgePool *pool)
{
	pool->edges = g_hash_table_new_full ((GHashFunc)gts_vertex_pair_hash, 
					     (GEqualFunc)gts_vertex_pair_equal, 
					     g_free, NULL);
}


GtsEdgePoolClass * gts_edge_pool_class(void)
{
	static GtsEdgePoolClass * klass = NULL;
	
	if (klass == NULL) {
		GtsObjectClassInfo edge_pool_info = {
			"GtsEdgePool",
			sizeof (GtsEdgePool),
			sizeof (GtsEdgePoolClass),
			(GtsObjectClassInitFunc) edge_pool_class_init,
			(GtsObjectInitFunc) edge_pool_init,
			(GtsArgSetFunc) NULL,
			(GtsArgGetFunc) NULL
		};
		klass = gts_object_class_new (gts_object_class (), 
					      &edge_pool_info);
	}
	
	return klass;
 
}
     
GtsEdgePool *    gts_edge_pool_new(GtsEdgePoolClass * klass)
{
	return GTS_EDGE_POOL (gts_object_new (GTS_OBJECT_CLASS (klass)));
}

GtsEdge *gts_edge_pool_new_edge(GtsEdgePool *pool, GtsEdgeClass *edge_class, GtsVertex *v1, GtsVertex *v2)
{
	
	g_assert(GTS_IS_EDGE_POOL(pool)); 
	
	VertexPair vp = {v1, v2}; 
	GtsEdge *edge = GTS_EDGE(g_hash_table_lookup(pool->edges, &vp)); 
	if (!edge) {
		VertexPair *vpp = g_new(VertexPair, 1); 
		vpp->v1 = v1; 
		vpp->v2 = v2; 
		edge = gts_edge_new(edge_class, v1, v2);
		g_hash_table_insert(pool->edges, vpp, edge); 
	}
	return edge; 
}

void gts_edge_pool_reset(GtsEdgePool *pool)
{
	g_assert(GTS_IS_EDGE_POOL(pool)); 
	g_hash_table_remove_all(pool->edges); 
	
}

void gts_edge_pool_test()
{
	GtsVertex *v1 = gts_vertex_new(gts_vertex_class(), 0.0, 0.1, 0.2); 
	GtsVertex *v2 = gts_vertex_new(gts_vertex_class(), 0.1, 0.2, 0.2); 
	GtsVertex *v3 = gts_vertex_new(gts_vertex_class(), 0.2, 0.3, 0.2);
	
	VertexPair vp1 = { v1, v2};
	VertexPair vp2 = { v2, v1};
	VertexPair vp3 = { v2, v3};

	g_assert(gts_vertex_pair_equal(&vp1, &vp2));  
	g_assert(!gts_vertex_pair_equal(&vp1, &vp3));
	
	GtsEdgePool *pool = gts_edge_pool_new(gts_edge_pool_class()); 
	g_assert(pool->edges); 
	
	g_assert(GTS_IS_EDGE_POOL(pool));
	
	GtsEdge *e1 = gts_edge_pool_new_edge(pool, gts_edge_class(), v1, v2); 
	GtsEdge *e2 = gts_edge_pool_new_edge(pool, gts_edge_class(), v2, v1);
	g_assert(e1 == e2); 
	GtsEdge *e3 = gts_edge_pool_new_edge(pool, gts_edge_class(), v2, v3);
	g_assert(e1 != e3);	
	
	gts_object_destroy(GTS_OBJECT(e1)); 
	gts_object_destroy(GTS_OBJECT(e3));
	

	gts_object_destroy(GTS_OBJECT(pool)); 
	g_message("EdgePool PASSED"); 
}
