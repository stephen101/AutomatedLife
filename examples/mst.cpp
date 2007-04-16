#include <semantic/semantic.hpp>
#include <semantic/storage/sqlite3.hpp>
#include <boost/graph/iteration_macros.hpp>
#include <semantic/utility.hpp>

#include <semantic/analysis/agglomerate_clustering/mst.hpp>
#include <semantic/analysis/agglomerate_clustering/dendrogram.hpp>
#include <semantic/subgraph.hpp>
#include <semantic/subgraph/pruning_random_walk.hpp>
#include <semantic/weighting/tf.hpp>
#include <semantic/weighting/idf.hpp>
#include <semantic/weighting/lg.hpp>

#include <map>


using namespace semantic;


typedef LGWeighting<TFWeighting, IDFWeighting, double> weighting;
typedef SESubgraph<SQLite3StoragePolicy, PruningRandomWalkSubgraph, weighting> Graph;


int main(int argc, char *argv[]) {

	typedef se_graph_traits<Graph> Traits;
	typedef Traits::vertex_descriptor vertex;
	std::string filename = argv[1];
	
	std::string collection = argv[2];
	
	std::string query = argv[3];
	
	Graph g(collection);
	g.set_file(filename);
	g.keep_only_top_edges(0.3);
		
	g.fetch_subgraph_starting_from(g.fetch_vertex_id_by_content_and_type(query, node_type_major_term));
	weighting_traits<Graph>::edge_weight_map weights;
	g.populate_weight_map(boost::make_assoc_property_map(weights));
		
	
	// vertex u[6];
	// for(int i = 0; i < 6; i++) {
	// 	Traits::vertex_properties_type p;
	// 	p.content = to_string(i);
	// 	u[i] = add_vertex(p, g);
	// 	std::cout << i << " is " << u[i] << std::endl;
	// }
	// 	
	// Traits::edge_properties_type p;
	// weights[add_edge(u[0], u[1], p, g).first] = 3;
	// weights[add_edge(u[1], u[2], p, g).first] = 2;
	// weights[add_edge(u[2], u[0], p, g).first] = 2;
	// weights[add_edge(u[2], u[3], p, g).first] = 1;
	// weights[add_edge(u[3], u[4], p, g).first] = 2;
	// weights[add_edge(u[3], u[5], p, g).first] = 2;
	// weights[add_edge(u[4], u[5], p, g).first] = 3;
	
	
	std::cout << "Graph: v " << num_vertices(g) << " e " << num_edges(g) << std::endl;
	
	// compute the min spanning tree
	std::vector<se_graph_traits<Graph>::edge_descriptor> mst;
	minimum_weight_spanning_tree(g, boost::make_assoc_property_map(weights), back_inserter(mst));
	
	std::cout << mst.size() << " edges in MST" << std::endl;
	
//	std::copy(mst.begin(), mst.end(), std::ostream_iterator<Traits::edge_descriptor>(std::cout, " "));
	
	// now let's try a dendrogram
	dendrogram<Graph> dgram(g);
	dendrogram_from_similarity_mst(g, mst, boost::make_assoc_property_map(weights), dgram, SingleLinkageDistanceCalculator());
	
	dgram.set_num_clusters(8);
	
	std::map<vertex, Traits::vertices_size_type> cluster_map;
	dgram.get_clusters(inserter(cluster_map, cluster_map.begin()));
	
	for(std::map<vertex, Traits::vertices_size_type>::iterator i = cluster_map.begin(); i!=cluster_map.end(); ++i) {
		std::cout << g[i->first].content << "\t\t" << i->second << std::endl;
	}
}