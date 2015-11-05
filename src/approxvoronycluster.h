/***************************************************************************
 *            run_tests.c
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

#ifndef APPROXVORONYCLUSTER_H
#define APPROXVORONYCLUSTER_H

#include <gts.h>
#include <energy.h>

typedef void (*DisplayCallback)(GtsSurface *s, gpointer userdata); 

gdouble gts_surface_center_of_cluster_area(GtsSurface *s, guint nr, GtsVector center); 

GtsSurface *gts_surface_cluster_copy(GtsSurface *s, guint n, GtsEnergy *energy, DisplayCallback display, gpointer userdata);

void  gts_surface_cluster(GtsSurface *s, guint n, GtsEnergy *energy, gboolean init, DisplayCallback display, gpointer userdata); 

void 	gts_approx_vorony_clustering_test(); 

#endif
