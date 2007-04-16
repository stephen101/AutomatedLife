/*
provides an empty SubgraphPolicy implementation.

this is useful if you are working with a SEGraph that will only reside in memory
for the duration of a single process
*/

#ifndef __SEMANTIC_SUBGRAPH_BFS_HPP__
#define __SEMANTIC_SUBGRAPH_BFS_HPP__

#include <set>

namespace semantic {
	template <class SEBase>
	class BFSSubgraph : public SEBase {
		typedef typename se_graph_traits<SEBase>::vertex_id_type id_type;
		
		public:
			typedef SEBase base_type;
			
			BFSSubgraph() : SEBase(), m_depth(4) {}
			
			void set_depth(unsigned int d) { m_depth = d; }
			unsigned int get_depth() { return m_depth; }
			
			// methods in the concept/policy
			template <class Iterator, class WeightingPolicy>
			void want_vertices(Iterator i, Iterator i_end, WeightingPolicy w) {
				// set the "energy hits" to 1 for BFS
				set_property(*this, graph_energy_hits, 1);
				
				typedef typename std::set<id_type>::iterator it_type;
				std::set<id_type> fringe;
				// do not fetch vertices which we've already handled
				std::set<id_type> m_set(i,i_end);
				set_difference(m_set.begin(), m_set.end(), m_fetched.begin(), m_fetched.end(), inserter(fringe, fringe.end()));
				
				// return if we have nothing to fetch
				if (fringe.empty()) return;
				
				// first add the vertices requested
				std::vector<typename se_graph_traits<SEBase>::vertex_properties_type> vertices;
				fetch_vertex_properties(fringe.begin(), fringe.end(), back_inserter(vertices));
				for(int i = 0; i < vertices.size(); i++) add_vertex(vertices[i], *this);
				
				if (m_depth == 0) return;
				
				for(unsigned int i = 0; i < m_depth; i++) {
					std::set<id_type> new_fringe;
					// fetch the neighbors of our fringe from the storage policy
					typename se_graph_traits<SEBase>::mapped_neighbor_list neighbor_list;
					// this method is part of our StoragePolicy
					fetch_vertex_neighbors(fringe.begin(), fringe.end(), neighbor_list);
					
					// add them all to our graph, with the edges
					for(it_type it = fringe.begin(); it != fringe.end(); ++it) {
						typename se_graph_traits<SEBase>::neighbor_list local_list = neighbor_list[*it];
						typename se_graph_traits<SEBase>::vertex_descriptor u = SEBase::vertex_by_id(*it);
						// we now have a local list of neighbors (consisting of std::pair<edge_properties, vertex_properties>)
						for(typename se_graph_traits<SEBase>::neighbor_list::iterator lit = local_list.begin();
						 		lit != local_list.end(); ++lit) {
							typename se_graph_traits<SEBase>::edge_properties_type ep = (*lit).first;
							typename se_graph_traits<SEBase>::vertex_properties_type vp = (*lit).second;
							
							// set energy hits to 1
							ep.energy_hits = 1;
							
							// add the vertex and then the edge between vertex (*it) and this vertex
							typename se_graph_traits<SEBase>::vertex_descriptor v = add_vertex(vp, *this);
							// add the edge
							add_edge(u, v, ep, *this);
							
							new_fringe.insert(get_vertex_id(vp));
						}
						
						// consider this vertex "fetched"!
						m_fetched.insert(*it);
					}
					
					// swap the fringes, ignoring any vertices we've handled before.
					fringe.clear();
					set_difference(new_fringe.begin(), new_fringe.end(), m_fetched.begin(), m_fetched.end(), inserter(fringe, fringe.end()));
				}
			}
			
			void will_clear() {
			    SEBase::will_clear();         // StoragePolicy::will_clear();
				m_fetched.clear();
			}
		
		private:
			unsigned int m_depth;
			std::set<id_type> m_fetched;
	};
}

#endif
