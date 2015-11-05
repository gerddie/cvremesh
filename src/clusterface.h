/***************************************************************************
 *            clusterface.h
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


#ifndef GTS_CLUSTERFACE_H
#define GTS_CLUSTERFACE_H

#include <gts.h>


typedef struct _GtsClusterFace          GtsClusterFace;
typedef struct _GtsClusterFaceClass     GtsClusterFaceClass;



/* Faces: face.c */

#define GTS_IS_CLUSTERFACE(obj) (gts_object_is_from_class (obj,\
						    gts_clusterface_class ()))
#define GTS_CLUSTERFACE(obj)          GTS_OBJECT_CAST (obj,\
						GtsClusterFace,\
						gts_clusterface_class ())
#define GTS_CLUSTERFACE_CLASS(klass)  GTS_OBJECT_CLASS_CAST (klass,\
						      GtsClusterFaceClass,\
						      gts_clusterface_class ())

struct _GtsClusterFace {
	GtsFace face;
	gint cluster; 
	gint generation; 
	gint centroid_valid; 
	gdouble area; 
	GtsVector centroid; 
	gint reserved; 
};

struct _GtsClusterFaceClass {
  GtsTriangleClass parent_class;
};


GtsClusterFaceClass * gts_clusterface_class                       (void);
GtsClusterFace *     gts_clusterface_new(GtsClusterFaceClass * klass,
					 GtsEdge * e1,
					 GtsEdge * e2,
					 GtsEdge * e3, gint c);

gint gts_clusterface_get_clusternr(GtsClusterFace *f);

void gts_clusterface_set_clusternr(GtsClusterFace *f, gint nr);

gint gts_clusterface_get_generation(GtsClusterFace *f);

void gts_clusterface_set_generation(GtsClusterFace *f, gint nr);


GSList *gts_clusterface_get_noncluster_neighbours(GtsClusterFace *f, GtsSurface *s);

gdouble gts_clusterface_area(GtsClusterFace *f); 
     
gdouble gts_clusterface_wcentroid(GtsClusterFace *f, GtsVector c);

void gts_clusterface_test(); 
     


#endif
