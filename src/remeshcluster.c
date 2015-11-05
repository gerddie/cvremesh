/***************************************************************************
 *            remeshclusters.h
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

#include <math.h>
#include <edgepool.h>
#include <triangulate.h>
#include <clusterface.h>
#include <remeshcluster.h> 
#include <approxvoronycluster.h>
#include <clist.h>
#include <helper.h>
#include <cvenergy.h>
#include <cvcenergy.h>
#include <paenergy.h>

typedef struct {
	gdouble area; 
	GtsVector centroid; 
	GtsVector normal;
} GtsClusterData; 

typedef struct {
	GtsClusterData *data;
	guint max_cluster;  
} ClusterGetData;

static gint f_get_center_area_normal(GtsClusterFace *f, ClusterGetData *cad)
{
	guint cn = gts_clusterface_get_clusternr(f); 
	if (cn < cad->max_cluster) {
		GtsVector c; 
		gdouble area, x, y, z;
		
		GtsClusterData *d = &cad->data[cn];
		
		area = gts_clusterface_wcentroid(f, c); 
		d->area += area;
		d->centroid[0] += c[0]; 
		d->centroid[1] += c[1];
		d->centroid[2] += c[2];
		
		gts_triangle_normal(GTS_TRIANGLE(f), &x, &y, &z);
		d->normal[0] += area * x; 
		d->normal[1] += area * y;
		d->normal[2] += area * z;
	}else
		g_warning("Clusternr %d not expected", cn); 
	
	return 0; 
}

void gts_surface_get_cluster_data(GtsSurface *s, GtsClusterData *cluster_data, guint cn)
{
	int i; 
	GtsClusterData *d;	
	ClusterGetData cgd = {cluster_data, cn};
	
	gts_surface_foreach_face(s, (GtsFunc)f_get_center_area_normal, &cgd);
	d = cluster_data; 
	
	for (i = 0; i < cn; ++i, ++d) {
		
		if (d->area > 0) {
			d->centroid[0] /= d->area;
			d->centroid[1] /= d->area;
			d->centroid[2] /= d->area;
			d->normal[0] /= d->area;
			d->normal[1] /= d->area;
			d->normal[2] /= d->area;
		}else
			g_warning("Cluster %d has zero area", i); 

	}
}


static gint f_get_normal(GtsTriangle *t, GtsVector normal)
{
	gdouble x, y, z, a; 
	gts_triangle_normal(t, &x, &y, &z);
	a = sqrt(x *x + y *y + z * z); 
	normal[0] += a * x; 
	normal[1] += a * y;
	normal[2] += a * z;
	return 0; 

}


/**
   sort a list of faces according to common edges.
   input: a sigle linked list of faces that share a common vertex
   output: a double linked list of faces where all pairs of neighboring faces 
      share one edge
*/
static GList *sort_faces(GSList *faces)
{
	GList *result = NULL; 
	GtsFace *face; 
	GSList *f = NULL; 
	
	result = g_list_append(result, faces->data); 
	face = GTS_FACE(faces->data); 
	
	// first element is stored
	faces = g_slist_remove_link(faces, faces); 
	
	do  {		
		while (faces) {
			if (gts_triangles_common_edge(GTS_TRIANGLE(faces->data),
						      GTS_TRIANGLE(g_list_first(result)->data))){
				result = g_list_prepend(result, faces->data); 
			}else if (gts_triangles_common_edge(GTS_TRIANGLE(faces->data),
							    GTS_TRIANGLE(g_list_last(result)->data))){
				result = g_list_append(result, faces->data);
			}else {
				f = g_slist_prepend(f, faces->data); 
			}
			faces = g_slist_remove_link(faces, faces);
		}
		faces = f; 
		f = NULL;
	} while (faces); 
	return result;
}

static GCList* get_vertex_clusters(GtsVertex *v, GtsSurface *s, GtsVertex **centroids, GtsVector normal)
{
	GCList *result = NULL;
	GtsVector cross;
	
	GSList *faces = gts_vertex_faces(v, s, NULL);	
	GList *fsorted = sort_faces(faces);
	
	GList *fs = g_list_first(fsorted); 
	gint old_cn = -1; 
	gint nclusters = 0;
	
	GList *fe = g_list_last(fsorted); 
	while (fsorted && gts_clusterface_get_clusternr(GTS_CLUSTERFACE(fs->data)) == 
	       gts_clusterface_get_clusternr(GTS_CLUSTERFACE(fe->data))) {
		fsorted = g_list_delete_link(fsorted, fe); 
		fs = g_list_first(fsorted); 
		fe = g_list_last(fsorted);
	}
	       
	fs = g_list_first(fsorted);
		
	// remove consecutive duplicates in the list  
	while (fs) {
		GtsClusterFace *f = GTS_CLUSTERFACE(fs->data); 
		gint cn = gts_clusterface_get_clusternr(f); 
		if (cn != old_cn) {
			result = g_clist_append(result, centroids[cn]); 
			++nclusters; 
			old_cn = cn; 
		}
		
		fs = g_list_next(fs); 
	}
	g_list_free(fsorted); 
	
	if (nclusters < 3) {
		g_clist_free(result); 
		return NULL; 
	}
	
		
	
	

#if 0	
	// remove loose edges (currently this is done in the triangulation step)
	GCList *r = result;
	do  {
		if (r->prev->data == r->next->data) {
			result = g_clist_delete_link(result, r->next);
			result = g_clist_delete_link(result, r);
			nclusters -= 2;
			r = result; 
		}else
			r = g_clist_next(r);
	}while (r != result && nclusters > 2); 
	
	if (nclusters < 3) {
		g_clist_free(result); 
		return NULL; 
	}
	
#endif	
	// get vertex normal 
	while (faces) {
		gdouble x,y,z;
		gts_triangle_normal(GTS_TRIANGLE(faces->data), &x, &y, &z);
		normal[0] += x; 
		normal[1] += y;
		normal[2] += z;
		faces = g_slist_next(faces); 
	} 
	
	gts_vector_normalize(normal); 
	
	gts_polygon_orientation(result, cross);	
	if (gts_vector_scalar(cross, normal) < 0) {
		//g_debug("reverse list");  
		result = g_clist_reverse(result); 
	}
	

	return result;
}


typedef struct {
	GtsVertex **centroids; 
	GtsSurface *cluster; 
	GtsSurface *surface;
	GtsEdgePool *pool; 
} SurfaceBuildData;


static gint  f_build_surface(GtsVertex *v, SurfaceBuildData *bsd)
{
	
	GtsVector normal = {0.0, 0.0, 0.0};
	GCList* polygon = get_vertex_clusters(v, bsd->cluster, bsd->centroids, normal);
	// create the polygon assuming it is ordered properly
	if (polygon)
		gts_triangulate_convex_polygon(bsd->surface, bsd->pool, polygon);
	return 0;
}

typedef struct {
	GtsVector normal; 
	GtsVector centroid; 
	double d;
	GtsVertex *best; 	
} VertexSearchData; 

static double distance_point_line(GtsPoint *p, GtsVector x1, GtsVector n)
{
	GtsVector dxp;
	GtsVector cross; 
	dxp[0] = x1[0] - p->x; 
	dxp[1] = x1[1] - p->y; 
	dxp[2] = x1[2] - p->z;
	gts_vector_cross(cross, n, dxp); 
	return gts_vector_norm(cross);
}

static gint f_closest_to_line(GtsVertex *v, VertexSearchData *vsd)
{
	double d = distance_point_line(GTS_POINT(v), vsd->centroid, vsd->normal); 
	if (d < vsd->d || !vsd->best) {
		vsd->d = d;
		vsd->best = v;
	}
	return 0;
}


static GtsVertex *gts_triangle_ray_intersect_vertex(GtsTriangle *triangle, GtsVector raypoint, GtsVector dir)
{
	/*	Based on: 
	  Copyright 2001, softSurfer (www.softsurfer.com)
	  This code may be freely used and modified for any purpose
	  providing that this copyright notice is included with it.
	  SoftSurfer makes no warranty for this code, and cannot be held
	  liable for any real or imagined damage resulting from its use.
	  Users of this code must verify correctness for their application.
	*/
	
	GtsVector    u, v, n;             // triangle vectors
	GtsVector    w0, w;          // ray vectors
	GtsVertex     *result;
	GtsVertex    *t1, *t2, *t3; 
	double     r, a, b;             // params to calc ray-plane intersect

	// get triangle edge vectors and plane normal
	gts_triangle_vertices(triangle, &t1, &t2, &t3); 
	
	gts_vector_init(u, GTS_POINT(t1), GTS_POINT(t2)); 
	gts_vector_init(v, GTS_POINT(t1), GTS_POINT(t3)); 
	gts_vector_cross(n, u, v); 
	
	if (gts_vector_scalar(n,n) < 1e-10) /* degenerate triangle? */
		return NULL; 
	
	w0[0] = raypoint[0] - GTS_POINT(t1)->x; 
	w0[1] = raypoint[1] - GTS_POINT(t1)->y; 
	w0[2] = raypoint[2] - GTS_POINT(t1)->z; 
	
		
	a = -gts_vector_scalar(n, w0);
	b = gts_vector_scalar(n, dir);
	if ( fabs(b) < 1e-10 )     // ray is parallel to triangle plane	
		return NULL;

	// get intersect point of ray with triangle plane
	r = a / b;
	g_debug("a=%e, b = %e r=%e ", a, b, r); 
	
	result = gts_vertex_new(gts_vertex_class(),
				raypoint[0] + r * dir[0], 
				raypoint[1] + r * dir[1],
				raypoint[2] + r * dir[2]);
	
	// is I inside T?
	float    uu, uv, vv, wu, wv, D;
	
	uu = gts_vector_scalar(u,u);
	uv = gts_vector_scalar(u,v);
	vv = gts_vector_scalar(v,v);
	gts_vector_init(w, GTS_POINT(t1), GTS_POINT(result));
	wu = gts_vector_scalar(w,u);
	wv = gts_vector_scalar(w,v);
	D = uv * uv - uu * vv;

	// get and test parametric coords
	float s, t;
	s = (uv * wv - vv * wu) / D;
	if (s < 0.0 || s > 1.0) {
		gts_object_destroy(GTS_OBJECT(result)); 
		return NULL;
	}
	t = (uv * wu - uu * wv) / D;
	if (t < 0.0 || (s + t) > 1.0) { 
		gts_object_destroy(GTS_OBJECT(result));
		return 0;
	}
	return result; 
}


static GtsVertex *gts_surface_closest_point_along_cnormal(GtsSurface *cluster)
{
	VertexSearchData vsd; 
	g_assert(gts_surface_vertex_number(cluster) > 0); 
	vsd.best = NULL; 
	vsd.d = 0.0; 
	
	vsd.normal[0] =  vsd.normal[1] = vsd.normal[2] = 0.0;  
	gts_surface_foreach_face(cluster, (GtsFunc)f_get_normal, vsd.normal);
	gts_vector_normalize(vsd.normal); 
	g_debug( "cluster normal %f %f %f", vsd.normal[0], vsd.normal[1], vsd.normal[2]);  
	
	gts_surface_center_of_area(cluster, vsd.centroid);

	gts_surface_foreach_vertex(cluster, (GtsFunc)f_closest_to_line,&vsd);
	GSList *faces = gts_vertex_faces(vsd.best,cluster,  NULL); 
	while (faces) {
		GtsVertex *r = gts_triangle_ray_intersect_vertex(GTS_TRIANGLE(faces->data), vsd.centroid, vsd.normal); 
		if (r)
			return r;
		faces = g_slist_next(faces); 
	}
	g_debug("reuse vertex"); 
	return gts_vertex_new(gts_vertex_class(), GTS_POINT(vsd.best)->x, GTS_POINT(vsd.best)->y, GTS_POINT(vsd.best)->z); 
}

static GtsVertex **get_corners_b(GtsSurface *cluster, guint n)
{
	
	int i = 0; 
	SurfaceClusterParams scp; 
	scp.nclusters = n;
	scp.clusters = g_new0(GtsSurface *, n); 
	for (i = 0; i < n; ++i){
		scp.clusters[i] = gts_surface_new(gts_surface_class(), 
						  gts_face_class(), gts_edge_class(), gts_vertex_class()); 
	}
	
	GtsVertex **result = g_new0(GtsVertex*, n);

	gts_surface_get_clusters(cluster, &scp);
	
	for (i = 0; i < n; ++i){
		result[i] = gts_surface_closest_point_along_cnormal(scp.clusters[i]);
		g_debug("Assign vertex %i: (%10.5f %10.5f %10.5f)", 
			  i, GTS_POINT(result[i])->x, GTS_POINT(result[i])->y, GTS_POINT(result[i])->z);   
		gts_object_destroy(GTS_OBJECT(scp.clusters[i])); 
	}
	g_free(scp.clusters); 
	return result;
}


static GtsVertex **get_centroids(GtsSurface *cluster, GtsClusterData *cluster_data, guint n)
{
	int i = 0; 
	
	GtsClusterData *d = cluster_data; 
	GtsVertex **result = g_new0(GtsVertex*, n);
	GtsVertex **r = result; 
	
	for (i = 0; i < n; ++i, ++r, ++d) { 
		*r = gts_vertex_new(gts_vertex_class(), d->centroid[0],  d->centroid[1],  d->centroid[2]);
		g_debug("Centriod %3d: %6.2f %6.2f %6.2f", i, d->centroid[0],  d->centroid[1],  d->centroid[2]); 
	}
	return result; 
}

GtsSurface *gts_surface_remesh_clusters(GtsSurface *cluster, guint n, gboolean area_opt)
{

	GtsClusterData *cluster_data; 
	SurfaceBuildData bsd;
	
	cluster_data = g_new0(GtsClusterData, n); 

	gts_surface_get_cluster_data(cluster, cluster_data, n); 
		
	bsd.centroids = area_opt ? get_corners_b(cluster,  n) : get_centroids(cluster, cluster_data, n);
	bsd.cluster = cluster; 
	
	bsd.surface = gts_surface_new(gts_surface_class(), 
				      gts_face_class(),
				      gts_edge_class(), 
				      gts_vertex_class());
	
	bsd.pool = gts_edge_pool_new(gts_edge_pool_class()); 

	gts_surface_foreach_vertex(cluster, (GtsFunc)f_build_surface, &bsd); 
	

	g_free(cluster_data); 
	g_free(bsd.centroids);
	
	gts_object_destroy(GTS_OBJECT(bsd.pool)); 
	
	return bsd.surface; 
}


static gint f_resize(GtsPoint *p, gpointer data) 
{
	p->x = (p->x + 0.5) * 256; 
	p->y = (p->y + 0.5) * 128; 
	p->z = (p->z + 0.5) * 64;
	return 0; 
}


static gint f_count_assigned(GtsClusterFace *f, guint *n)
{
	if (gts_clusterface_get_clusternr(f) >=0)
		++(*n); 
	return 0; 
}

static guint gts_surface_faces_assigned(GtsSurface *s)
{
	guint n = 0; 
	gts_surface_foreach_face(s, (GtsFunc)f_count_assigned, &n);
	return n; 
}

static gint f_test_vertex_facesort(GtsVertex *v, GtsSurface *surface)
{
	GSList *faces = gts_vertex_faces(v, surface, NULL);	
	guint l = g_slist_length(faces);  
	g_message("vertex with %d faces", l);
	if ( l < 3)
		return 0; 
	
	GList *sorted = sort_faces(faces);
	GList *s = sorted; 
	
	gint old = -1; 
	while (s) {
		GtsClusterFace *f = GTS_CLUSTERFACE(s->data); 
		g_assert((old == -1) ||
			 (old < gts_clusterface_get_clusternr(f)) ||
			 (old == 2 && gts_clusterface_get_clusternr(f) == 0));
		s = g_list_next(s); 
	}
	g_list_free(sorted);
	return 0; 
}
			    
void gts_vertex_face_sort_test()
{
#define  nVtx   5
#define  nEdge  8 
#define  nFace  4

	
	double v[nVtx][3] = { { 0.0, 0.0, 0.0},
			       {-1.0, 0.0, 0.0},
			       { 0.0, 1.0, 0.0},
			       { 1.0, 0.0, 0.0},
			       { 0.0,-1.0, 0.0}
	}; 
	
	int e[nEdge][2] = {
		{ 0, 1}, // 0 
		{ 0, 2}, // 1
		{ 0, 3}, // 2
		{ 0, 4}, // 3
		{ 1, 2}, // 4
		{ 2, 3}, // 5 
		{ 3, 4}, 
		{ 4, 1}
	}; // 6 
	
	int f[nFace][4] = { {0, 4, 1, 0}, { 2, 6, 3, 2}, { 3, 7, 0, 0}, { 1, 5, 2, 1} };
	
	g_message("run vertex face sort test"); 
	GtsSurface *surface = create_clusterface_surface(nVtx, v, nEdge, e, nFace, f); 
	
		
	gts_surface_foreach_vertex(surface, (GtsFunc)f_test_vertex_facesort, surface);

	
	gts_object_destroy(GTS_OBJECT(surface)); 

	g_message("done vertex face sort test");
}

void 	gts_surface_remesh_clusters_test()
{
	int i; 
	const int nclusters = 50;
	GtsSurface *result, *cluster_surface, *result2; 
	GtsSurface *s =	create_std_surface();

	g_message("run remesh test"); 
	gts_surface_generate_sphere(s,5);
	
	// shift and resize the sphere
	gts_surface_foreach_vertex(s,(GtsFunc)f_resize,0); 
	
	FILE *f = fopen("sphere.gts","w"); 
	
	gts_surface_write(s, f); 
	fclose(f);
	
	GtsEnergy *cvenergy = GTS_ENERGY(gts_cvenergy_new(gts_cvenergy_class())); 
	GtsEnergy *paenergy = GTS_ENERGY(gts_paenergy_new(gts_paenergy_class())); 

	
	cluster_surface = gts_surface_cluster_copy(s, nclusters, cvenergy, NULL, NULL);
	gts_surface_cluster(cluster_surface, nclusters, paenergy, FALSE, NULL, NULL);

	g_assert(gts_surface_faces_assigned(cluster_surface) == gts_surface_face_number(s)); 
	

	result = gts_surface_remesh_clusters(cluster_surface, nclusters, TRUE);
	g_assert(GTS_IS_SURFACE(result));
	
	g_assert(gts_surface_vertex_number(result) == nclusters);
	
	f = fopen("sphere_ao.gts","w");
	gts_surface_write(result, f); 
	fclose(f);
	
	result2 = gts_surface_remesh_clusters(cluster_surface, nclusters, FALSE);

	f = fopen("sphere_ce.gts","w");
	gts_surface_write(result2, f); 
	fclose(f);

	
	SurfaceClusterParams scp; 
	scp.clusters = g_new(GtsSurface *, nclusters); 
	scp.nclusters = nclusters; 
	
	for (i = 0; i < nclusters; ++i) {
		scp.clusters[i] = gts_surface_new(gts_surface_class(), 
						  gts_face_class(), 
						  gts_edge_class(), 
						  gts_vertex_class()); 
	}
	
	gts_surface_get_clusters(cluster_surface, &scp); 
	for (i = 0; i < nclusters; ++i) {
		char fname[100]; 
		snprintf(fname,100, "cluster%03d.gts", i); 
		FILE *f = fopen(fname, "w");
		gts_surface_write(scp.clusters[i], f); 
		gts_object_destroy(GTS_OBJECT(scp.clusters[i])); 
		fclose(f); 
	}
	g_free(scp.clusters); 
	
	
	GtsSurfaceStats sstats; 
	gts_surface_stats(result, &sstats);
	g_assert(sstats.n_incompatible_faces == 0); 
	g_assert(sstats.n_duplicate_faces == 0);	
	g_assert(sstats.n_duplicate_edges == 0);	
	g_assert(sstats.n_boundary_edges == 0);	
	g_assert(sstats.n_non_manifold_edges == 0);	
	g_assert(sstats.faces_per_edge.min == 2); 
	g_assert(sstats.faces_per_edge.max == 2);	
	
	gts_energy_destroy(cvenergy);
	gts_energy_destroy(paenergy);
	gts_object_destroy(GTS_OBJECT(cluster_surface)); 
	gts_object_destroy(GTS_OBJECT(result2));
	gts_object_destroy(GTS_OBJECT(result)); 
	gts_object_destroy(GTS_OBJECT(s));

	g_message("Sphere remesh test PASS"); 
}
