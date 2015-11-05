/***************************************************************************
 *            csurface.c
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

#include <csurface.h>
#include <helper.h>


static void csurface_add_face(GtsSurface *, GtsFace *);
static void csurface_remove_face(GtsSurface *, GtsFace *);

static void csurface_destroy(GtsObject *object)
{
	g_assert(GTS_IS_CSURFACE(object));
	(* GTS_OBJECT_CLASS (gts_csurface_class ())->parent_class->destroy) (object);
}

static void csurface_class_init(GtsCSurfaceClass *klass)
{
	GTS_OBJECT_CLASS (klass)->destroy = csurface_destroy;
	GTS_SURFACE_CLASS(klass)->add_face = csurface_add_face; 
	GTS_SURFACE_CLASS(klass)->remove_face = csurface_remove_face;
}

static void csurface_init(GtsCSurface *csurface)
{
	csurface->n_clusters = 0; 
	csurface->free_faces = 0; 
}

GtsCSurfaceClass * gts_csurface_class(void)
{
	static GtsCSurfaceClass * klass = NULL;
	
	if (klass == NULL) {
		GtsObjectClassInfo csurface_info = {
			"GtsCSurface",
			sizeof (GtsCSurface),
			sizeof (GtsCSurfaceClass),
			(GtsObjectClassInitFunc) csurface_class_init,
			(GtsObjectInitFunc) csurface_init,
			(GtsArgSetFunc) NULL,
			(GtsArgGetFunc) NULL
		};
		klass = gts_object_class_new (GTS_OBJECT_CLASS(gts_surface_class ()), 
					      &csurface_info);
	}
	return klass;
}

GtsCSurface *gts_csurface_new(GtsCSurfaceClass *klass, GtsClusterFaceClass *fclass, 
			      GtsEdgeClass *eklass, GtsVertexClass *vklass)
{
	return GTS_CSURFACE (gts_surface_new (GTS_SURFACE_CLASS(klass), GTS_FACE_CLASS(fclass), eklass, vklass));
}
	
static void csurface_add_face(GtsSurface *s, GtsFace *f)
{
	g_assert(GTS_IS_CLUSTERFACE(f)); 
	g_assert(GTS_IS_CSURFACE(s));
	gint cn = gts_clusterface_get_clusternr(GTS_CLUSTERFACE(f));  
	if ( cn < 0 ) {
		GTS_CSURFACE(s)->free_faces++;
	} else if (! (GTS_CSURFACE(s)->n_clusters > cn)) {
		GTS_CSURFACE(s)->n_clusters = cn + 1; 
	}
}

static void csurface_remove_face(GtsSurface *s, GtsFace *f)
{
	g_assert(GTS_IS_CLUSTERFACE(f)); 
	g_assert(GTS_IS_CSURFACE(s));
	gint cn = gts_clusterface_get_clusternr(GTS_CLUSTERFACE(f));  
	if ( cn < 0 )
		GTS_CSURFACE(s)->free_faces--;
}
	
void gts_csurface_test()    
{
	const guint nclusters = 2;
	
	GtsCSurface *csurface = gts_csurface_new(gts_csurface_class(), gts_clusterface_class(), 
						 gts_edge_class(), gts_vertex_class()); 
	g_assert(GTS_IS_CSURFACE(csurface));
	
	
	gts_csurface_destroy(csurface);	
}

