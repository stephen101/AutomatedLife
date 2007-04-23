
#ifndef _SEMANTIC_HPP_
#define _SEMANTIC_HPP_


#include <semantic/config.hpp>
#include <semantic/storage/none.hpp>
#include <semantic/properties.hpp>

#include <boost/graph/iteration_macros.hpp>


using std::string;
using std::vector;
using std::set;
using namespace boost;
using namespace semantic;

namespace semantic {

    
    template <
        class StoragePolicySelector = NoStoragePolicy,
        class SEBase = StoragePolicy<
            StoragePolicySelector,
            typename se_graph_traits<StoragePolicySelector>::base_graph_type >
        > class SEGraph : public SEBase
    {
        typedef SEGraph<StoragePolicySelector, SEBase> self_type;
        typedef se_storage_traits<StoragePolicySelector> storage_traits;
        typedef se_graph_traits<StoragePolicySelector> se_traits;
    
        public:
            
            typedef SEBase base_type;
            typedef typename se_traits::base_graph_type base_graph_type;
            typedef StoragePolicySelector storage_policy_selector;
            typedef base_type storage_policy_type;
            
            SEGraph() { SEGraph(""); }
            explicit SEGraph(string name)
                : base_type(), __refcount(0)
            {
                storage_policy_type::set_mirror_changes_to_storage(false);
                set_property(*this, graph_name, name);
                set_property(*this, graph_energy_hits, 1);
            }
            
            typename storage_traits::vertex_properties_type &
            operator [] (typename se_traits::vertex_descriptor v) {
                return (get(vertex_bundle, *this))[v];
            }
            const typename storage_traits::vertex_properties_type &
            operator [] (typename se_traits::vertex_descriptor v) const {
                return (get(vertex_bundle, *this))[v];
            }
            typename storage_traits::edge_properties_type &
            operator [] (typename se_traits::edge_descriptor e) {
                return (get(edge_bundle, *this))[e];
            }
            const typename storage_traits::edge_properties_type &
            operator [] (typename se_traits::edge_descriptor e) const {
                return (get(edge_bundle, *this))[e];
            }
            
            std::string collection() const {
                return get_property(*this, graph_name);
            }
            
            void set_collection(string name){
                clear();
                set_property(*this, graph_name, name);
                set_property(*this, graph_energy_hits, 1);
            }
            
            template <class WeightingPolicy, class WeightMap>
            void populate_weight_map(WeightingPolicy weighting, WeightMap w) {
                typedef typename WeightingPolicy::weight_type weight_type;
                typedef typename se_traits::edge_descriptor Edge;
                typedef typename se_traits::vertex_iterator iterator;
            
                // make sure we were passed the right kind of WeightMap
                function_requires< WritablePropertyMapConcept<WeightMap, Edge> >();
                BOOST_STATIC_ASSERT((
                    boost::is_same<typename boost::property_traits<WeightMap>::value_type,
                    weight_type>::value
                ));
            
                // go through all the vertices and apply our weighting algorithm
                iterator vi, vi_end;
                for(boost::tie(vi, vi_end) = vertices(*this); vi != vi_end; ++vi) {
                    weighting.apply_weights(*vi, *this, w);
                }
            }
            
            void clear() {
                storage_policy_type::will_clear();
                base_type::clear();
                storage_policy_type::did_clear();
            }
            
            template <class Functor>
            void apply_to_graph(Functor f) {
                f(*this);
            }
            
            template <class Functor>
            void apply_to_all_vertices(Functor f) {
                BGL_FORALL_VERTICES_T(v, (*this), base_graph_type) f(v, *this);
            }
            
            template <class Functor>
            void apply_to_all_edges(Functor f) {
                BGL_FORALL_EDGES_T(e, (*this), base_graph_type) f(e, *this);
            }
            unsigned int __refcount;
    };
    

    
    template <class StoragePolicySelector>
    class index_manager : public StoragePolicy<StoragePolicySelector, empty_class>
    {
        typedef StoragePolicy<StoragePolicySelector, empty_class> SEBase;
        public:
            std::vector<std::string> list_collections() {
                std::vector<std::string> collections;
                SEBase::get_collections_list(back_inserter(collections));
    
                return collections;
            }
    
            void rename_collection(std::string oldname, std::string newname) {
                SEBase::rename_collection(oldname, newname);
            }
    
            void remove_collection(std::string name) {
                SEBase::remove_collection(name);
            }
    }; // index_manager
    
    template <class S>
    class StorageInfoQuery : public index_manager<S> {};
    

} // namespace semantic

namespace boost {
    
    template <class StoragePolicySelector, class SEBase>
    inline typename se_graph_traits<StoragePolicySelector>::vertex_descriptor
    add_vertex(const typename se_storage_traits<StoragePolicySelector>::vertex_properties_type &p,
               SEGraph<StoragePolicySelector, SEBase> &g)
    {
        typedef typename se_graph_traits<StoragePolicySelector>::base_graph_type base_type;
        typedef typename graph_traits<base_type>::vertex_descriptor Vertex;
        typedef typename se_storage_traits<
                              StoragePolicySelector
                            >::vertex_properties_type vertex_properties_type;
    
        base_type& base_g = static_cast<base_type&>(g);
    
        Vertex v;
        bool should_add;
    
        boost::tie(should_add, v) = g.will_add_vertex(p);
        // should we add this vertex?
        if (should_add) {
            v = boost::add_vertex(p, base_g);
            g.did_add_vertex(v, p);
        }
    
        return v;
    }
    
    // THIS FUNCTION SHOULD NOT BE CALLED! USE add_vertex(properties, graph) INSTEAD
    template <class StoragePolicySelector, class SEBase>
    inline void
    add_vertex(SEGraph<StoragePolicySelector, SEBase> &g)
    {
        struct DoNotUseThisMethodException {};
        throw DoNotUseThisMethodException();
    }
    
    template <class StoragePolicySelector, class SEBase>
    inline std::pair<typename se_graph_traits<StoragePolicySelector>::edge_descriptor, bool>
    add_edge(
        typename se_graph_traits<StoragePolicySelector>::vertex_descriptor u,
        typename se_graph_traits<StoragePolicySelector>::vertex_descriptor v,
        const typename se_storage_traits<StoragePolicySelector>::edge_properties_type &p,
        SEGraph<StoragePolicySelector, SEBase> &g)
    {
        typedef se_graph_traits<StoragePolicySelector> graph_traits;
        typedef typename graph_traits::vertex_descriptor Vertex;
        typedef typename graph_traits::edge_descriptor Edge;
        typedef typename graph_traits::base_graph_type base_type;
    
        base_type& base_g = static_cast<base_type&>(g);
        std::pair<Edge, bool> edge_result;
    
        if (g.will_add_edge(u, v, p)) {
            edge_result = boost::add_edge(u, v, p, base_g);
            // if it already existed, update the properties
            if (edge_result.second == false) {
                g[edge_result.first] = p;
            }
    
            g.did_add_edge(edge_result.first, edge_result.second, u, v, p);
        } else {
            edge_result = std::make_pair(Edge(), false);
        }
    
        return edge_result;
    }
    
    // THIS FUNCTION SHOULD NOT BE CALLED! USE add_edge(vertex, vertex, properties, graph) INSTEAD
    template <class StoragePolicySelector, class SEBase>
    inline std::pair<typename se_graph_traits<StoragePolicySelector>::edge_descriptor, bool>
    add_edge(
        typename se_graph_traits<StoragePolicySelector>::vertex_descriptor u,
        typename se_graph_traits<StoragePolicySelector>::vertex_descriptor v,
        SEGraph<StoragePolicySelector, SEBase> &g)
    {
        struct DoNotUseThisMethodException {};
        throw DoNotUseThisMethodException();
    }
    
    template <class StoragePolicySelector, class SEBase>
    inline std::pair<typename se_graph_traits<StoragePolicySelector>::edge_descriptor, bool>
    add_or_strengthen_edge(
        typename se_graph_traits<StoragePolicySelector>::vertex_descriptor u,
        typename se_graph_traits<StoragePolicySelector>::vertex_descriptor v,
        const typename se_storage_traits<StoragePolicySelector>::edge_properties_type &p,
        SEGraph<StoragePolicySelector, SEBase> &g)
    {
        typedef se_graph_traits<StoragePolicySelector> graph_traits;
        typedef typename graph_traits::vertex_descriptor Vertex;
        typedef typename graph_traits::edge_descriptor Edge;
        typedef typename graph_traits::base_graph_type base_type;
    
        base_type& base_g = static_cast<base_type&>(g);
        std::pair<Edge, bool> edge_result;
    
        if (g.will_add_edge(u, v, p)) {
            edge_result = boost::add_edge(u, v, p, base_g);
            // if it already exists, strengthen the edge
            if (edge_result.second == false) {
                typename graph_traits::edge_properties_type old_p = g[edge_result.first];
                g[edge_result.first] = p;
                g[edge_result.first].strength += old_p.strength;
            }
    
            g.did_add_edge(edge_result.first, edge_result.second, u, v, g[edge_result.first]);
        } else {
            edge_result = std::make_pair(Edge(), false);
        }
    
        return edge_result;
    }
    
    template <class StoragePolicySelector, class SEBase>
    inline void
    remove_edge(
        typename se_graph_traits<StoragePolicySelector>::vertex_descriptor u,
        typename se_graph_traits<StoragePolicySelector>::vertex_descriptor v,
        SEGraph<StoragePolicySelector, SEBase> &g)
    {
        typedef se_graph_traits<StoragePolicySelector> graph_traits;
        typedef typename graph_traits::edge_descriptor Edge;
        typedef typename graph_traits::base_graph_type base_type;
    
        base_type& base_g = static_cast<base_type&>(g);
    
        g.will_remove_edge(u, v);
        boost::remove_edge(u, v, base_g);
        g.did_remove_edge(u, v);
    }
    
    template <class StoragePolicySelector, class SEBase>
    inline void
    remove_edge(
        typename se_graph_traits<StoragePolicySelector>::edge_descriptor e,
        SEGraph<StoragePolicySelector, SEBase> &g)
    {
        typedef se_graph_traits<StoragePolicySelector> graph_traits;
        typedef typename graph_traits::vertex_descriptor Vertex;
        typedef typename graph_traits::edge_descriptor Edge;
        typedef typename graph_traits::base_graph_type base_type;
    
        base_type& base_g = static_cast<base_type&>(g);
    
        Vertex u,v;
        u = boost::source(e, base_g);
        v = boost::target(e, base_g);
    
        g.will_remove_edge(u, v);
        boost::remove_edge(e, base_g);
        g.did_remove_edge(u, v);
    }
    
    template <class StoragePolicySelector, class SEBase>
    inline void
    remove_edge(
        typename se_graph_traits<StoragePolicySelector>::out_edge_iterator i,
        SEGraph<StoragePolicySelector, SEBase>& g)
    {
        typedef se_graph_traits<StoragePolicySelector> graph_traits;
        typedef typename graph_traits::vertex_descriptor Vertex;
        typedef typename graph_traits::edge_descriptor Edge;
        typedef typename graph_traits::base_graph_type base_type;
    
        Edge e = *i;
        base_type& base_g = static_cast<base_type&>(g);
    
        Vertex u,v;
        u = boost::source(e, base_g);
        v = boost::target(e, base_g);
    
        g.will_remove_edge(u, v);
        boost::remove_edge(i, base_g);
        g.did_remove_edge(u, v);
    }
    
    template <class StoragePolicySelector, class SEBase, class Predicate>
    inline void
    remove_out_edge_if(
        typename se_graph_traits<StoragePolicySelector>::vertex_descriptor u,
        Predicate predicate,
        SEGraph<StoragePolicySelector, SEBase> &g)
    {
        typedef se_graph_traits<StoragePolicySelector> graph_traits;
        typedef typename graph_traits::vertex_descriptor Vertex;
        typedef typename graph_traits::edge_descriptor Edge;
        typedef typename graph_traits::base_graph_type base_type;
        typedef typename graph_traits::out_edge_iterator iterator;
    
        base_type& base_g = static_cast<base_type&>(g);
    
        std::vector<Edge> to_remove;
        iterator ei, ei_end;
        for(boost::tie(ei, ei_end) = boost::out_edges(u, base_g); ei != ei_end; ++ei) {
            if (predicate(*ei)) to_remove.push_back(*ei);
        }
    
        for(typename graph_traits::edges_size_type i = 0; i < to_remove.size(); i++) {
            Vertex v;
            v = boost::target(to_remove[i], base_g);
            g.will_remove_edge(u, v);
            boost::remove_edge(to_remove[i], base_g);
            g.did_remove_edge(u, v);
        }
    }
    
    template <class StoragePolicySelector, class SEBase, class Predicate>
    inline void
    remove_in_edge_if(
        typename se_graph_traits<StoragePolicySelector>::vertex_descriptor,
        Predicate, SEGraph<StoragePolicySelector, SEBase>&)
    {
        struct DoNotUseThisMethodException {};
        throw DoNotUseThisMethodException();
    }
    
    template <class StoragePolicySelector, class SEBase, class Predicate>
    inline void
    remove_edge_if(
        Predicate predicate,
        SEGraph<StoragePolicySelector, SEBase> &g)
    {
        typedef se_graph_traits<StoragePolicySelector> graph_traits;
        typedef typename graph_traits::vertex_descriptor Vertex;
        typedef typename graph_traits::edge_descriptor Edge;
        typedef typename graph_traits::base_graph_type base_type;
        typedef typename graph_traits::edge_iterator iterator;
    
        base_type& base_g = static_cast<base_type&>(g);
    
        std::vector<Edge> to_remove;
        iterator ei, ei_end;
        for(boost::tie(ei, ei_end) = boost::edges(base_g); ei != ei_end; ++ei) {
            if (predicate(*ei)) to_remove.push_back(*ei);
        }
    
        for(typename graph_traits::edges_size_type i = 0; i < to_remove.size(); i++) {
            Vertex u, v;
            u = boost::source(to_remove[i], base_g);
            v = boost::target(to_remove[i], base_g);
            g.will_remove_edge(u, v);
            boost::remove_edge(to_remove[i], base_g);
            g.did_remove_edge(u, v);
        }
    }
    
    template <class StoragePolicySelector, class SEBase>
    inline void
    clear_vertex(
        typename se_graph_traits<StoragePolicySelector>::vertex_descriptor u,
        SEGraph<StoragePolicySelector, SEBase> &g)
    {
        typedef se_graph_traits<StoragePolicySelector> graph_traits;
        typedef typename graph_traits::base_graph_type base_type;
    
        base_type& base_g = static_cast<base_type&>(g);
        g.will_clear_vertex(u);
        boost::clear_vertex(u, base_g);
        g.did_clear_vertex(u);
    }
    
    template <class StoragePolicySelector, class SEBase>
    inline void
    clear_out_edges(
        typename se_graph_traits<StoragePolicySelector>::vertex_descriptor u,
        SEGraph<StoragePolicySelector, SEBase> &g)
    {
        typedef se_graph_traits<StoragePolicySelector> graph_traits;
        typedef typename graph_traits::vertex_descriptor Vertex;
        typedef typename graph_traits::edge_descriptor Edge;
        typedef typename graph_traits::base_graph_type base_type;
        typedef typename graph_traits::out_edge_iterator iterator;
    
        base_type& base_g = static_cast<base_type&>(g);
    
        iterator ei, ei_end;
        boost::tie(ei, ei_end) = boost::out_edges(u, base_g);
        std::vector<Edge> to_remove(ei, ei_end);
    
        for(typename graph_traits::edges_size_type i = 0; i < to_remove.size(); i++) {
            Vertex v;
            v = boost::target(to_remove[i], base_g);
            g.will_remove_edge(u, v);
            boost::remove_edge(to_remove[i], base_g);
            g.did_remove_edge(u, v);
        }
    }
    
    template <class StoragePolicySelector, class SEBase>
    inline void
    clear_in_edges(
        typename se_graph_traits<StoragePolicySelector>::vertex_descriptor,
        SEGraph<StoragePolicySelector, SEBase>&)
    {
        struct DoNotUseThisMethodException {};
        throw DoNotUseThisMethodException();
    }
    
    template <class StoragePolicySelector, class SEBase>
    inline void
    remove_vertex(
        typename se_graph_traits<StoragePolicySelector>::vertex_descriptor u,
        SEGraph<StoragePolicySelector, SEBase> &g)
    {
        typedef se_graph_traits<StoragePolicySelector> graph_traits;
        typedef typename graph_traits::base_graph_type base_type;
    
        base_type& base_g = static_cast<base_type&>(g);
        typename se_storage_traits<StoragePolicySelector>::vertex_properties_type vp = g[u];
    
        g.will_remove_vertex(u, vp);
        boost::remove_vertex(u, base_g);
        g.did_remove_vertex(vp);
    }
    
    template <class StoragePolicySelector, class SEBase>
    inline void
    merge_vertices(
        typename se_graph_traits<StoragePolicySelector>::vertex_descriptor u,
        typename se_graph_traits<StoragePolicySelector>::vertex_descriptor v,
        SEGraph<StoragePolicySelector, SEBase> &g)
    {
        typedef se_graph_traits<StoragePolicySelector> graph_traits;
        typedef typename graph_traits::base_graph_type base_type;
    
        base_type& base_g = static_cast<base_type&>(g);
    
        // step one: take all the edges from #2 and run into #1 (this is slow...)
        typename graph_traits::edge_iterator ei, ei_end;
        for(boost::tie(ei, ei_end) = edges(g); ei != ei_end; ++ei) {
            if (source(*ei, g) == v || target(*ei, g) == v) {
                // copy the edge to u
                typename graph_traits::edge_properties_type &ep = g[*ei], nep;
                nep.strength = ep.strength;
    
                if (source(*ei, g) == v) {
                    add_edge(u, target(*ei, g), nep, g);
                } else {
                    add_edge(source(*ei, g), u, nep, g);
                }
            }
        }
    
        // now remove the old vertex
        remove_vertex(v, g);
    }
    
    template <class StoragePolicySelector, class SEBase>
    inline std::pair<typename se_graph_traits<StoragePolicySelector>::edge_descriptor, bool>
    either_edge(
        typename se_graph_traits<StoragePolicySelector>::vertex_descriptor u,
        typename se_graph_traits<StoragePolicySelector>::vertex_descriptor v,
        SEGraph<StoragePolicySelector, SEBase> &g)
    {
        typedef se_graph_traits<StoragePolicySelector> graph_traits;
        typedef typename graph_traits::edge_descriptor edge_desc;
    
        std::pair<edge_desc, bool> edge_exists = edge(u, v, g);
        if (edge_exists.second) return edge_exists;
        return edge(v, u, g);
    }
    
} // namespace boost


template<class S, class B>
std::ostream & operator<<(std::ostream &o, const SEGraph<S,B> &g) {
    o << "graph: " << get_property(g, graph_name)
                   << " (v:" << num_vertices(g) << " e:"
                   << num_edges(g) << ")" << std::endl;
    typename se_graph_traits<S>::vertex_iterator vi, vi_end;
    for(boost::tie(vi, vi_end) = vertices(g); vi != vi_end; ++vi) {
        typename se_graph_traits<S>::adjacency_iterator ai, ai_end;
        o << g[*vi].content << "(" << g[*vi].type_major << "): ";
        for(boost::tie(ai, ai_end) = adjacent_vertices(*vi, g); ai != ai_end; ++ai) {
            o << g[*ai].content << "(" << g[*ai].type_major << ") ";
        }
        o << std::endl;
    }
    return o;
}



namespace boost {
    template <class S, class B>
    inline void intrusive_ptr_add_ref(SEGraph<S,B> * p) {
        ++(p->__refcount);
    }
    template <class S, class B>
    inline void intrusive_ptr_release(SEGraph<S,B> * p) {
        if (--(p->__refcount) == 0)
            delete p;
    }
} // namespace boost


#endif /* _SEMANTIC_HPP_ */

