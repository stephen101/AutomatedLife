#include <semantic/semantic.hpp>
#include <semantic/storage/none.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/iteration_macros.hpp>
#include <semantic/subgraph.hpp>
#include <semantic/subgraph/none.hpp>
#include <semantic/utility.hpp>

#include <semantic/subgraph/bfs.hpp>
#include <semantic/storage/mysql5.hpp>

#include <iostream>
#include <map>

using namespace semantic;




#include <fstream>
#include <semantic/weighting/idf.hpp>
#include <semantic/weighting/tf.hpp>
#include <semantic/weighting/lg.hpp>
#include <semantic/analysis/harel_simple_random_walk_cluster.hpp>
#include <semantic/analysis/harel_bfs_cluster.hpp>
#include <semantic/analysis/utility.hpp>
#include <semantic/pruning.hpp>



#include <semantic/subgraph/random_walk.hpp>
#include <semantic/ranking/spreading_activation.hpp>

void mysql_test() {
	typedef SESubgraph<
		MySQL5StoragePolicy, 
		RandomWalkSubgraph,
		LGWeighting<TFWeighting, IDFWeighting, double>
		> Graph;
	typedef weighting_traits<Graph> wtraits;
	typedef se_graph_traits<Graph> traits;
	
	Graph g("nuclear");
	g.set_host("localhost");
	g.set_user("root");
	g.set_pass("");
	g.set_database("semantic");
	
	g.set_depth(4);
	g.set_trials(100);
	
	// set the node list
	std::map<unsigned long, double> nodes;
	unsigned long id = g.fetch_vertex_id_by_content_and_type("iraq", 2);
	nodes[id] = pow(10,10); // starting energy on search
	wtraits::vertex_weight_map rank_map;
	wtraits::edge_weight_map weights;
	
	g.fetch_subgraph_starting_from( id );
	
	g.populate_weight_map(boost::make_assoc_property_map(weights));
//	BGL_FORALL_EDGES(e, g, Graph) std::cout << e << "\t" << get(weight_map, e) << std::endl;
	spreading_activation(g, nodes, boost::make_assoc_property_map(weights), boost::make_assoc_property_map(rank_map));
	
	std::cout << "graph: " << get_property(g, graph_name) << " (v:" << num_vertices(g) << " e:" << num_edges(g) << ")" << std::endl;
	
	// output le rankmap
	BGL_FORALL_VERTICES(u, g, Graph) std::cout << g[u].content << "\t" << rank_map[u] << std::endl;
	
//	std::cout << g << std::endl;
}




int main() {
	mysql_test();
}
