/*
a small convenience include containing the classes/structs pertaining
to se_graph properties
*/

#ifndef __SEMANTIC_PROPERTIES_HPP__
#define __SEMANTIC_PROPERTIES_HPP__

#include <string>
#include <vector>
#include <map>
#include <boost/graph/adjacency_list.hpp>
#include <semantic/utility.hpp>

#include <boost/functional/hash.hpp>
#ifdef HAVE_TR1_UNORDERED_MAP
#include <tr1/unordered_set> // for our edge weight map hash
#endif

// if you want to use the hash as our list type for the graph, uncomment this
// #define USE_HASH_IN_GRAPH

using namespace boost;


enum graph_energy_hits_t { graph_energy_hits };
namespace boost {
	BOOST_INSTALL_PROPERTY(graph, energy_hits);
} // namespace boost

struct tr1_hash_setS {};

namespace boost {
    template <class ValueType>
    struct container_gen<tr1_hash_setS, ValueType> {
        #ifdef HAVE_TR1_UNORDERED_MAP
        typedef std::tr1::unordered_set<ValueType, boost::hash<ValueType> > type;
        #else
        typedef typename container_gen<setS, ValueType>::type type;
        #endif
    };

    template <>
    struct parallel_edge_traits<tr1_hash_setS> {
        typedef disallow_parallel_edge_tag type;
    };
}

namespace semantic {
	// our node types
	enum node_type_major {
		node_type_major_undef,
		node_type_major_doc,
		node_type_major_term
	};
	enum node_type_minor {
		node_type_minor_undef,
		node_type_minor_term,
		node_type_minor_phrase
	};
	
	// undefined template class to define a storage policy's methods
	template <class StoragePolicySelector, class SEBase> class StoragePolicy;
	
	// undefined template struct to define a storage policy's traits
	template <class StoragePolicySelector> struct se_storage_traits;
	
	// defines many of the same things as storage_traits, but also gives us our base_graph type
	template <class StoragePolicySelector>
	struct se_graph_traits {
		typedef StoragePolicySelector storage_policy_selector;
		typedef se_storage_traits<StoragePolicySelector> storage_traits;
		// these are custom types from our StoragePolicy
		typedef typename storage_traits::vertex_id_type vertex_id_type;
		typedef typename storage_traits::vertex_properties_type vertex_properties_type;
		typedef typename storage_traits::edge_properties_type edge_properties_type;
		// convenience types for interfacing with storage policy
		typedef std::vector< std::pair< edge_properties_type, vertex_properties_type > > neighbor_list;
		typedef maps::unordered< vertex_id_type, neighbor_list> mapped_neighbor_list;
		
		typedef adjacency_list<
#ifdef USE_HASH_IN_GRAPH
			tr1_hash_setS, listS, directedS,
#else
            setS, listS, directedS,
#endif
			typename storage_traits::vertex_properties_type,
			typename storage_traits::edge_properties_type,
			property<graph_name_t, std::string, property<graph_energy_hits_t, int> >
		> base_graph_type;
		
		typedef graph_traits<base_graph_type> 				graph_traits;
		// these mirror the BOOST graph_traits adjacency_list assoc. types
		typedef typename graph_traits::vertex_descriptor 	vertex_descriptor;
		typedef typename graph_traits::edge_descriptor 		edge_descriptor;
		typedef typename graph_traits::vertex_iterator 		vertex_iterator;
		typedef typename graph_traits::edge_iterator 		edge_iterator;
		typedef typename graph_traits::out_edge_iterator 	out_edge_iterator;
		typedef typename graph_traits::adjacency_iterator 	adjacency_iterator;
		typedef typename graph_traits::directed_category 	directed_category;
		typedef typename graph_traits::edge_parallel_category edge_parallel_category;
		typedef typename graph_traits::vertices_size_type 	vertices_size_type;
		typedef typename graph_traits::edges_size_type 		edges_size_type;
		typedef typename graph_traits::degree_size_type 	degree_size_type;
		
		// these define the vertex_pair_edge types
		// BOOST supplies helper functions to deal with edges as pairs
		typedef std::pair<vertex_descriptor, vertex_descriptor> vertex_pair_edge;
	};	
	
	
	// weighting traits allows for quicker generation of weightmaps and the like
	template <class S, class W = empty_class> struct weighting_traits;
	
	template <class S, class W>
	struct weighting_traits {
		typedef W weighting_policy;
		typedef typename W::weight_type weight_type;
		typedef maps::unordered<
			typename se_graph_traits<S>::edge_descriptor,
			weight_type> edge_weight_map;
		
		typedef maps::unordered<
			typename se_graph_traits<S>::vertex_id_type,
			weight_type> id_weight_map;
		
		typedef maps::unordered<
			typename se_graph_traits<S>::vertex_descriptor,
			weight_type> vertex_weight_map;
	};
		
	// defines the very very basic BOOST graph structure we use. this should never actually be used
#ifdef USE_HASH_IN_GRAPH
	typedef adjacency_list<tr1_hash_setS, listS, directedS> se_basic_boost_graph_type;
#else
	typedef adjacency_list<setS, listS, directedS> se_basic_boost_graph_type;
#endif
	
	template <template <class, class> class Graph, class StoragePolicySelector, class SEBase>
	struct se_graph_traits<Graph<StoragePolicySelector, SEBase> > : public se_graph_traits<StoragePolicySelector> {};
	
	// basic struct for vertex properties
	struct vertex_properties {
		int type_major;
		int type_minor;
		std::string content;
		
		vertex_properties() : type_major(node_type_major_undef), type_minor(node_type_minor_undef), content("") {}
	};
	
	// basic struct for edge properties
	struct edge_properties {
		int strength;
		unsigned long from_degree;
		unsigned long to_degree;
		int energy_hits;
		
		edge_properties() : strength(0), from_degree(0), to_degree(0), energy_hits(0) {}
	};
	
} // namespace semantic

namespace boost {
	inline std::size_t hash_value(const boost::detail::edge_desc_impl<boost::directed_tag, void*> &e)
	{
		std::size_t res = 0;
		boost::hash_combine(res, e.m_source);
		boost::hash_combine(res, e.m_target);
		return res;
	}
} // namespace boost

namespace std {
    inline bool operator< (const boost::detail::edge_desc_impl<boost::directed_tag, void*> &e1,
                            const boost::detail::edge_desc_impl<boost::directed_tag, void*> &e2)
    {
    	if (e1.m_source < e2.m_source) return true;
    	if (e1.m_source == e2.m_source) {
    		if (e1.m_target < e2.m_target) return true;
    	}
    	return false;
    }

    inline bool operator== (const boost::detail::edge_desc_impl<boost::directed_tag, void*> &e1,
                             const boost::detail::edge_desc_impl<boost::directed_tag, void*> &e2)
    {
    	return (e1.m_source == e2.m_source && e1.m_target == e2.m_target);	    
    }
} // namespace std

#endif
