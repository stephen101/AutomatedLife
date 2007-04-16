/*
random walk subgraph creation

attempts to choose a number (trials) of paths of statistically greatest "significance" on a dense graph
*/

#ifndef __SEMANTIC_SUBGRAPH_RANDOM_WALK_HPP__
#define __SEMANTIC_SUBGRAPH_RANDOM_WALK_HPP__

#include <semantic/semantic.hpp>
#include <boost/random/linear_congruential.hpp>
#include <time.h>
#include <set>
#include <vector>
#include <numeric>
//#include <algorithm>
#include <iterator>
#include <boost/iterator/transform_iterator.hpp>

//#include <iostream>

namespace semantic {
	template <class SEBase>
	class PruningRandomWalkSubgraph : public SEBase {
		typedef se_graph_traits<SEBase> traits;
		typedef typename traits::vertex_id_type id_type;
		
		public:
			typedef SEBase base_type;
			
			PruningRandomWalkSubgraph() : SEBase(), m_trials(100), m_depth(4), m_prune_keep(1.0) { // defaults
				// init with current time
				m_rand.seed(static_cast<typename boost::minstd_rand::result_type>(time(0)));
			}
			
			void set_depth(unsigned int d) { m_depth = d; }
			void set_trials(unsigned int t) { m_trials = t; }
			void set_seed(typename boost::minstd_rand::result_type s) { m_rand.seed(s); }
			void keep_only_top_edges(float f) { keep_within_range(f, 0.0f, 1.0f); m_prune_keep = f; }
			
			unsigned int get_depth() { return m_depth; }
			unsigned int get_trials() { return m_trials; }
			
			template <class Iterator, class WeightingPolicy>
			void want_vertices(Iterator i, Iterator i_end, WeightingPolicy w) {
				// set the graph's energy hit count
				set_property(*this, graph_energy_hits, get_trials());
				typedef weighting_traits<typename traits::storage_policy_selector, WeightingPolicy> wtraits;
				std::set<id_type> fringe;
				std::vector<typename traits::vertex_properties_type> vertex_list;
				std::map<id_type, unsigned int> v_runs; // for keeping track of how many "pathways" we compute for each node
				
				// first step - remove vertices from our list that we've already gotten
				std::set<id_type> m_set(i,i_end);
				set_difference(m_set.begin(), m_set.end(), m_fetched.begin(), m_fetched.end(), inserter(fringe, fringe.end()));
				
				// no vertices, no fetching, no work
				if (fringe.empty()) return;
				
				// add those vertices to our graph
				fetch_vertex_properties(fringe.begin(), fringe.end(), back_inserter(vertex_list));
				for(typename std::vector<typename traits::vertex_properties_type>::iterator it = vertex_list.begin(); it != vertex_list.end(); ++it) {
					add_vertex(*it, *this);
					v_runs[get_vertex_id(*it)] = m_trials;
					m_fetched.insert(get_vertex_id(*it));
				}
				
				// and begin the loop
				for(unsigned int depth = 0; depth < m_depth; depth++) {
					std::set<id_type> to_fetch, new_fringe;
					std::map<id_type, unsigned int> v_runs_next;
					// don't fetch neighbors we've already gotten
					set_difference(fringe.begin(), fringe.end(), 
						extract_first_iterator(m_neighbor_cache.begin()),
						extract_first_iterator(m_neighbor_cache.end()), inserter(to_fetch, to_fetch.end()));
					
					// fetch the others into the cache
					fetch_vertex_neighbors(to_fetch.begin(), to_fetch.end(), m_neighbor_cache);
										
					// go through our fringe and perform path extensions on the vertices we
					// have flagged for that process
					for(typename std::set<id_type>::iterator it = fringe.begin(); it != fringe.end(); ++it) {
						typename traits::vertex_descriptor u = vertex_by_id(*it);
						typename traits::neighbor_list &local_list = m_neighbor_cache[*it];
						// apply our weighting algorithm
						typename wtraits::id_weight_map weights;
						w.apply_weights(u, local_list, *this, boost::make_assoc_property_map(weights));
						
						// go through those we just fetched and prune out the extra edges (as denoted by m_prune_keep)
						prune_edges(local_list, weights, w);
						
						// get the total local weight so we can normalize
						typename WeightingPolicy::weight_type total =
							accumulate(extract_values(weights.begin()), extract_values(weights.end()), (typename WeightingPolicy::weight_type) 0);
						
						// get a random number and walk, v_runs[id_of(u)] times
						for (unsigned int k = 0; k < v_runs[*it]; k++) {
							// choose a random neighbor
							double rand = get_rand();
							double cumulative = 0;
							for(typename traits::neighbor_list::iterator ni = local_list.begin(); ni != local_list.end(); ++ni) {
								typename traits::edge_properties_type ep;
								typename traits::vertex_properties_type vp;
								boost::tie(ep, vp) = *ni;
								id_type vertex_id = get_vertex_id(vp);
								cumulative += weights[vertex_id]/total;
								
								if (cumulative >= rand) {
									// add this vertex and the edge to the graph
									if (!v_runs_next.count(vertex_id)) {
										// no point in doing this twice
										typename traits::vertex_descriptor v = add_vertex(vp, *this);
										add_edge(u, v, ep, *this);
										new_fringe.insert(vertex_id);
										v_runs_next[vertex_id] = 0;
									}
									
									// on our next fringe run, we're going to create another path
									// using this node
									v_runs_next[vertex_id]++;
									
									// if this edge or the opposite dont yet exist, create it
									if (!either_edge(u, vertex_by_id(vertex_id), *this).second) {
										add_edge(u, vertex_by_id(vertex_id), ep, *this);
									}
									
									// increment the energy hits on the edge between u and vertex_id
									(*this)[either_edge(u, vertex_by_id(vertex_id), *this).first].energy_hits++;
									break; // and go to the next one!
								}
							}
						}
					}
					
					// swap the fringes
					fringe.swap(new_fringe);
					v_runs.swap(v_runs_next);
				}
				
				// get the leaf edges into the cache
				std::set<id_type> to_fetch;
				set_difference(fringe.begin(), fringe.end(), 
					extract_first_iterator(m_neighbor_cache.begin()),
					extract_first_iterator(m_neighbor_cache.end()), inserter(to_fetch, to_fetch.end()));
				fetch_vertex_neighbors(to_fetch.begin(), to_fetch.end(), m_neighbor_cache);
				
				// go through our neighbor cache (edges) and create any edges between vertices
				// that we have in our graph that don't already exist
				for(
					typename traits::mapped_neighbor_list::iterator it = m_neighbor_cache.begin();
					it != m_neighbor_cache.end(); ++it) {
					id_type u_id, v_id;
					typename traits::vertex_descriptor u, v;
					u_id = (*it).first;
					u = vertex_by_id(u_id);
					for(typename traits::neighbor_list::iterator ni = (*it).second.begin(); ni != (*it).second.end(); ++ni) {
						v_id = get_vertex_id((*ni).second);
						// if this vertex is also in our neighbor cache it is in the graph, so add the edge
						// if it doesn't already exist
						if (m_neighbor_cache.count(v_id)) {
							v = vertex_by_id(v_id);
							if (!(edge(u, v, *this).second)) {
								add_edge(u, v, (*ni).first, *this);
							}
						}
					}
				}
			}
			
			void did_clear() {
			    SEBase::did_clear();
				m_fetched.clear();
				m_neighbor_cache.clear();
			}
		
		private:
			template <class WeightingPolicy>
			void prune_edges(typename traits::neighbor_list &list, typename weighting_traits<typename traits::storage_policy_selector, WeightingPolicy>::id_weight_map &weights, WeightingPolicy) {
				typedef weighting_traits<typename traits::storage_policy_selector, WeightingPolicy> wtraits;
				typedef typename WeightingPolicy::weight_type weight_type;
				typedef std::multimap<weight_type, unsigned long, std::greater<weight_type> > sorted_weights_map;
				typename traits::neighbor_list new_list;
				typename wtraits::id_weight_map new_weights;
				typename WeightingPolicy::weight_type total = 0, running_total = 0;
				sorted_weights_map sorted_weights;
				
				// get the total local weight so we can normalize
				total = accumulate(extract_values(weights.begin()), extract_values(weights.end()), total);
				
				// add the weights all into the sorted_weights so we can take the biggest ones
				for(unsigned long i = 0; i < list.size(); i++) {
					sorted_weights.insert(std::make_pair(weights[get_vertex_id(list[i].second)], i));
				}
				
				// now go through and add our edges/vertices and weights that we want to keep into their respective containers
				for(typename sorted_weights_map::iterator i = sorted_weights.begin(); i != sorted_weights.end(); ++i) {
					running_total += i->first / total;
					new_list.push_back(list[i->second]);
					new_weights[get_vertex_id(list[i->second].second)] = i->first;
					
					if (running_total >= m_prune_keep) break; // only run until we hit threshold or we're done anyway
				}
				
				list.swap(new_list);
				weights.swap(new_weights);
				// done
			}
		
			double get_rand() {
				double r = static_cast<double>(m_rand() + minstd_rand::min_value)/(minstd_rand::max_value - minstd_rand::min_value);
				return r;
			}
			
			unsigned int m_trials;
			unsigned int m_depth;
			boost::minstd_rand m_rand;
			float m_prune_keep;
			
			std::set<id_type> m_fetched;
			typename traits::mapped_neighbor_list m_neighbor_cache;
	};
}

#endif
