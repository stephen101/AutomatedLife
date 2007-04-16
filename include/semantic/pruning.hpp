/*
this files provides some basic pruning operations
*/

#ifndef __SEMANTIC_PRUNING_HPP__
#define __SEMANTIC_PRUNING_HPP__

#include <semantic/semantic.hpp>
#include <semantic/properties.hpp>
#include <boost/graph/properties.hpp>
#include <vector>
#include <map>
#include <set>

#include <iostream>

namespace semantic {
	
	// ensures bidirectionality in our graph
	namespace detail {
		template<class WeightMap> class weight_copier { // this class will only ever be
											   // parsed if have_w = true
			public:
			template <class Edge>
			void operator() (Edge e, Edge n, WeightMap w) const {
				put(w, n, get(w, e)); // done!
			}
		};
		template<> class weight_copier<empty_class> {
			public:
			template <class Edge, class WeightMap>
			void operator() (Edge e, Edge n, WeightMap w) const {}
		};
	} // namespace detail
	
	template <class WeightMap = empty_class>
	class ensure_bidirectionality {
		public:
			explicit ensure_bidirectionality() {}
			explicit ensure_bidirectionality(WeightMap w) : w(w) {}
		
			template <class Edge, class Graph>
			void operator() (Edge e, Graph &g) {
				// check if we have an edge going the other direction
				if (!edge(target(e, g), source(e, g), g).second) {
					// add a new edge
					typename se_graph_traits<Graph>::edge_properties_type ep(g[e]); // copy
					int tmp = ep.to_degree;
					ep.to_degree = ep.from_degree; ep.from_degree = tmp; // swap
					
					Edge n = add_edge(target(e, g), source(e, g), ep, g).first;
					
					// if we have a weight map, copy the weight
					copier(e, n, w);
				}
			}
		private:
			WeightMap w;
			bool have_w;
			detail::weight_copier<WeightMap> copier;
	}; // class ensure_bidirecionality
		
	// ensures that our graph has symmetrical edges (weight-wise)
	template <class Graph, class WeightMap>
	inline void ensure_graph_symmetry(Graph &g, WeightMap w) {
		// go through the graph's edges and if an opposite edge exists,
		// average the two weights
		std::set<typename se_graph_traits<Graph>::edge_descriptor> seen;
		typename se_graph_traits<Graph>::edge_iterator ei, ei_end;
		for(boost::tie(ei, ei_end) = edges(g); ei != ei_end; ++ei) {
			if (seen.count(*ei)) continue;
			if (edge(target(*ei, g), source(*ei, g), g).second) {
				typename se_graph_traits<Graph>::edge_descriptor e = edge(target(*ei, g), source(*ei, g), g).first;
				if (seen.count(e)) continue;
				
				// use geometric mean (sqrt(a*b)) versus old-timer mean (a+b)/2
				put(w, *ei, sqrt(get(w, *ei) * get(w, e)));
				put(w, e, get(w, *ei));
			}
		}
		
		// lastly, ensure bidirectionality
		g.apply_to_all_edges(ensure_bidirectionality<WeightMap>(w));
	}
	
	// keeps the top n percent of edges for each vertex passed
	template <class WeightMap>
	class keep_top_n_percent_of_edges {
		typedef typename property_traits<WeightMap>::value_type weight;
		public:
			explicit keep_top_n_percent_of_edges(float percent, WeightMap weights) : p(percent), w(weights) {}
		
			template <class Vertex, class Graph>
			void operator() (Vertex u, Graph &g) {
				typedef typename boost::graph_traits<Graph>::out_edge_iterator iterator;
				typedef typename boost::graph_traits<Graph>::edge_descriptor edge;
				typedef typename boost::graph_traits<Graph>::degree_size_type degree;
				typedef typename boost::graph_traits<Graph>::vertices_size_type vsize;			
			
				if (p >= 1) return;
				// can't do this -- would remove in-edges, too
//				if (p <= 0) {clear_vertex(u, g);return;}
				
				// begin iterating through the vertices,
				// and remove the bottom 1 - p percent of them
				std::vector<edge> to_remove;
				iterator ei, eend;
			
				degree d = out_degree(u, g);
				// how many edges should we remove?
				degree rem = (degree) floor((1-p) * d);
			
				if (rem > 0) {

					// create a multi-map to the edges based on their weight so we can quickly choose the
					// edges with least weight
					std::multimap<weight, edge> weight_map;

					for(boost::tie(ei, eend) = out_edges(u,g); ei != eend; ++ei) {
						weight_map.insert(std::pair<weight, edge>(get(w, *ei), *ei));
					}

					// begin reducing the degree
					for(; rem > 0; rem--) {
						weight the_weight;
						edge the_edge;
						typename std::multimap<weight, edge>::iterator least = weight_map.begin();
						boost::tie(the_weight, the_edge) = *least;

						// remove this entry from the map
						weight_map.erase(least);
					
						// put it in our list of edges to remove
						to_remove.push_back(the_edge);
					}
				}
			
				// remove all the edges
				for(vsize i = 0; i < to_remove.size(); i++) {
					remove_edge(to_remove[i], g);
				}
			
				// done.
			}
			
		private:
			float p;
			WeightMap w;
	}; // class keep_top_n_percent_of_edges
	
	// keeps the top n percent of edges for each vertex passed (based on edge weight)
	template <class WeightMap>
	class keep_top_n_percent_of_edges_weighted {
		typedef typename property_traits<WeightMap>::value_type weight;
		public:
			explicit keep_top_n_percent_of_edges_weighted(float percent, WeightMap weights) : p(percent), w(weights) {}

			template <class Vertex, class Graph>
			void operator() (Vertex u, Graph &g) {
				typedef typename boost::graph_traits<Graph>::out_edge_iterator iterator;
				typedef typename boost::graph_traits<Graph>::edge_descriptor edge;
				typedef typename boost::graph_traits<Graph>::degree_size_type degree;
				typedef typename boost::graph_traits<Graph>::vertices_size_type vsize;			

				if (p >= 1) return;

				// begin iterating through the vertices
				std::vector<edge> to_remove;
				iterator ei, eend;

				// to keep track of our total weight
				weight running_total = 0;

				// create a multi-map to the edges based on their weight so we can quickly choose the
				// edges with least weight
				typedef std::multimap<weight, edge, std::greater<weight> > my_map;
				my_map weight_map;
				weight total = 0;

				for(boost::tie(ei, eend) = out_edges(u,g); ei != eend; ++ei) {
					weight wt = get(w, *ei);
					weight_map.insert(std::pair<weight, edge>(wt, *ei));
					total += wt;
				}

				// begin reducing the degree
				for(typename my_map::iterator i = weight_map.begin(); i != weight_map.end(); ++i) {
					weight the_weight;
					edge the_edge;
					boost::tie(the_weight, the_edge) = *i;
					
					if (running_total >= p) to_remove.push_back(the_edge);
					
					running_total += the_weight/total;
				}

				// remove all the edges
				for(vsize i = 0; i < to_remove.size(); i++) {
					remove_edge(to_remove[i], g);
				}

				// done.
			}

		private:
			float p;
			WeightMap w;
	}; // class keep_top_n_percent_of_edges_weighted
	
	// convenience function
	template <class Graph, class WeightMap>
	void keep_top_n_percent_of_graph_edges(Graph &g, float percent, WeightMap w) {
		g.apply_to_all_vertices(keep_top_n_percent_of_edges<WeightMap>(percent, w));
	}
	
	// another one
	template <class Graph, class WeightMap>
	void keep_top_n_percent_of_graph_edges_weighted(Graph &g, float percent, WeightMap w) {
		g.apply_to_all_vertices(keep_top_n_percent_of_edges_weighted<WeightMap>(percent, w));
	}

	// keeps the top n edges for each vertex passed
	// otherwise referred to as Locally Linear Embedding
	template <class WeightMap>
	class keep_top_n_edges {
		typedef typename property_traits<WeightMap>::value_type weight;
		public:
			explicit keep_top_n_edges(int num, WeightMap weights) : n(num), w(weights) {}
		
			template <class Vertex, class Graph>
			void operator() (Vertex u, Graph &g) {
				typedef typename boost::graph_traits<Graph>::out_edge_iterator iterator;
				typedef typename boost::graph_traits<Graph>::edge_descriptor edge;
				typedef typename boost::graph_traits<Graph>::degree_size_type degree;
				typedef typename boost::graph_traits<Graph>::vertices_size_type vsize;			
							
				// begin iterating through the vertices,
				// and remove the bottom d - n
				std::vector<edge> to_remove;
				iterator ei, eend;
			
				degree d = out_degree(u, g);
			
				if (d >= n) {
					// how many edges should we remove?
					degree rem = d - n;

					// create a multi-map to the edges based on their weight so we can quickly choose the
					// edges with least weight
					std::multimap<weight, edge> weight_map;

					for(boost::tie(ei, eend) = out_edges(u,g); ei != eend; ++ei) {
						weight_map.insert(std::pair<weight, edge>(get(w, *ei), *ei));
					}

					// begin reducing the degree
					weight lowest_weight = (std::numeric_limits<weight>::max)();
					edge lowest_edge;
					for(; rem > 0; rem--) {
						weight the_weight;
						edge the_edge;
						typename std::multimap<weight, edge>::iterator least = weight_map.begin();
						boost::tie(the_weight, the_edge) = *least;

						// remove this entry from the map
						weight_map.erase(least);
					
						// put it in our list of edges to remove
						to_remove.push_back(the_edge);
					}
				}
			
				// remove all the edges
				for(vsize i = 0; i < to_remove.size(); i++) {
					remove_edge(to_remove[i], g);
				}
			
				// done.
			}
			
		private:
			int n;
			WeightMap w;
	}; // class keep_top_n_edges
	
	// removes any edges whos strength is less than the passed threshold
	class remove_low_strength_edges {
		public:
			explicit remove_low_strength_edges(int t) : m_threshold(t) {}
			
			template <class Vertex, class Graph>
			void operator() (Vertex u, Graph &g) {
				typedef typename se_graph_traits<Graph>::out_edge_iterator iterator;
				typedef typename se_graph_traits<Graph>::edge_descriptor edge;
				
				std::vector<edge> to_remove;
				iterator ei, ei_end;
				for(boost::tie(ei, ei_end) = out_edges(u, g); ei != ei_end; ++ei) {
					if (g[*ei].strength < m_threshold) to_remove.push_back(*ei);
				}
				
				for(typename std::vector<edge>::iterator i = to_remove.begin(); i != to_remove.end(); ++i) {
					remove_edge(*i, g);
				}
			}
		private:
			int m_threshold;
	}; // class remove_low_strength_edges

	// removes any nodes which do not contain an edge of strength with passed threshold
	class remove_low_strength_vertices {
		public:
			explicit remove_low_strength_vertices(int t) : m_threshold(t) {}
			
			template <class Graph>
			void operator() (Graph &g) {
				typename se_graph_traits<Graph>::vertex_iterator vi, vi_end;
				typedef std::set<typename se_graph_traits<Graph>::vertex_descriptor> my_set;
				my_set to_remove;
				
				// go through the graph, remove any vertices that are connected
				// by edges all weaker than m_threshold
				for(boost::tie(vi, vi_end) = vertices(g); vi != vi_end; ++vi) {
					bool found = false;
					typename se_graph_traits<Graph>::out_edge_iterator ei, ei_end;
					for(boost::tie(ei, ei_end) = out_edges(*vi, g); ei != ei_end; ++ei) {
						if (g[*ei].strength >= m_threshold) { found = true; break; }
					}
					if (!found) to_remove.insert(*vi);
				}
				
				// now remove all the in-edges to any of these vertices
				typename se_graph_traits<Graph>::edge_iterator ei, ei_end;
				typedef std::vector<typename se_graph_traits<Graph>::edge_descriptor> my_vector;
				my_vector to_remove_edges;
				for(boost::tie(ei, ei_end) = edges(g); ei != ei_end; ++ei) {
					if (to_remove.count(target(*ei, g))) to_remove_edges.push_back(*ei);
				}
				
				// remove in-edges
				for(typename my_vector::iterator i = to_remove_edges.begin(); i != to_remove_edges.end(); ++i) {
					remove_edge(*i, g);
				}
				
				// remove vertices
				for(typename my_set::iterator i = to_remove.begin(); i != to_remove.end(); ++i) {
					remove_vertex(*i, g);
				}
			}
		private:
			int m_threshold;
	}; // class remove_low_strength_vertices
	
	// removes nodes that are too well connected or not well enough connected
	// *** this assumes a bi-partite graph, or at least one where all out edges
	// are connected to the same node type for a given vertex
	class remove_extremes {
		public:
			remove_extremes(int min = 3, double max = 0.1) : min_connectivity(min), max_factor(max) {}
			
			template <class Graph>
			void operator() (Graph &g) {
				typedef typename boost::graph_traits<Graph>::vertex_iterator iterator;
				typedef typename boost::graph_traits<Graph>::vertex_descriptor vertex;
				typedef typename boost::graph_traits<Graph>::edge_descriptor edge;
				typedef typename boost::graph_traits<Graph>::degree_size_type degree;
				typedef typename boost::graph_traits<Graph>::vertices_size_type vsize;
				
				vsize count = num_vertices(g);
				std::set<vertex> to_remove;
				iterator vi, vi_end;
				for(boost::tie(vi, vi_end) = vertices(g); vi != vi_end; ++vi) {
					// is this vertex not well enough connected?
					if (out_degree(*vi, g) <= min_connectivity) {
						to_remove.insert(*vi);
						continue; // to avoid adding a vertex twice?
					}
					
					// is this vertex too well connected?
					if (out_degree(*vi, g) > 0 &&
							out_degree(*vi, g)
								>
							max_factor * g.get_vertex_count_of_type(g[*(adjacent_vertices(*vi, g).first)].type_major)) {
						to_remove.insert(*vi);
					}
				}
								
				// to speed things up, we're going to handle deleting the in-edges
				// for these vertices in one go, versus calling clear_vertex() on
				// each one
				std::vector<edge> to_remove_edge;
				typename boost::graph_traits<Graph>::edge_iterator ei, ei_end;
				for(boost::tie(ei, ei_end) = edges(g); ei != ei_end; ++ei) {
					if (to_remove.count(target(*ei, g))) to_remove_edge.push_back(*ei);
				}
				
				for(unsigned long i = 0; i < to_remove_edge.size(); i++) {
					remove_edge(to_remove_edge[i], g);
				}
				
				for(typename std::set<vertex>::iterator i = to_remove.begin(); i != to_remove.end(); ++i) {
//					clear_vertex(*i, g);
					remove_vertex(*i, g);
				}
			}
		private:
			int min_connectivity;
			double max_factor;
	}; // class remove_extremes
	
} // namespace semantic

#endif
