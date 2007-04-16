

#ifndef __SEMANTIC_ANALYSIS_CONNECTED_COMPONENTS_HPP__
#define __SEMANTIC_ANALYSIS_CONNECTED_COMPONENTS_HPP__

#include <semantic/semantic.hpp>
#include <map>
#include <vector>

namespace semantic { namespace analysis {
	// takes a graph, weight map, and threshold and performs connected components on that graph
	// by treating each edge with weight < threshold should not be followed
	// any two vertices u, v are treated as in the same component if either a path from u->v exists
	// or one fro v->u
	namespace detail {
		template <class Graph, class EdgeList, class WeightMap, class ComponentMap, class SeenMap, class MapVector>
//		inline typename property_traits<ComponentMap>::value_type
		inline void
		weak_components_tagger_rec
			(Graph &g, EdgeList &edge_list, typename graph_traits<Graph>::vertex_descriptor u,
			typename property_traits<ComponentMap>::value_type comp, WeightMap w, ComponentMap c,
			typename property_traits<WeightMap>::value_type threshold, SeenMap &seen, MapVector &v)
		{
			typedef graph_traits<Graph> traits;
			typedef typename traits::vertex_descriptor Vertex;
			typedef typename EdgeList::value_type::second_type::iterator eiterator;
			typedef typename traits::edge_descriptor Edge;
			typedef typename property_traits<WeightMap>::value_type weight_type;
			typedef typename property_traits<ComponentMap>::value_type comp_type;

			// set this vertex's component
			put(c, u, comp);
			seen.insert(u);

			// spider starting on vertex u, paying attention to the component of target
			// vertices and the edge weight (if it falls below threshold)
			eiterator ei, ei_end;
			for(ei = (edge_list[u].begin()); ei != edge_list[u].end(); ++ei) {
				if (seen.count((*ei).first)) continue;
				if ((*ei).second < threshold) continue;

				weak_components_tagger_rec(g, edge_list, (*ei).first, comp, w, c, threshold, seen, v);
			}
		}
	} // namespace detail

	template <class Graph, class WeightMap, class ComponentMap>
	inline typename property_traits<ComponentMap>::value_type
	weak_components_by_threshold(Graph &g, WeightMap w, ComponentMap c, typename property_traits<WeightMap>::value_type threshold)
	{
		if (num_vertices(g) == 0) return 0;

		typedef graph_traits<Graph> traits;
		typedef typename traits::vertex_descriptor Vertex;
		typedef typename traits::vertex_iterator viterator;
		typedef typename traits::out_edge_iterator eiterator;
		typedef typename property_traits<ComponentMap>::value_type comp_type;
		typedef typename property_traits<WeightMap>::value_type Weight;
		typedef typename boost::graph_traits<Graph>::directed_category directed;
		BOOST_STATIC_ASSERT((boost::is_same<directed, directed_tag>::value));

		typedef typename property_traits<ComponentMap>::value_type comp_type;
		// to compensate for the directed nature of the graph, we're going to construct
		// our own edge list that ensure bidirectionality -- i don't like this method
		// but can't think of anything better right now.
		std::map<Vertex, std::vector<std::pair<Vertex, Weight> > > edge_list;
		comp_type c_count((std::numeric_limits<comp_type>::max)());

		// initialize the component map to "null" (actually max value)
		viterator vi, vi_end;
		for(tie(vi, vi_end) = vertices(g); vi != vi_end; ++vi) {
			put(c, *vi, c_count);
			eiterator ei, ei_end;
			for(tie(ei, ei_end) = out_edges(*vi, g); ei != ei_end; ++ei) {
				edge_list[*vi].push_back(std::pair<Vertex, Weight>(target(*ei, g), get(w, *ei)));
				if (!(edge(target(*ei, g), *vi, g).second)) {
					edge_list[target(*ei, g)].push_back(std::pair<Vertex, Weight>(*vi, get(w, *ei)));
				}
			}
		}

		std::set<Vertex> seen;
		std::vector<std::pair<comp_type, comp_type> > mapvector;

		for(c_count = 0, tie(vi, vi_end) = vertices(g); vi != vi_end; ++vi) {
			if (get(c, *vi) != (std::numeric_limits<comp_type>::max)()) continue;
			detail::weak_components_tagger_rec(g, edge_list, *vi, c_count, w, c, threshold, seen, mapvector);
			seen.clear();
			c_count++;
		}

		return c_count;
	}
} } // namespace semantic::analysis


#endif
