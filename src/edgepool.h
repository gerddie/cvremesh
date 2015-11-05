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


#ifndef GTS_EDGEPOOL_H
#define GTS_EDGEPOOL_H

#include <gts.h>

typedef struct _GtsEdgePool          GtsEdgePool;
typedef struct _GtsEdgePoolClass     GtsEdgePoolClass;


#define GTS_IS_EDGE_POOL(obj) (gts_object_is_from_class (obj,\
						     gts_edge_pool_class ()))
#define GTS_EDGE_POOL(obj)              GTS_OBJECT_CAST (obj,\
						     GtsEdgePool,\
						     gts_edge_pool_class ())
#define GTS_EDGE_POOL_CLASS(klass)      GTS_OBJECT_CLASS_CAST (klass,\
							   GtsEdgePoolClass,\
							   gts_edge_pool_class ())

struct _GtsEdgePool {
	GtsObject object;
	GHashTable *edges; 

};

struct _GtsEdgePoolClass {
	GtsObjectClass parent_class;
};

GtsEdgePoolClass * gts_edge_pool_class(void);
GtsEdgePool *    gts_edge_pool_new(GtsEdgePoolClass * klass);
GtsEdge *gts_edge_pool_new_edge(GtsEdgePool *pool, GtsEdgeClass *edge_class, GtsVertex *v1, GtsVertex *v2); 
void gts_edge_pool_reset(GtsEdgePool *pool); 


void gts_edge_pool_test(); 

#endif
