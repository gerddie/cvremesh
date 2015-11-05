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


#ifndef CV_CVCENERGY_H
#define CV_CVCENERGY_H

#include <gts.h>
#include <energy.h>

typedef struct _GtsCVCEnergy          GtsCVCEnergy;
typedef struct _GtsCVCEnergyClass     GtsCVCEnergyClass;


#define GTS_IS_CVCENERGY(obj) (gts_object_is_from_class (obj,\
						     gts_cvcenergy_class ()))
#define GTS_CVCENERGY(obj)              GTS_OBJECT_CAST (obj,\
						     GtsCVCEnergy,\
						     gts_cvcenergy_class ())
#define GTS_CVCENERGY_CLASS(klass)      GTS_OBJECT_CLASS_CAST (klass,\
							   GtsCVCEnergyClass,\
							   gts_cvcenergy_class ())

struct _GtsCVCEnergy {
	GtsEnergy object;
	gdouble *area;
	gdouble *angle;
	GtsVector *wcentroid; 
	gint n_clusters; 
	GtsSurface *surface; 
};

struct _GtsCVCEnergyClass {
	GtsEnergyClass parent_class;
};

GtsCVCEnergyClass * gts_cvcenergy_class(void);

GtsCVCEnergy *    gts_cvcenergy_new(GtsCVCEnergyClass *klass);

#define gts_cvcenergy_destroy(energy) gts_object_destroy(GTS_OBJECT(energy))

void gts_cvcenergy_test(); 

#endif
