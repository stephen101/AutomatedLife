#ifndef _CLUSTER_HELPER_HPP_
#define _CLUSTER_HELPER_HPP_

#include <semantic/utility.hpp> // for maps

namespace semantic
{


    template <class Graph>
    class cluster_helper {
        typedef se_graph_traits<Graph> traits;
        typedef typename traits::vertex_descriptor vertex;
        typedef typename traits::vertices_size_type size_type;
        
        public:
    
        
            template <class Dendrogram, class RankMap>
            void build(Graph &g, Dendrogram &dgram, RankMap r) {
                vertex2comp_.clear();
                clusters_.clear();
                top_terms_.clear();
            
                dgram.get_clusters(inserter(vertex2comp_, vertex2comp_.begin()));
                        
                typedef maps::ordered_multi<double, vertex, std::greater<double> > my_rank_map;
                my_rank_map sorted;
                BGL_FORALL_VERTICES_T(u, g, Graph) {
                    sorted.insert(std::make_pair(r[u], u));
                }
            
                for(typename my_rank_map::iterator i = sorted.begin(); i != sorted.end(); ++i) {
                    vertex u = i->second;
                    if(g[u].type_major == node_type_major_doc) clusters_[vertex2comp_[u]].push_back(u);
                    else top_terms_[vertex2comp_[u]].push_back(u);
                }
            }
        
            size_type count() const {
                return clusters_.size();
            }
        
            std::vector<vertex> docs(size_type i) {
                return clusters_[i];
            }
        
            std::vector<vertex> terms(size_type i) {
                return top_terms_[i];
            }
        
        private:
            maps::unordered<vertex, size_type> vertex2comp_;
            maps::unordered<size_type, std::vector<vertex> > clusters_;
            maps::unordered<size_type, std::vector<vertex> > top_terms_;
    }; // class cluster_helper



} // namespace semantic





#endif
