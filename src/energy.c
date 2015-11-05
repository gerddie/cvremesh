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


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "energy.h"


static void energy_destroy(GtsObject *object)
{
	g_assert(GTS_IS_ENERGY(object));
	(* GTS_OBJECT_CLASS (gts_energy_class ())->parent_class->destroy) (object);
}

static void energy_class_init(GtsEnergyClass *klass)
{
	klass->move_face = NULL;
	klass->init_data = NULL;
	GTS_OBJECT_CLASS (klass)->destroy = energy_destroy;
}

static void energy_init(GtsEnergy *energy)
{
	
}

GtsEnergyClass * gts_energy_class(void)
{
	static GtsEnergyClass * klass = NULL;
	
	if (klass == NULL) {
		GtsObjectClassInfo energy_info = {
			"GtsEnergy",
			sizeof (GtsEnergy),
			sizeof (GtsEnergyClass),
			(GtsObjectClassInitFunc) energy_class_init,
			(GtsObjectInitFunc) energy_init,
			(GtsArgSetFunc) NULL,
			(GtsArgGetFunc) NULL
		};
		klass = gts_object_class_new (gts_object_class (), 
					      &energy_info);
	}
	
	return klass;

}

gboolean gts_energy_move_face(GtsEnergy *energy, GtsClusterFace *f, gint old_cn, gint new_cn)
{
	GtsEnergyClass *klass = GTS_ENERGY_CLASS (GTS_OBJECT (energy)->klass); 
	if (!klass->move_face)
		g_assert(!"gts_energy_move_face for abstract baseclass called"); 
	
	return klass->move_face(energy, f, old_cn, new_cn); 
}

void gts_energy_init_data(GtsEnergy *energy, GtsSurface *s, guint nclusters)
{
	GtsEnergyClass *klass = GTS_ENERGY_CLASS (GTS_OBJECT (energy)->klass); 
	if (klass->init_data)
		klass->init_data(energy, s, nclusters); 
}

GtsEnergy *    gts_energy_new(GtsEnergyClass * klass)
{
	return GTS_ENERGY (gts_object_new (GTS_OBJECT_CLASS(klass)));
}



void gts_energy_test()    
{
	GtsEnergy *energy = gts_energy_new(gts_energy_class()); 
	g_assert(GTS_IS_ENERGY(energy));
	gts_energy_destroy(energy); 
	
}
