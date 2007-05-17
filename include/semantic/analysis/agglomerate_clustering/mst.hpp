// computes and hands you back a Minimum (Weight) Spanning Tree for a graph

#ifndef __SEMANTIC_ANALYSIS_MST_HPP__
#define __SEMANTIC_ANALYSIS_MST_HPP__

#include <boost/pending/disjoint_sets.hpp>
#include <boost/pending/indirect_cmp.hpp>
#include <queue>
#include <map>

namespace semantic { 
	
	namespace detail {
		
		template <class Graph, class It, class WeightMap, class OutputIterator, template <class> class Compare>
		inline void spanning_tree(Graph &g, It edge_start, It edge_end, WeightMap w, OutputIterator out)
		{
			typedef se_graph_traits<Graph> Traits;
			typedef typename Traits::vertex_descriptor vertex;
			typedef typename It::value_type edge;
			typedef typename Traits::degree_size_type degree_type;
			typedef std::map<vertex, vertex> pred_map_type;
			typedef std::map<vertex, typename Traits::vertices_size_type> rank_map_type;
			// we need to create disjoint sets for this algorithm, necessitating a "rank map" (index) and
			// a pred-map (parentage)
			pred_map_type pred_map;
			rank_map_type rank_map;
	
			// make the disjoint set
			boost::disjoint_sets<
				boost::associative_property_map<rank_map_type>,
				boost::associative_property_map<pred_map_type> > 
					dset(boost::make_assoc_property_map(rank_map),
						boost::make_assoc_property_map(pred_map));
						
			typename Traits::vertex_iterator vi, vi_end;
			for(boost::tie(vi, vi_end) = vertices(g); vi != vi_end; ++vi) {
				dset.make_set(*vi);
			}

			// now we need a Q of edges, sorted by their weight
			typedef boost::indirect_cmp<WeightMap, Compare<typename boost::property_traits<WeightMap>::value_type> > weight_compare;
			weight_compare compare(w);
			std::priority_queue<edge, std::vector<edge>, weight_compare> Q(compare);
			
			// push all the edges onto Q

			// this needs to be made faster!
			for(It i = edge_start; i != edge_end; ++i) {
				Q.push(*i);
			}

			while(! Q.empty()) {
				edge e = Q.top();
				Q.pop();
				vertex u, v;
				u = dset.find_set(source(e, g));
				v = dset.find_set(target(e, g));
				if ( u != v ) {
					*out++ = e;
					dset.link(u,v);
				}
			}
		}
		
		template <class G, class IPair, class W, class O, template <class> class C>
		inline void spanning_tree(G &g, IPair i, W w, O o) {
			spanning_tree<G, typename IPair::first_type, W, O, C>(g, i.first, i.second, w, o);
		}
	} // namespace detail
	
	template <class Graph, class WeightMap, class OutputIterator>
	inline void minimum_weight_spanning_tree(Graph &g, WeightMap w, OutputIterator out)
	{
		detail::spanning_tree<Graph, std::pair<typename se_graph_traits<Graph>::edge_iterator, typename se_graph_traits<Graph>::edge_iterator>, WeightMap, OutputIterator, std::greater>(g, edges(g), w, out);
	}
	
	template <class Graph, class EdgeIt, class WeightMap, class OutputIterator>
	inline void minimum_weight_spanning_tree(Graph &g, EdgeIt edge_start, EdgeIt edge_end, WeightMap w, OutputIterator out)
	{
		detail::spanning_tree<Graph, EdgeIt, WeightMap, OutputIterator, std::greater>(g, edge_start, edge_end, w, out);
	}
	
	template <class Graph, class WeightMap, class OutputIterator>
	inline void maximum_weight_spanning_tree(Graph &g, WeightMap w, OutputIterator out)
	{
		detail::spanning_tree<Graph, WeightMap, OutputIterator, std::less>(g, w, out);
	}

	template <class Graph, class EdgeIt, class WeightMap, class OutputIterator>
	inline void maximum_weight_spanning_tree(Graph &g, EdgeIt edge_start, EdgeIt edge_end, WeightMap w, OutputIterator out)
	{
		detail::spanning_tree<Graph, EdgeIt, WeightMap, OutputIterator, std::less>(g, edge_start, edge_end, w, out);
	}	

} // namespace semantic


#endif
