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


#include "clist.h"
#include <glib.h>

GCList *g_clist_append(GCList *list, gpointer data)
{
	GCList *node = g_slice_new(GCList); 
	node->data = data; 
	if (list) {
		node->next = list->next; 
		node->prev = list; 
		list->next->prev = node; 
		list->next = node;
		return list; 
	}else{
		node->next = node; 
		node->prev = node;
		return node; 
	}
}


guint g_clist_length(GCList *list)
{
	GCList *start = list; 
	g_return_val_if_fail(list, 0); 
	
	guint n = 1; 
	while (start->next != list) {
		start = start->next; 
		++n; 
	}
	return n;
}

GCList *g_clist_reverse(GCList *list) 
{
	g_return_val_if_fail(list, NULL); 
	
	GCList *last = list; 
	GCList *head = last->next;
	do {
		
		last->next = last->prev; 
		last->prev = head; 
		last = head;
		head = head->next;
	} while (last != list); 
	
	return list;
}

GCList *g_clist_prepend(GCList *list, gpointer data)
{
	GCList *node = g_slice_new(GCList); 
	node->data = data; 
	if (list) {
		node->prev = list->prev; 
		node->next = list; 
		list->prev->next = node;
		list->prev = node; 
		return list; 
	}else{
		node->next = node; 
		node->prev = node;
		return node; 
	}
}

GCList *g_clist_delete_link(GCList *list, GCList *link)

{
	g_assert(list);
	
	GCList *head = list;
	GCList *node = list; 
	while (link != node && node->next != head)
		node = node->next;
	
	
	if (node->next == head && node->next != node) {
		g_warning("requested node not in the list"); 
		return list; 
	}
	
	// if the head is to be removed, move the head to the next node
	if (head == node)
		head = node->next;
	
	// remove the links; 
	node->prev->next = node->next; 
	node->next->prev = node->prev;
	g_slice_free(GCList, node); 
	
	return head != node ? head : NULL;
}

void g_clist_free(GCList *list)
{
	if (list) {
		GCList *n = list; 
		do {
			n = g_clist_delete_link(n, n);
		} while (n != NULL); 
	}
}

void g_clist_foreach(GCList *list, GFunc f, gpointer data) 
{
	g_return_if_fail(list); 

	GCList *head = list;
	do {
		f(head->data, data);
		head = g_clist_next(head); 
	} while (head != list);  
	
}
		     
void f_print(gpointer item, GString *output) 
{
	g_string_append_printf(output, "%d", GPOINTER_TO_UINT(item)); 
}

void g_clist_test() 
{
	GCList *l = NULL; 
	guint numbers[4] = { 1, 2, 3, 4}; 
	GString *result = g_string_new(""); 
	GString *proper = g_string_new("1324"); 
	GString *rproper = g_string_new("1423"); 
	
	
	l = g_clist_append(l, GUINT_TO_POINTER(numbers[0])); 
	l = g_clist_prepend(l, GUINT_TO_POINTER(numbers[1]));
	g_assert(g_clist_length(l) == 2); 
	l = g_clist_append(l, GUINT_TO_POINTER(numbers[2]));
	l = g_clist_prepend(l, GUINT_TO_POINTER(numbers[3])); 

	g_assert(g_clist_length(l) == 4); 
	
	g_clist_foreach(l, (GFunc)f_print, result);	
	g_assert(g_string_equal(result, proper));
	
	g_assert(GPOINTER_TO_UINT(l->data) == 1);
	g_assert(GPOINTER_TO_UINT(l->prev->data) == 4);
	g_assert(GPOINTER_TO_UINT(l->next->data) == 3);
	g_assert(GPOINTER_TO_UINT(l->prev->prev->data) == 2);
	g_assert(GPOINTER_TO_UINT(l->next->next->data) == 2);
	
	result = g_string_assign(result, ""); 
	l = g_clist_reverse(l); 
	g_clist_foreach(l, (GFunc)f_print, result);
	g_assert(g_string_equal(result, rproper));
	
	g_string_free(result, TRUE); 
	g_string_free(proper, TRUE);
	g_string_free(rproper, TRUE);
	g_clist_free(l); 
	g_message("CList PASSED"); 
}

	
