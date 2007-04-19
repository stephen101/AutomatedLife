
#ifndef _SPREADING_ACTIVATION_HPP_
#define _SPREADING_ACTIVATION_HPP_


#include <semantic/semantic.hpp>
#include <semantic/utility.hpp>

#include <map>
#include <set>
#include <iterator>


namespace semantic {
    namespace detail {
        
        template <class Graph, class WeightMap, class RankMap>
        void activation_spider(
                Graph &g,
                typename se_graph_traits<Graph>::vertex_descriptor u,
                WeightMap w, RankMap r,
                int energy_hits,
                typename property_traits<RankMap>::value_type energy,
                std::set<typename se_graph_traits<Graph>::edge_descriptor> &seen)
        {
            if (energy_hits == 0) return; // don't even go there! *snap* -- only if we won't be modifying anything
            
            typename property_traits<RankMap>::value_type total = 0;
            BGL_FORALL_OUTEDGES_T(u, e, g, Graph) total += get(w, e);
            put(r, u, get(r, u) + energy * energy_hits);
            
            typename se_graph_traits<Graph>::out_edge_iterator ei, ei_end;
            for(boost::tie(ei, ei_end) = out_edges(u, g); ei != ei_end; ++ei) {
                if (!seen.count(*ei)) {
                    seen.insert(*ei);
                    // we haven't seen this vertex yet, spider it!
                    activation_spider(g, target(*ei, g), w, r, g[*ei].energy_hits, energy * (get(w, *ei) / total), seen);
                }
            }
        }
    } // namespace detail

    
    template <class Graph, class NodeMap, class WeightMap, class RankMap>
    void spreading_activation(Graph &g, NodeMap &n, WeightMap w, RankMap r)
    {
        typedef typename se_graph_traits<Graph>::vertex_iterator viterator;
        typedef typename se_graph_traits<Graph>::out_edge_iterator eiterator;
        typedef typename se_graph_traits<Graph>::vertex_id_type id_type;
        typedef typename se_graph_traits<Graph>::edge_descriptor edge_desc;
    
        BGL_FORALL_VERTICES_T(v, g, Graph) put(r, v, 0);
    
        // create the seen set
        std::set<edge_desc> seen;
        // and our map of vertices
        std::map<id_type, typename se_graph_traits<Graph>::vertex_descriptor> vertex_map;
    
        // get those vertex descriptors from the graph
        g.vertices_by_id(extract_first_iterator(n.begin()), extract_first_iterator(n.end()), inserter(vertex_map, vertex_map.end()));
    
        
        for(typename NodeMap::iterator i = n.begin(); i != n.end(); ++i) {
            // id is i->first, starting energy is i->second
            id_type id = (*i).first;
            detail::activation_spider(g, vertex_map[id], w, r, get_property(g, graph_energy_hits), (*i).second, seen);
        }
    }
} // namespace semantic

#endif /* _SPREADING_ACTIVATION_HPP_ */
