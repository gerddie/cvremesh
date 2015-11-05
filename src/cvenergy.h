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


#ifndef CV_CVENERGY_H
#define CV_CVENERGY_H

#include <gts.h>
#include <energy.h>

typedef struct _GtsCVEnergy          GtsCVEnergy;
typedef struct _GtsCVEnergyClass     GtsCVEnergyClass;


#define GTS_IS_CVENERGY(obj) (gts_object_is_from_class (obj,\
						     gts_cvenergy_class ()))
#define GTS_CVENERGY(obj)              GTS_OBJECT_CAST (obj,\
						     GtsCVEnergy,\
						     gts_cvenergy_class ())
#define GTS_CVENERGY_CLASS(klass)      GTS_OBJECT_CLASS_CAST (klass,\
							   GtsCVEnergyClass,\
							   gts_cvenergy_class ())

struct _GtsCVEnergy {
	GtsEnergy object;
	gdouble *area; 
	GtsVector *wcentroid; 
	gint n_clusters; 
};

struct _GtsCVEnergyClass {
	GtsEnergyClass parent_class;
};

GtsCVEnergyClass * gts_cvenergy_class(void);

GtsCVEnergy *    gts_cvenergy_new(GtsCVEnergyClass *klass);

#define gts_cvenergy_destroy(energy) gts_object_destroy(GTS_OBJECT(energy))

void gts_cvenergy_test(); 

#endif
