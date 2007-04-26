/*
empty (or memory only) storage policy.

this follows the storage policy description found in docs/storage/policy.txt
*/

#include <map>
#include <semantic/properties.hpp>

#ifndef __SEMANTIC_STORAGE_NONE_HPP__
#define __SEMANTIC_STORAGE_NONE_HPP__

namespace semantic {
	struct NoStoragePolicy;
	
	template <> struct se_storage_traits<NoStoragePolicy> {
		// we don't have an internal index type - use the BOOST vertex descriptor
		typedef boost::graph_traits<se_basic_boost_graph_type>::vertex_descriptor vertex_id_type; 
		typedef vertex_properties vertex_properties_type; // use the base properties struct
		typedef edge_properties edge_properties_type;	  // ditto
	};
	
	template <class SEBase>
	class StoragePolicy<NoStoragePolicy, SEBase> : public SEBase {
		typedef se_graph_traits<NoStoragePolicy> traits;
		typedef traits::base_graph_type base_graph_type;
		typedef traits::vertex_descriptor Vertex;
		typedef traits::edge_descriptor Edge;

		typedef se_storage_traits<NoStoragePolicy> storage_traits;
		typedef storage_traits::vertex_properties_type vertex_properties;
		typedef storage_traits::edge_properties_type edge_properties;
		typedef storage_traits::vertex_id_type id_type;
		
		public:
			typedef SEBase base_type;
			
			StoragePolicy() : mirror_flag(false) {}
		
			// methods having to do directly with this storage policy implementation
			void set_mirror_changes_to_storage(bool b) { if (!b) { adjust_edge_degrees(); count_cache.clear(); } mirror_flag = b; }
			bool get_mirror_changes_to_storage() { return mirror_flag; }
			void commit_changes_to_storage() {} // do nothing
			
			// methods having to do with structure alteration to the graph
			std::pair<bool, Vertex> will_add_vertex(const vertex_properties &vp) {return std::make_pair(true, Vertex());}
			void did_add_vertex(Vertex v, const vertex_properties &vp) {}
			
			void will_remove_vertex(Vertex v, const vertex_properties &vp) {}
			void did_remove_vertex(const vertex_properties &vp) {}
			
			bool will_add_edge(Vertex u, Vertex v, const edge_properties &ep) { return true; }
			void did_add_edge(Edge e, bool did, Vertex u, Vertex v, const edge_properties &ep) {}
			
			void will_remove_edge(Vertex u, Vertex v) {}
			void did_remove_edge(Vertex u, Vertex v) {}
			
			void will_clear_vertex(Vertex u) {}
			void did_clear_vertex(Vertex u) {}
			
			void will_clear() {}
			void did_clear() {}
			
			// methods for fetching graph contents
			Vertex vertex_by_id(id_type id) { return id; }
			
			id_type get_vertex_id(const Vertex u) const { return u; }
			id_type get_vertex_id(const vertex_properties &p) const { return NULL; }
			
			
			traits::vertices_size_type get_vertex_count_of_type(int node_type) {
				if (count_cache.count(node_type)) return count_cache[node_type];
				
				traits::vertices_size_type cnt = 0;
				traits::vertex_iterator vi, vi_end;
				for(boost::tie(vi, vi_end) = vertices(*this); vi != vi_end; ++vi) {
					if ((*this)[*vi].type_major == node_type) cnt++;
				}
				
				count_cache.insert(std::make_pair(node_type, cnt));
				return cnt;
			}
			
			void rename_collection(std::string, std::string) {}
			void remove_collection(std::string) {}
			void reset_collection(){}
			
		protected:
			bool mirror_flag;
			
			void adjust_edge_degrees() {
				traits::edge_iterator ei, ei_end;
				for(boost::tie(ei, ei_end) = edges(*this); ei != ei_end; ++ei) {
					(*this)[*ei].from_degree = out_degree(source(*ei, *this), *this);
					(*this)[*ei].to_degree = out_degree(target(*ei, *this), *this);
				}
			}
		
		private:
			std::map<int, traits::vertices_size_type> count_cache;
	};
} // namespace semantic

#endif
