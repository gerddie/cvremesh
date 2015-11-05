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


#ifndef GTS_TRIANGULATE_H
#define GTS_TRIANGULATE_H

#include "edgepool.h"
#include "clist.h"

gint gts_polygon_orientation(GCList *polygon, GtsVector retval); 
void gts_triangulate_convex_polygon(GtsSurface *s, GtsEdgePool *pool, GCList *p);
GCList *gts_surface_add_polygon(GtsSurface *s, GtsEdgePool *pool, GCList *polygon, GtsVector normal); 
void gts_polygon_triangulate_test(); 
void gts_triangulate_convex_polygon_test(); 




#endif
     
