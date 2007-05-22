// defines the Storage Policy concept and provides constraints

#ifndef __SEMANTIC_STORAGE_CONCEPT_HPP__
#define __SEMANTIC_STORAGE_CONCEPT_HPP__

#include <semantic/properties.hpp>

namespace semantic {

	template <class StoragePolicySelector>
	class StoragePolicyConcept {
		typedef StoragePolicy<StoragePolicySelector>	ThePolicy;
		typedef se_graph_traits<StoragePolicySelector>	Traits;
		public:
			public constraints() {
				p.set_mirror_changes_to_storage(b);
				b = p.get_mirror_changes_to_storage();
				
				p.commit_changes_to_storage();
				
				boost::tie(b, u) = p.will_add_vertex(vp);
				p.did_add_vertex(u, vp);
				
				p.will_remove_vertex(u, vp);
				p.did_remove_vertex(vp);
				
				b = p.will_add_edge(u, v, ep);
				p.did_add_edge(e1, b, u, v, ep);
				
				p.will_remove_edge(u, v);
				p.did_remove_edge(u, v);
				
				p.will_clear_vertex(u);
				p.did_clear_vertex(u);
				
				p.will_clear();
				p.did_clear();
				
				u = p.vertex_by_id(id);
				p.vertices_by_id(id, id, id);	// all template arguments (iterators)
				
				id = p.get_vertex_id(u);
				id = p.get_vertex_id(vp);
				
				id = p.fetch_vertex_id_by_content_and_type(s, i);
				
				vs = p.get_vertex_count_of_type(i);
				
				b = p.fetch_vertex_properties(id, vp);
				b = p.fetch_vertex_properties(id, id, id);	// all template arguments (iterators)
				
				b = p.fetch_vertex_neighbors(id, id); 		// 2nd argument is template (iterator)
				b = p.fetch_vertex_neighbors(id, id, id);	// 1 & 2 = iterators, 3 = neighbor map
				
				p.get_collections_list(id);					// argument = iterator
				p.set_meta_value(s, s);
				s = p.get_meta_value(s, s);
				
				p.set_vertex_meta_value(u, s, s);
				s = p.get_vertex_meta_value(u, s);
				
				b = p.populate_full_graph();
				
				p.rename_collection(s, s);
				p.remove_collection(s);
				p.reset_collection();
				p.reset_all_collections();
				
				p.mark_as_dirty(e1);
				p.mark_as_dirty(u);
			}
		private:
			ThePolicy p;
			typename Traits::vertex_descriptor u, v;
			typename Traits::edge_descriptor e1, e2;
			typename Traits::vertex_properties_type vp;
			typename Traits::edge_properties_type ep;
			typename Traits::vertex_id_type id;
			
			bool b;
			int i;
			std::string s;
			typename Traits::vertices_size_type vs;
	}; // class StoragePolicyConcept

	
} // namespace semantic

#endif
