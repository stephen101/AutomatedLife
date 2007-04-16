
#ifndef _LG_WEIGHTING_HPP_
#define _LG_WEIGHTING_HPP_

#include <map>
#include <semantic/utility.hpp>
#include <boost/graph/properties.hpp>

namespace semantic {

    template <class Local, class Global, typename WeightType>
    class LGWeighting {
        public:
            typedef typename Local::weight_type local_weight_type;
            typedef typename Global::weight_type global_weight_type;
            typedef WeightType weight_type;

            
            template<class Vertex, class NeighborList, class Graph, class WeightMap>
            void apply_weights(Vertex u, const NeighborList &nlist, Graph &g, WeightMap w)
            {
                typedef typename NeighborList::const_iterator iterator;

                // start by calculating the local and global weights, respectively
                typedef typename property_traits<WeightMap>::key_type key_type;
                typedef std::map<key_type, typename Local::weight_type> my_lmap;
                typedef std::map<key_type, typename Global::weight_type> my_gmap;
                my_lmap lw; boost::associative_property_map<my_lmap> lmap(lw);
                my_gmap gw; boost::associative_property_map<my_gmap> gmap(gw);

                local.apply_weights(u, nlist, g, lmap);
                global.apply_weights(u, nlist, g, gmap);

                iterator ei;
                for(ei = nlist.begin(); ei != nlist.end(); ++ei) {
                    typename se_graph_traits<typename Graph::storage_policy_type>::vertex_id_type id = g.get_vertex_id((*ei).second);
                    put(w, id, get(lmap, id) * get(gmap, id));
                }
            }

            
            template<class Vertex, class Graph, class WeightMap>
            void apply_weights(Vertex u, Graph &g, WeightMap w)
            {
                typedef typename se_graph_traits<Graph>::out_edge_iterator iterator;
                typedef typename se_graph_traits<Graph>::edge_descriptor edge;

                function_requires< WritablePropertyMapConcept<WeightMap, edge> >();

                // start by calculating the local and global weights, respectively
                typedef typename property_traits<WeightMap>::key_type key_type;
                typedef std::map<edge, typename Local::weight_type> my_lmap;
                typedef std::map<edge, typename Global::weight_type> my_gmap;
                my_lmap lw; boost::associative_property_map<my_lmap> lmap(lw);
                my_gmap gw; boost::associative_property_map<my_gmap> gmap(gw);

                local.apply_weights(u, g, lmap);
                global.apply_weights(u, g, gmap);

                iterator ei, ei_end;
                for(boost::tie(ei, ei_end) = out_edges(u, g); ei != ei_end; ++ei) put(w, *ei, lw[*ei] * gw[*ei]);
            }

        private:
            Local local;
            Global global;
    };

} // namespace semantic

#endif
