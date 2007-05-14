/**
 * this file offers Silhouette cluster "quality" measure.
 * 
 * it expects a graph, a weight map (symmetrized), and a property map containing component numbers for each vertex
 */

#ifndef __SEMANTIC_ANALYSIS_SILHOUETTE_HPP__
#define __SEMANTIC_ANALYSIS_SILHOUETTE_HPP__

#include <semantic/semantic.hpp>
#include <semantic/analysis/shortest_paths.hpp>

namespace semantic { namespace analysis {

	template <class Graph>
	struct silhouette_traits {
		typedef se_graph_traits<Graph> 							_traits;
		typedef typename _traits::vertex_descriptor 			vertex;
		typedef typename weighting_traits<Graph>::weight_type	length_type;
		typedef maps::unordered<
			vertex,
			length_type>								 		single_distances_map;
		typedef maps::unordered<
			vertex,
			single_distances_map>	 							distances_map;
	};

	template <class Graph, class WeightMap, class OutputMap>
	inline void shortest_paths_forall_vertices(Graph &g, WeightMap weights, OutputMap shortest_paths)
	{
		BGL_FORALL_VERTICES_T(u, g, Graph) {
			typename silhouette_traits<Graph>::single_distances_map spm;
			dijkstra_shortest_paths(g, u, weights, boost::make_assoc_property_map(spm));
			put(shortest_paths, u, spm);
		}
	}
	
	template <class Graph, class ComponentMap, class ShortestPathsMap, class OutputMap>
	inline typename property_traits<OutputMap>::value_type
	silhouette(Graph &g, ComponentMap components, ShortestPathsMap distances, OutputMap quality)
	{
		typedef typename boost::property_traits<OutputMap>::value_type quality_measure_type;
		typedef typename boost::property_traits<ComponentMap>::value_type component_type;
		typedef typename silhouette_traits<Graph>::length_type weight_type;
		typedef typename se_graph_traits<Graph>::vertex_iterator vertex_iterator;
		typedef typename se_graph_traits<Graph>::vertex_descriptor vertex;
		
		// for storing cluster quality and cluster size
		std::vector<quality_measure_type> cluster_quality(num_vertices(g), 0);
		std::vector<unsigned long> cluster_size(num_vertices(g), 0);
		unsigned long cluster_count = 0;
		
		// iterate over the vertices and compute the a and b values (ie, the silhouette)
		vertex_iterator vi, vi_end;
		for(boost::tie(vi, vi_end) = vertices(g); vi != vi_end; ++vi) {
			typename silhouette_traits<Graph>::single_distances_map &path_length = get(distances, *vi);
			
			// now essentially we add up the "a" and "b" values
			quality_measure_type a = 0;
			unsigned long a_count = 0;
			std::vector<quality_measure_type> b(num_vertices(g), 0);		// to be safe we init with num_vertices(g)
			std::vector<unsigned long> b_count(num_vertices(g), 0);
			
			component_type this_component = get(components, *vi);
			// go through each vertex and add the shortest path value onto either "a" or "b" depending
			// on if it's in our component or not
			vertex_iterator vi2, vi2_end;
			for(boost::tie(vi2, vi2_end) = vertices(g); vi2 != vi2_end; ++vi2) {
				if (*vi == *vi2) continue; // don't do ourselves
				weight_type len = path_length[*vi2];
				component_type that_component = get(components, *vi2);
				if (that_component == this_component) {
					a += len;
					a_count++;
				} else {
					b[that_component] += len;
					b_count[that_component]++;
				}
			}
			
			// now a and b have the total "dissimilarity" measures of in-cluster and out-of-cluster
			// let's find the smallest b
			quality_measure_type b_best = 0;
			for(unsigned long i = 0; i < b.size(); i++) {
				if (b[i] == 0) continue;
				b[i] /= b_count[i];
				if (b[i] < b_best || b_best == 0) b_best = b[i];
			}
			a = a_count==0?0:a/a_count;
			quality_measure_type m = a>b_best?a:b_best;

			quality_measure_type s = (b_best - a) / m;
			if (a_count == 0) s = 0; // experiment -- if our cluster only has one node, it gets a quilty of zero
			put(quality, *vi, s);
			cluster_quality[this_component] += s;
			cluster_size[this_component]++;
			if (this_component > cluster_count) cluster_count = this_component;
		}
		
		// averageize (a real word) the cluster qualities
		quality_measure_type average = 0;
		cluster_count++;
		for(unsigned long i = 0; i < cluster_count; i++) {
			if (cluster_size[i] == 0) continue;
			average += cluster_quality[i] / cluster_size[i];
		}
		
		return average / cluster_count;
	}
	
}} // namespace semantic::analysis


#endif


