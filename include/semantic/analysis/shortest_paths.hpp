#ifndef __SEMANTIC_ANALYSIS_SHORTEST_PATHS_HPP__
#define __SEMANTIC_ANALYSIS_SHORTEST_PATHS_HPP__

/*
	this offers a dijkstra_shortest_paths funciton which works with directed graphs as well as undirected
	
	it assumes that all edges are bidirectional (for directed graphs this means that an opposite edge exists
	in the graph for every other edge) and that weights are symmetric
*/

#include <semantic/utility.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>

namespace semantic { namespace analysis {
	
	template <class Graph, class Vertex, class WeightMap, class PathLengthMap, class PathMap>
	inline void dijkstra_shortest_paths(const Graph &g, Vertex v, WeightMap weights, PathLengthMap path_lengths, PathMap paths)
	{
		maps::unordered<Vertex, typename se_graph_traits<Graph>::vertices_size_type> vindex;
		typename se_graph_traits<Graph>::vertex_iterator vi, vi_end;
		typename se_graph_traits<Graph>::vertices_size_type cnt;
		for(boost::tie(vi, vi_end) = vertices(g), cnt = 0; vi != vi_end; ++vi, cnt++) {
			vindex[*vi] = cnt;
		}
		boost::dijkstra_shortest_paths(g, v, boost::weight_map(weights).distance_map(path_lengths).predecessor_map(paths).vertex_index_map(boost::make_assoc_property_map(vindex)));
	}
	
	template <class Graph, class Vertex, class WeightMap, class PathLengthMap>
	inline void dijkstra_shortest_paths(const Graph &g, Vertex v, WeightMap weights, PathLengthMap path_lengths)
	{
		typedef typename se_graph_traits<Graph>::vertex_descriptor vertex;
		maps::unordered<vertex, vertex> paths;
		dijkstra_shortest_paths(g, v, weights, path_lengths, boost::make_assoc_property_map(paths));
	}
	
}} // namespace semantic::analysis

#endif
