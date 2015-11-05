/***************************************************************************
 *            csurface.h
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


#ifndef CV_CSURFACE_H
#define CV_CSURFACE_H

#include <gts.h>
#include <clusterface.h>

typedef struct _GtsCSurface          GtsCSurface;
typedef struct _GtsCSurfaceClass     GtsCSurfaceClass;


#define GTS_IS_CSURFACE(obj) (gts_object_is_from_class (obj,\
						     gts_csurface_class ()))
#define GTS_CSURFACE(obj)              GTS_OBJECT_CAST (obj,\
						     GtsCSurface,\
						     gts_csurface_class ())
#define GTS_CSURFACE_CLASS(klass)      GTS_OBJECT_CLASS_CAST (klass,\
							   GtsCSurfaceClass,\
							   gts_csurface_class ())

struct _GtsCSurface {
	GtsSurface object;
	gint n_clusters; 
	gint free_faces; 
};

struct _GtsCSurfaceClass {
	GtsSurfaceClass parent_class;
};

GtsCSurfaceClass * gts_csurface_class(void);

GtsCSurface *gts_csurface_new(GtsCSurfaceClass *klass, GtsClusterFaceClass *fclass, 
			      GtsEdgeClass *eklass, GtsVertexClass *vklass);

GtsVector *gts_csurface_get_cluster_normals(GtsCSurface *); 

#define gts_csurface_destroy(surface) gts_object_destroy(GTS_OBJECT(surface))



void gts_csurface_test(); 

#endif
