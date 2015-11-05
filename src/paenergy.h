/***************************************************************************
 *            cvenergy.h
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


#ifndef CV_CFAENERGY_H
#define CV_CFAENERGY_H

#include <gts.h>
#include <energy.h>

typedef struct _GtsPAEnergy          GtsPAEnergy;
typedef struct _GtsPAEnergyClass     GtsPAEnergyClass;


#define GTS_IS_PAENERGY(obj) (gts_object_is_from_class (obj,\
						     gts_paenergy_class ()))
#define GTS_PAENERGY(obj)              GTS_OBJECT_CAST (obj,\
						     GtsPAEnergy,\
						     gts_paenergy_class ())
#define GTS_PAENERGY_CLASS(klass)      GTS_OBJECT_CLASS_CAST (klass,\
							   GtsPAEnergyClass,\
							   gts_paenergy_class ())

struct _GtsPAEnergy {
	GtsEnergy object;
	gdouble *area; 
	gdouble *perimeter;
	GtsSurface *surface; 
	gint n_clusters; 
};

struct _GtsPAEnergyClass {
	GtsEnergyClass parent_class;
};

GtsPAEnergyClass * gts_paenergy_class(void);

GtsPAEnergy *    gts_paenergy_new(GtsPAEnergyClass *klass);

#define gts_paenergy_destroy(energy) gts_object_destroy(GTS_OBJECT(energy))

void gts_paenergy_test(); 

#endif
