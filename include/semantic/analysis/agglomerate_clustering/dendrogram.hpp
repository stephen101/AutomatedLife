#ifndef __SEMANTIC_ANALYSIS_DENDROGRAM_HPP__
#define __SEMANTIC_ANALYSIS_DENDROGRAM_HPP__

#include <boost/pending/disjoint_sets.hpp>

namespace semantic {
	
	class SingleLinkDistanceCalculator {
		public:
		template <class I>
		typename I::value_type operator () (I i, I end) {
			typename I::value_type min = (std::numeric_limits<typename I::value_type>::max)();
			for(; i != end; ++i) {
				if (*i < min) min = *i;
			}
			
			return min;
		}
	};
	class SingleLinkageDistanceCalculator : public SingleLinkDistanceCalculator {};
	
	template <class Graph>
	class dendrogram {
		typedef se_graph_traits<Graph> 					Traits;
		typedef typename Traits::vertex_descriptor 		vertex;
		typedef typename Traits::vertices_size_type		vertices_size_type;
		typedef std::pair<vertex, vertex>				vertex_pair;
		typedef std::map<vertex, vertex> 				pred_map_type;
		typedef std::map<vertex, 
			typename Traits::vertices_size_type> 		rank_map_type;

		typedef boost::disjoint_sets<
			boost::associative_property_map
				<rank_map_type>,
			boost::associative_property_map
				<pred_map_type> > 						disjoint_sets_type;
		
		public:
			dendrogram(Graph &g) : 
						m_base(boost::make_assoc_property_map(m_base_rank), boost::make_assoc_property_map(m_base_pred)),
						m_current(boost::make_assoc_property_map(m_current_rank), boost::make_assoc_property_map(m_current_pred)),
						m_current_num_links(0) 
			{
				m_num_vertices = num_vertices(g);
				// populate our base and our running set
				typename se_graph_traits<Graph>::vertex_iterator vi, vi_end;
				for(boost::tie(vi,vi_end) = vertices(g); vi != vi_end; ++vi) {
					m_base.make_set(*vi);
					m_current.make_set(*vi);
				}
			}
			
			void set_num_clusters(vertices_size_type num_clusters) {
				vertices_size_type links_needed = m_num_vertices - num_clusters;
				if (links_needed <= 0) return; // do nothing
				
				// if our current number of links is < links_needed, we're in good shape
				// otherwise re-create m_current
				if (m_current_num_links > links_needed) {
					m_current_num_links = 0;
					m_current_rank = rank_map_type(m_base_rank);
					m_current_pred = pred_map_type(m_base_pred);
					m_current = disjoint_sets_type(
						boost::make_assoc_property_map(m_current_rank), 
						boost::make_assoc_property_map(m_current_pred));
				}
				
				// perform the linking operations
				while(m_current_num_links < links_needed) {
					// if (m_current_num_links >= m_links.size()) std::cerr << "too big!!! " << m_current_num_links << " " << m_links.size() << " " << links_needed << std::endl;
					m_current.link(m_links[m_current_num_links].first, m_links[m_current_num_links].second);
					
					m_current_num_links++;
				}
				
				m_current.compress_sets(extract_keys(m_base_rank.begin()), extract_keys(m_base_rank.end())); // make sure our parent map is compressed
			}
			
			template <class OutputIterator>
			vertices_size_type get_clusters(OutputIterator out) {
				std::map<vertex, vertices_size_type> vc;
				vertices_size_type count = 0;
				
				for(typename pred_map_type::iterator i = m_current_pred.begin(); i != m_current_pred.end(); ++i)
				{
					vertex p = i->second;
					if (!vc.count(p)) vc[p] = count++;
					*out++ = std::make_pair<vertex, vertices_size_type>(i->first, vc[p]);
				}
				
				return count;
			}
			
			template <class OutputIterator>
			void get_vertices_in_cluster_with(vertex u, OutputIterator out) {
				vertex p = m_current.find_set(u);
				
				for(typename pred_map_type::iterator i = m_current_pred.begin(); i != m_current_pred.end(); ++i)
				{
					if (i->second == p) *out++ = i->first;
				}
			}
			
			void add_link(vertex u, vertex v) {
				m_links.push_back(vertex_pair(u,v));
				
				m_current.link(u, v);
				m_current_num_links++;
			}
			
			vertex get_rep_for(vertex u) {
				return m_current.find_set(u);
			}
			
		private:
			rank_map_type					m_base_rank, m_current_rank;
			pred_map_type					m_base_pred, m_current_pred;
		
			disjoint_sets_type 				m_base, 
											m_current;
			
			vertices_size_type				m_num_vertices,
											m_current_num_links;
											
			std::vector<vertex_pair>		m_links;
	}; // class dendrogram
	
	namespace detail {
	template <class Graph, class WeightMap, class DistanceCalculator, template <class> class Compare>
	class dendrogram_helper {
		typedef dendrogram<Graph> dendrogram_type;
		typedef typename boost::property_traits<WeightMap>::value_type weight;
		typedef se_graph_traits<Graph> Traits;
		typedef typename Traits::vertex_descriptor vertex;
		typedef std::pair<vertex, vertex> edge;
		typedef Compare<weight> comparison_type;
		typedef std::multimap<weight, edge, comparison_type> priority_map_type;
		typedef typename priority_map_type::iterator map_item;
		// typedef std::multimap<vertex, map_item> edge_map_type;
		// typedef typename edge_map_type::iterator edge_map_item;
		
		public:
			template <class EdgeIterator>
			dendrogram_helper(Graph &g, dendrogram_type &d, EdgeIterator i, EdgeIterator i_end, WeightMap weights, DistanceCalculator calc) : m_g(g), m_dendrogram(d), new_weight_calc(calc) {
				// go through edges i -> i_end and add their weights to the priority map
				for(; i != i_end; ++i) {
					vertex u = source(*i, g);
					vertex v = target(*i, g);
					weight w = weights[*i];
					edge the_edge(u, v);
//					m_edge_map.insert(std::make_pair(u, m_priority_map.insert(std::make_pair(w, edge))));
					m_priority_map.insert(std::make_pair(w, the_edge));
					// std::cerr << "--> new edge in priority map: " << the_edge.first << "-" << the_edge.second << " " << w << std::endl;
				}
			}
			
			edge get_top() {
//				std::cerr << "top is " << m_g[m_priority_map.begin()->second.first].content << "-" << m_g[m_priority_map.begin()->second.second].content << " with " << m_priority_map.begin()->first << std::endl;
				return m_priority_map.begin()->second;
			}
			
			void link(vertex u, vertex v) {
				// std::cerr << "linking " << u << " and " << v << std::endl;
				// std::cerr << "\tmap size is " << m_priority_map.size() << std::endl;
				// steps:
				//	- check what vertices u and v are "connected" to, save them
				//	- clear the lists of edges from u or v
				//	- calculate the new distances from the new u/v combo to the lists of u and v's old edges
				typedef std::multimap<vertex, weight> save_set;
				std::set<vertex> to_consider;
				save_set save;
				// edge_map_item i, i_end;
				
				// save & clear
				for(map_item i = m_priority_map.begin(); i != m_priority_map.end();) {
					map_item next = i;
					++next;
					if (i->second.first == u || i->second.first == v) {
						save.insert(std::make_pair(i->second.second, i->first));
						to_consider.insert(i->second.second);
						// std::cerr << "\t" << "erasing " << i->second.first << "-" << i->second.second << std::endl;
						m_priority_map.erase(i);
					} else if (i->second.second == u || i->second.second == v) {
						save.insert(std::make_pair(i->second.first, i->first));
						to_consider.insert(i->second.first);
						// std::cerr << "\t" << "erasing " << i->second.first << "-" << i->second.second << std::endl;
						m_priority_map.erase(i);
					}
					
					i = next;
				}
				
				// put this in our dendrogram
				m_dendrogram.add_link(u, v);
				vertex new_rep = m_dendrogram.get_rep_for(u); // either u or v will work here as they are now in
																// the same component

				// do new weights & edges
				// our new cluster which contains representatives u and v will be connected to each of the
				// representatives in the set "save", ignoring of course links between u and v themselves.
				for(typename std::set<vertex>::iterator i = to_consider.begin(); i != to_consider.end(); ++i) {
					vertex t = *i;
					if (t == u || t == v) continue; // don't consider a link between the source cluster and itself
					
					typename save_set::iterator wi, wi_end;
					boost::tie(wi, wi_end) = save.equal_range(t);
					
					weight new_weight = new_weight_calc(extract_values(wi), extract_values(wi_end));
					
					m_priority_map.insert(std::make_pair(new_weight, edge(new_rep, t))); // done!
				}
				
				// std::cerr << "\tmap size is " << m_priority_map.size() << std::endl;
				
			}
			
			void build() {
				// loop through our priority list and create links
				while(m_priority_map.size() > 0) {
					edge e = get_top();
					link(e.first, e.second);
				}
			}
			
		private:
		
			Graph &				m_g;
			priority_map_type 	m_priority_map;
			// edge_map_type 		m_edge_map;
			dendrogram_type	&	m_dendrogram;
			DistanceCalculator	new_weight_calc;
	}; // class dendrogram_helper
	} // namespace detail
	
	template <
		class Graph, 
		class MST, 
		class WeightMap, 
		class Dendrogram, 
		class DistanceCalculator>
	void dendrogram_from_distance_mst(Graph &g, MST &mst, WeightMap w, Dendrogram &out, DistanceCalculator dist = DistanceCalculator())
	{
		typedef se_graph_traits<Graph> Traits;
		typedef typename Traits::vertex_descriptor vertex;
		typedef typename boost::property_traits<WeightMap>::value_type weight;

		// create the dendrogram helper, and have the helper build the dendrogram
		typedef detail::dendrogram_helper<Graph, WeightMap, DistanceCalculator, std::less> helper;
	
		helper builder(g, out, mst.begin(), mst.end(), w, dist);
		builder.build();
	}
	
	template <
		class Graph,
		class MST,
		class WeightMap,
		class Dendrogram,
		class DistanceCalculator>
	void dendrogram_from_similarity_mst(Graph &g, MST &mst, WeightMap w, Dendrogram &out, DistanceCalculator dist = DistanceCalculator())
	{
		typedef se_graph_traits<Graph> Traits;
		typedef typename Traits::vertex_descriptor vertex;
		typedef typename boost::property_traits<WeightMap>::value_type weight;

		// create the dendrogram helper, and have the helper build the dendrogram
		typedef detail::dendrogram_helper<Graph, WeightMap, DistanceCalculator, std::greater> helper;
	
		helper builder(g, out, mst.begin(), mst.end(), w, dist);
		builder.build();
	}
} // namespace semantic


#endif
