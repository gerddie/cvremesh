/***************************************************************************
 *            helper.h
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


#ifndef CVTEST_HELPER_H
#define CVTEST_HELPER_H

#ifdef __cplusplus
extern "C" {
#endif	
#include <gts.h>
	
GtsSurface *create_std_surface();
gboolean gts_vector_equal(GtsVector a, GtsVector b);
GtsSurface *create_clusterface_surface(guint nvtx, double vtx[][3], guint nedge, int edge[][2], guint nface, int face[][4]); 

#define gts_vector_add(c, a, b) \
c[0] = a[0] + b[0]; \
c[1] = a[1] + b[1]; \
c[2] = a[2] + b[2]; 


#define gts_vector_sub(c, a, b) \
c[0] = a[0] - b[0]; \
c[1] = a[1] - b[1]; \
c[2] = a[2] - b[2]; 
	

typedef struct {
	gint nclusters;
	GtsSurface **clusters; 
} SurfaceClusterParams;

void gts_surface_get_clusters(GtsSurface *s, SurfaceClusterParams *scp); 
	
	
#ifdef __cplusplus
}
#endif 

#endif
