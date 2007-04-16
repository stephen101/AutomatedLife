#include <semantic/semantic.hpp>
#include <boost/graph/depth_first_search.hpp>
#include <boost/graph/iteration_macros.hpp>
#include <semantic/utility.hpp>
#include <boost/random/linear_congruential.hpp>
#include <time.h>
//#include <ext/hash_map>

	
	//debug
	#include <boost/progress.hpp>

#ifndef __SEMANTIC_ANALYSIS_STRONG_NEIGHBOR_CLUSTER_HPP__
#define __SEMANTIC_ANALYSIS_STRONG_NEIGHBOR_CLUSTER_HPP__

using namespace boost;

namespace semantic { namespace analysis {
	/**
	 * cluster by random walk
	 * 
	 * this algorithm does iterative random walk separation/strengthening of edges
	 */
	
	namespace detail {
		template <class Graph, class WeightMap>
		struct harel_simple_random_walk_traits
		{
			typedef typename property_traits<WeightMap>::value_type weight;
			typedef typename graph_traits<Graph>::vertex_descriptor vertex;
			typedef typename std::map<vertex, weight> neighbor_similarity_map;
		};
		
		template <class Graph, class WeightMap, class RandomGenerator>
		inline void
		harel_simple_create_ns_map(
			Graph &g,
			WeightMap w,
			typename graph_traits<Graph>::vertex_descriptor v,
			int depth,
			int trials,
			RandomGenerator &rand,
			typename harel_simple_random_walk_traits<Graph, WeightMap>::neighbor_similarity_map &ns)
		{
			typedef typename graph_traits<Graph>::vertex_descriptor Vertex;
			typedef typename graph_traits<Graph>::edge_descriptor Edge;
			typedef typename graph_traits<Graph>::out_edge_iterator edge_iterator;
			typedef typename property_traits<WeightMap>::value_type weight_type;
			
			// init
			std::map<Vertex, int> runs, runs_next;
			std::map<Vertex, weight_type> total_cache;
			std::set<Vertex> fringe, fringe_next;
			
			runs[v] = trials;
			fringe.insert(v);
			
			// trials times, run a random walk of <depth>
			for(int d = 0; d < depth; d++) {
				for(typename std::set<Vertex>::iterator f = fringe.begin(); f != fringe.end(); ++f) {
					Vertex u = *f;
					weight_type total;
					if (!total_cache.count(u)) {
						total = 0;
						BGL_FORALL_OUTEDGES_T(v,e,g,Graph) total += get(w, e);
						total_cache[u] = total;
					} else total = total_cache[u];
					
					// perform random neighbor selection runs[u] times
					for(int i = 0; i < runs[u]; i++) {
						// get a random number
						double r = (double) (rand() + RandomGenerator::min_value)/(RandomGenerator::max_value - RandomGenerator::min_value);
						// now find a neighbor that hits that random number
						double cum = 0;
						edge_iterator ei, ei_end;
						for(boost::tie(ei, ei_end) = out_edges(u, g); ei != ei_end; ++ei) {
							cum += (double) (get(w, *ei)/total);
							if (cum >= r) {
								Vertex target = boost::target(*ei, g);
								if(!ns.count(target)) ns[target] = 0;
								ns[target]++;
								if(!runs_next.count(target)) runs_next[target] = 0;
								runs_next[target]++;
								fringe_next.insert(target);
								break;
							}
						}
					}
				}
				
				fringe.swap(fringe_next);
				fringe_next.clear();
				runs.swap(runs_next);
				runs_next.clear();
			}
		}	
	} // namespace detail
	
	// performs the edge separation process (strengthening intra-cluster edges and weakening inter-cluster edges)
	template <class Graph, class WeightMap>
	inline void
	harel_simple_random_walk_separation(Graph &g, WeightMap w, int passes, int k = 3)
	{
		typedef typename graph_traits<Graph>::vertex_descriptor vertex;
		typedef typename graph_traits<Graph>::edge_descriptor edge;
		typedef typename graph_traits<Graph>::vertex_iterator vertex_iterator;
		typedef typename graph_traits<Graph>::edge_iterator edge_iterator;
		typedef typename graph_traits<Graph>::directed_category directed;
		typedef typename property_traits<WeightMap>::value_type weight;
		typedef typename detail::harel_simple_random_walk_traits<Graph, WeightMap>::neighbor_similarity_map NSMap;
		
		BOOST_STATIC_ASSERT((is_same<directed, directed_tag>::value));
		function_requires< WritablePropertyMapConcept<WeightMap, edge> >();
		
		// process:
		//	- calculate the NS map for all vertices
		//	- use the NS map to populate the weight map again
		//	- do this <passes> times
		
		// this will cache our NSMaps
		std::map<vertex, NSMap> ns_cache;
//		boost::timer clock;
		
		boost::minstd_rand rand;
		rand.seed(static_cast<typename boost::minstd_rand::result_type>(time(0)));
		
		for(int pass = 0; pass < passes; pass++) {
			std::cout << "starting pass " << pass << std::endl;
			vertex_iterator vi, vend;
			boost::progress_display dsp(num_vertices(g), std::cout);

			for(tie(vi, vend) = vertices(g); vi != vend; ++vi) {
				detail::harel_simple_create_ns_map(g, w, *vi, k, 50, rand, ns_cache[*vi]);
				++dsp;
			}
			
			// re-populate the weight map with the new info
			edge_iterator ei, ei_end;
			for(boost::tie(ei, ei_end) = edges(g); ei != ei_end; ++ei) {
				vertex u, v;
				u = source(*ei, g);
				v = target(*ei, g);
				
				weight wu, wv;
				wu = ns_cache[u].count(v)?ns_cache[u][v]:0;
				wv = ns_cache[v].count(u)?ns_cache[v][u]:0;
				
				put(w, *ei, wu+wv); // add the two to ensure symmetry. averaging is unnecessary as it is all normalized later
			}
			
			std::cout << "pass " << pass << " complete." << std::endl;
			
			ns_cache.clear();
		}
	}
	
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
		typedef std::multimap<comp_type, comp_type> remap_type;
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
