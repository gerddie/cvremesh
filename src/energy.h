/***************************************************************************
 *            energy.h
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


#ifndef CV_ENERGY_H
#define CV_ENERGY_H

#include <gts.h>
#include <clusterface.h>

typedef struct _GtsEnergy          GtsEnergy;
typedef struct _GtsEnergyClass     GtsEnergyClass;


#define GTS_IS_ENERGY(obj) (gts_object_is_from_class (obj,\
						     gts_energy_class ()))
#define GTS_ENERGY(obj)              GTS_OBJECT_CAST (obj,\
						     GtsEnergy,\
						     gts_energy_class ())
#define GTS_ENERGY_CLASS(klass)      GTS_OBJECT_CLASS_CAST (klass,\
							   GtsEnergyClass,\
							   gts_energy_class ())

struct _GtsEnergy {
	GtsObject object;
};

struct _GtsEnergyClass {
	GtsObjectClass parent_class;
	gboolean (*move_face)(GtsEnergy *energy, GtsClusterFace *f, gint old_cn, gint new_cn);
	void (*init_data)(GtsEnergy *energy, GtsSurface *s, guint nclusters); 
};

GtsEnergyClass * gts_energy_class(void);

GtsEnergy *    gts_energy_new(GtsEnergyClass *klass);

void gts_energy_init_data(GtsEnergy *energy, GtsSurface *s, guint nclusters); 

gboolean gts_energy_move_face(GtsEnergy *energy, GtsClusterFace *f, gint old_cn, gint new_cn); 

#define gts_energy_destroy(energy) gts_object_destroy(GTS_OBJECT(energy))

void gts_energy_test(); 

#endif
