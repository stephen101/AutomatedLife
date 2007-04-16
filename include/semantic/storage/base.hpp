// this class provides empty implementations of the optional
// methods in the Storage Policy Concept

#ifndef __SEMANTIC_STORAGE_BASE_HPP__
#define __SEMANTIC_STORAGE_BASE_HPP__

#include <semantic/properties.hpp>

namespace semantic {
	template <class StoragePolicySelector>
	class StoragePolicyBase {
		typedef se_graph_traits<StoragePolicySelector> traits;
		typedef typename traits::base_graph_type base_graph_type;
		typedef typename traits::vertex_descriptor Vertex;
		typedef typename traits::edge_descriptor Edge;

		typedef typename traits::vertex_properties_type vertex_properties;
		typedef typename traits::edge_properties_type edge_properties;
		typedef typename traits::vertex_id_type id_type;
	
		public:
			std::pair<bool, Vertex> will_add_vertex(const vertex_properties &) { return std::make_pair(true, Vertex()); }	
			void did_add_vertex(Vertex, const vertex_properties &) {}
			void will_remove_vertex(Vertex, const vertex_properties &) {}
			void did_remove_vertex(const vertex_properties &) {}
			bool will_add_edge(Vertex, Vertex, const edge_properties &) { return true; }
			void did_add_edge(Edge, bool, Vertex, Vertex, const edge_properties &) {}
			void will_remove_edge(Vertex, Vertex) {}
			void did_remove_edge(Vertex, Vertex) {}
			void will_clear_vertex(Vertex) {}
			void did_clear_vertex(Vertex) {}
			void will_clear() {}
			void did_clear() {}
		
	}; // class StoragePolicyBase	
} // namespace semantic


#endif
