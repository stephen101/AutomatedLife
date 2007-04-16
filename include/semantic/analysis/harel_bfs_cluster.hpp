#include <semantic/semantic.hpp>
#include <boost/graph/iteration_macros.hpp>
#include <semantic/utility.hpp>
//#include <ext/hash_map>

	
	//debug
	#include <boost/progress.hpp>

#ifndef __SEMANTIC_ANALYSIS_CLUSTER_HPP__
#define __SEMANTIC_ANALYSIS_CLUSTER_HPP__

using namespace boost;

namespace semantic { namespace analysis {
	/**
	 * cluster by random walk
	 * 
	 * this algorithm does iterative random walk clustering as described in "On Clustering Using Random Walks" (Harel & Koren 2001)
	 */
	
	namespace detail {
		template <class Graph, class WeightMap>
		struct harel_bfs_traits
		{
			typedef typename property_traits<WeightMap>::value_type weight;
			typedef typename graph_traits<Graph>::vertex_descriptor vertex;
			typedef typename std::map<vertex, weight> neighbor_similarity_map;
		};
		
		template <class Graph, class WeightMap>
		inline void
		harel_bfs_create_ns_map(
			Graph &g,
			WeightMap w,
			typename graph_traits<Graph>::vertex_descriptor v,
			int depth,
			typename harel_bfs_traits<Graph, WeightMap>::neighbor_similarity_map &ns,
			bool use_threshold = false)
		{			
			// spider!
			ns_spider(g, w, v, ns, 0, depth, 1, use_threshold);
		}
		
		template <class Graph, class WeightMap>
		inline void
		ns_spider(
			Graph &g, 
			WeightMap w, 
			typename graph_traits<Graph>::vertex_descriptor v,
			typename harel_bfs_traits<Graph, WeightMap>::neighbor_similarity_map &ns,
			int cur_depth,
			int limit,
			typename property_traits<WeightMap>::value_type running_weight,
			bool use_threshold = false)
		{
			if (cur_depth > limit) return; // hit our depth limit. we're done!
			// get this node's neighbors
			typedef typename property_traits<WeightMap>::value_type weight;
			typedef typename graph_traits<Graph>::out_edge_iterator out_edge_iterator;
			typedef typename graph_traits<Graph>::vertex_descriptor vertex;
			
			weight total = 0;
			
			BGL_FORALL_OUTEDGES_T(v,e,g,Graph) total += get(w, e);
			
			out_edge_iterator ei, eend;
			for(tie(ei, eend) = out_edges(v, g); ei != eend; ++ei) {
				// add to the map this edge's weight - if it exists already -- use addition
				vertex target = boost::target(*ei, g);
				weight new_running_weight = running_weight * (get(w, *ei) / total);
				// if new_running_weight is below a certain threshold, let's just ignore it
				if (use_threshold && new_running_weight < 0.001) continue;
				if (ns.count(target)) {
					ns[target] = ns[target] + new_running_weight;
				} else {
					ns[target] = new_running_weight;
				}
				
				// and go ahead and start spidering it, too
				ns_spider(g, w, target, ns, cur_depth + 1, limit, new_running_weight, use_threshold);
			}
		}
		
		template<class Graph, class WeightMap>
		inline typename harel_bfs_traits<Graph, WeightMap>::weight
		ns_difference(
			typename harel_bfs_traits<Graph, WeightMap>::neighbor_similarity_map &one,
			typename harel_bfs_traits<Graph, WeightMap>::neighbor_similarity_map &two)
		{
			typedef typename harel_bfs_traits<Graph,WeightMap>::neighbor_similarity_map nsmap;
			typedef typename harel_bfs_traits<Graph,WeightMap>::weight weight_type;
			typedef typename harel_bfs_traits<Graph,WeightMap>::vertex key_type;
			typedef typename nsmap::iterator iter;
			weight_type diff = 0;
			
			iter one_i, two_i;
			
			for(one_i = one.begin(), two_i = two.begin(); one_i != one.end(); ++one_i) {
				iter it;
				if ((it = two.find((*one_i).first)) != two.end()) {
					// we found the same key in the other iterator

					// if it's not at our current position, copy all the elements up to
					// our current position
					while(two_i != it) {
						diff += t_abs((*two_i).second);
						++two_i;
					}
					
					// now do this element
					diff += t_abs((*one_i).second - (*two_i).second);
					++two_i;
				} else {
					diff += t_abs((*one_i).second);
				}
			}
			
			// lastly if our second list still contains elements, keep going
			while(two_i != two.end()) {
				diff += t_abs((*two_i).second);
				++two_i;
			}

			return diff;
		}
		
	} // namespace detail
	
	// performs the edge separation process (strengthening intra-cluster edges and weakening inter-cluster edges)
	template <class Graph, class WeightMap>
	inline void
	harel_bfs_separation(Graph &g, WeightMap w, int passes, int k = 3)
	{
		typedef typename graph_traits<Graph>::vertex_descriptor vertex;
		typedef typename graph_traits<Graph>::edge_descriptor edge;
		typedef typename graph_traits<Graph>::vertex_iterator vertex_iterator;
		typedef typename graph_traits<Graph>::out_edge_iterator out_edge_iterator;
		typedef typename graph_traits<Graph>::directed_category directed;
		typedef typename property_traits<WeightMap>::value_type weight;
		typedef typename detail::harel_bfs_traits<Graph, WeightMap>::neighbor_similarity_map NSMap;
		
		BOOST_STATIC_ASSERT((is_same<directed, directed_tag>::value));
		function_requires< WritablePropertyMapConcept<WeightMap, edge> >();
		
		// process:
		//	- iterate through all the edges in this graph (however to save computation time we're going to iterate
		//			through each vertex and its out_edges so we're only computing half the NS map for each edge)
		//	- calculate the NS map to a depth of k for each vertex
		//	- use the NS map to apply our separating algorithm
		//	- ^ do above passes times
		
		// this is our new map, which we'll use to hold our new weights until the iteration is done
		std::map<edge, weight> new_weights;
		
		// this will cache our NSMaps
		std::map<vertex, NSMap> ns_cache;
		
		for(int pass = 0; pass < passes; pass++) {
			std::cout << "starting pass " << pass << std::endl;
			vertex_iterator vi, vend;
			vertex target;
			boost::progress_display dsp(num_vertices(g), std::cout);

			for(tie(vi, vend) = vertices(g); vi != vend; ++vi) {
				// calculate the NS map for the source node
				if (!ns_cache.count(*vi)) detail::harel_bfs_create_ns_map(g, w, *vi, k, ns_cache[*vi], true);
				NSMap &source_map = ns_cache[*vi];
				
				// now go through each of the edges coming out of this vertex
				out_edge_iterator ei, eend;
				for(tie(ei, eend) = out_edges(*vi, g); ei != eend; ++ei) {
					// get the NS map for the target node
					target = boost::target(*ei, g);
					if (!ns_cache.count(target)) detail::harel_bfs_create_ns_map(g, w, target, k, ns_cache[target], true);
					NSMap &target_map = ns_cache[target];
					
					// calculate their difference
					weight difference = detail::ns_difference<Graph, WeightMap>(source_map, target_map);
					// use exp(2k - ||x-y||) - 1  --> where ||x-y|| is difference
					new_weights.insert(std::make_pair<edge, weight>(*ei, exp(2 * k - difference)/* - 1*/)); // no longer subtracting 1 to avoid getting negative numbers when difference > 2*k
				}
				++dsp;
			}
			
			// now transfer the new weights over to our old weight map
			for(typename std::map<edge, weight>::iterator i = new_weights.begin(); i != new_weights.end(); ++i) {
				//put(w, (*i).first, (get(w, (*i).first) > 0)?(*i).second:0);
				put(w, (*i).first, (*i).second); // above line is redundant
			}
			new_weights.clear();
			ns_cache.clear();
			
			std::cout << "pass " << pass << " complete." << std::endl;
		}
	}

} } // namespace semantic::analysis

#endif
