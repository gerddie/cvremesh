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


#ifndef G_CLIST_H
#define G_CLIST_H


#include <glib.h>
#ifdef __cplusplus
extern "C" {
#endif	

	
	typedef struct _GCList GCList; 
	struct _GCList {
		gpointer data; 
		GCList *prev; 
		GCList *next;
	}; 

	guint g_clist_length(GCList *list);
	
	GCList *g_clist_append(GCList *list, gpointer data);
	
	GCList *g_clist_prepend(GCList *list, gpointer data);

	void g_clist_foreach(GCList *list, GFunc f, gpointer data); 

	GCList *g_clist_reverse(GCList *list); 

	
#define g_clist_next(list) list ? list->next: list  
#define g_clist_prev(list) list ? list->prev: list
	
	GCList *g_clist_delete_link(GCList *list, GCList *link);

	void g_clist_free(GCList *); 

	void g_clist_test(); 
#ifdef __cplusplus
}
#endif	

#endif
