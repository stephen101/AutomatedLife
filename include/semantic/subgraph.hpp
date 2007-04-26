/*
This class provides subgraph access to an SEGraph object. 

The advantage of using this class is:

	1) it takes a sub-graph creation policy, which allows you to take a large graph stored out of memory
	and extract only the parts of it that you need into memory
	
This class should be used for the "searching" portion of the Semantic Engine
*/

#ifndef __SEMANTIC_SUBGRAPH_HPP__
#define __SEMANTIC_SUBGRAPH_HPP__

#include <semantic/semantic.hpp>
#include <semantic/weighting/none.hpp>
#include <semantic/utility.hpp>
#include <string>
#include <map>

//#include <iostream>

namespace semantic {
	
	template <
		class StoragePolicySelector, 
		template <class> class SubgraphPolicy, 
		class WeightingPolicy = NoWeighting
	>
	class SESubgraph :
		public SubgraphPolicy<SEGraph<StoragePolicySelector> >
	{
		typedef SubgraphPolicy<SEGraph<StoragePolicySelector> > SEBase;
		typedef se_storage_traits<StoragePolicySelector> storage_traits;
		typedef typename SEBase::base_type::base_type storage_policy;
		typedef typename SEBase::base_type base_graph_type;
		typedef se_graph_traits<StoragePolicySelector> se_traits;
		
		public:
			typedef SEBase base_type;
			typedef WeightingPolicy weighting_policy;

			SESubgraph() : SESubgraph("") {}
			explicit SESubgraph(const std::string name) : base_type() {
			    storage_policy::set_mirror_changes_to_storage(false);
                set_property(*this, graph_name, name);
                set_property(*this, graph_energy_hits, 1);
			}
			
			void fetch_subgraph_starting_from(typename se_traits::vertex_id_type id) {				
				// tell the subgraph policy we're about to get this vertex
				want_vertices(&id, (&id)+1, weighting);
			}
			
			template <class Iterator>
			void fetch_subgraph_starting_from(Iterator i, Iterator i_end) {
				// tell the subgraph policy we're about to get all these vertices
				want_vertices(i, i_end, weighting);
			}
			
			void expand_vertex(typename se_traits::vertex_id_type id) {
				expand_vertices(&id, (&id) + 1);
			}
			
			template <class Iterator>
			std::map<typename se_traits::vertex_id_type,unsigned int> get_intersection(Iterator i,Iterator i_end){
				typedef std::map<typename se_traits::vertex_id_type, unsigned int> Nodes;
				typedef std::vector<typename se_traits::vertex_id_type> vertices;
				Nodes nodes;
				for( ; i != i_end; ++i){
					typename se_traits::mapped_neighbor_list neighbors;
					//typename se_traits::vertex_id_type v = *i;
					fetch_vertex_neighbors(i,i_end, neighbors );
					typename se_traits::neighbor_list::iterator ni;
					for( ni = neighbors[*i].begin(); ni != neighbors[*i].end(); ++ni){
						typename se_traits::edge_properties_type ep;
						typename se_traits::vertex_properties_type vp;
						boost::tie(ep, vp) = *ni;
						typename se_traits::vertex_id_type id = get_vertex_id(vp);
						if( id ){
							nodes[id]++;
						}
					}
				}
				Nodes intersection;
				for( typename Nodes::iterator iter = nodes.begin(); iter != nodes.end(); ++iter ){
					if( iter->second > 1 ){
						intersection[iter->first] = iter->second;
					}
				}
				return intersection;
			}
			
			template <class Iterator>
			void fetch_neighbors(Iterator i,Iterator i_end, typename se_traits::mapped_neighbor_list neighbors){
				fetch_vertex_neighbors(i,i_end,neighbors);
			}
			
			template <class Iterator>
			void expand_vertices(Iterator i, Iterator i_end) {
				// fetch the vertices
				typename se_traits::mapped_neighbor_list neighbors;
				
				std::vector<typename se_traits::vertex_properties_type> vertex_list;
				fetch_vertex_properties(i, i_end, back_inserter(vertex_list));
				for(typename std::vector<typename se_traits::vertex_properties_type>::iterator it = vertex_list.begin(); it != vertex_list.end(); ++it) {
					add_vertex(*it, *this);
				}
				
				fetch_vertex_neighbors(i, i_end, neighbors);
				
				for(; i != i_end; ++i) {
					typename se_traits::neighbor_list::iterator ni;
					typename se_traits::vertex_descriptor u = vertex_by_id(*i);
					for(ni = neighbors[*i].begin(); ni != neighbors[*i].end(); ++ni) {
						typename se_traits::edge_properties_type ep;
						typename se_traits::vertex_properties_type vp;
						boost::tie(ep, vp) = *ni;
						
						ep.energy_hits = 1;
						
						// add the new edge/vertex
						typename se_traits::vertex_descriptor v = add_vertex(vp, *this);
						add_edge(u, v, ep, *this);
					}
				}
			}
			
			std::string unstem_term(const std::string &stem){
				typename se_traits::vertex_descriptor u;
				u = storage_policy::vertex_by_id(
	                     storage_policy::fetch_vertex_id_by_content_and_type(
	                         stem, node_type_major_term));

				std::string term = storage_policy::get_vertex_meta_value(u, "term");
	            std::string::size_type pos = term.find_last_of(":");
				if( pos != std::string::npos && pos > 0 ){
					return term.substr(0,pos);
				}
				return stem;
			}
			
			
			template <class WeightMap>
            void populate_weight_map(WeightMap w) {
                // forward this to our superclass's implementation
                base_graph_type::populate_weight_map(weighting, w);
            }
			
			void clear() {
				base_graph_type::clear();   // SEGraph::clear()
				SEBase::did_clear();          // SubgraphPolicy::did_clear()
			}
			
		private:
			WeightingPolicy weighting;
	};
	
	// convenience
	template<class StoragePolicySelector, template <class> class SubgraphPolicy, class WeightingPolicy>
	struct se_graph_traits<SESubgraph<StoragePolicySelector, SubgraphPolicy, WeightingPolicy> > : public se_graph_traits<StoragePolicySelector> {};
	
	// weighting traits!
	template <class StoragePolicySelector, template <class> class SubgraphPolicy, class WeightingPolicy>
	struct weighting_traits<SESubgraph<StoragePolicySelector, SubgraphPolicy, WeightingPolicy>, empty_class> : public weighting_traits<StoragePolicySelector, WeightingPolicy> {};
		
/*	// and again for subgraph policies
	template<template <class> class SubgraphPolicy, template <class, class> class Graph, class StoragePolicySelector, class SEBase>
	struct se_graph_traits<SubgraphPolicy<Graph<StoragePolicySelector, SEBase> > > : public se_graph_traits<StoragePolicySelector> {};
*/	
} // namespace semantic

#endif
