

#ifndef _IDF_WEIGHTING_HPP_
#define _IDF_WEIGHTING_HPP_

#include <math.h>

namespace semantic {

    class IDFWeighting {
        public:
            typedef double weight_type;

            
            template<class Vertex, class NeighborList, class Graph, class WeightMap>
            void apply_weights(Vertex u, const NeighborList &nlist, Graph &g, WeightMap w)
            {
                typedef typename NeighborList::const_iterator iterator;
                
                    
                    typename se_graph_traits<typename Graph::storage_policy_selector>::vertices_size_type count
                                = g.get_vertex_count_of_type(g[u].type_major);
                iterator ei;
                for(ei = nlist.begin(); ei != nlist.end(); ++ei) { 
                                                                       put(w, g.get_vertex_id((*ei).second), (float)log((float)1+count/((*ei).first.to_degree))); }
            }

            
            template<class Vertex, class Graph, class WeightMap>
            void apply_weights(Vertex u, Graph &g, WeightMap w)
            {
                typedef typename se_graph_traits<Graph>::out_edge_iterator iterator;
                typedef typename se_graph_traits<Graph>::edge_descriptor edge;
                function_requires< WritablePropertyMapConcept<WeightMap, edge> >();
                
                    
                    typename se_graph_traits<typename Graph::storage_policy_selector>::vertices_size_type count
                                = g.get_vertex_count_of_type(g[u].type_major);
                iterator ei, ei_end;
                for(boost::tie(ei, ei_end) = out_edges(u, g); ei != ei_end; ++ei) { 
                                                                                        put(w, *ei, (float)log((float)1+count/(g[*ei].to_degree))); }
            }
    };
} // namespace semantic

#endif // _IDF_WEIGHTING_HPP_

