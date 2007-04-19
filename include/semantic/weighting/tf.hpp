

#ifndef _TF_WEIGHTING_HPP_
#define _TF_WEIGHTING_HPP_

#include <math.h>

namespace semantic {

    class TFWeighting {
        public:
            typedef double weight_type;

            
            template<class Vertex, class NeighborList, class Graph, class WeightMap>
            void apply_weights(Vertex, const NeighborList &nlist, Graph &g, WeightMap w)
            {
                typedef typename NeighborList::const_iterator iterator;
                
                iterator ei;
                for(ei = nlist.begin(); ei != nlist.end(); ++ei) { 
                      put(w, g.get_vertex_id((*ei).second), (*ei).first.strength); 
				}
            }

            
            template<class Vertex, class Graph, class WeightMap>
            void apply_weights(Vertex u, Graph &g, WeightMap w)
            {
                typedef typename se_graph_traits<Graph>::out_edge_iterator iterator;
                typedef typename se_graph_traits<Graph>::edge_descriptor edge;
                function_requires< WritablePropertyMapConcept<WeightMap, edge> >();
                
                iterator ei, ei_end;
                for(boost::tie(ei, ei_end) = out_edges(u, g); ei != ei_end; ++ei) { 
                    put(w, *ei, g[*ei].strength); 
				}
            }
    };
} // namespace semantic

#endif // _TF_WEIGHTING_HPP_

