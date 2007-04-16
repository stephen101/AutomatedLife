/***********************************
 This file is an example of how to use the
 Semantic Engine to perform a simple search
*************************************/

// include the core semantic classes
#include <semantic/semantic.hpp>

// include the sub-graph classes (searching is essentially creating a subgraph of a larger graph)
#include <semantic/subgraph.hpp>

// how do we want to create the sub-graph?
#include <semantic/subgraph/random_walk.hpp>
// other options are:
//	subgraph/bfs.hpp		-- breadth first search, fine for small graphs but slow for larger ones
	
// decide on our weighting policy. here we will use a combination of TF (term frequency)
// and IDF (inverse document frequency) and combign them with the LGWeighting helper class
// (which stands simply for local-global weighting - it does nothing special)
#include <semantic/weighting/tf.hpp>
#include <semantic/weighting/idf.hpp>
#include <semantic/weighting/lg.hpp>

// last we need a storage policy. we will choose SQLite3
#include <semantic/storage/sqlite3.hpp>
// other options are:
//	storage/mysql5.hpp		-- MySQL5 database
//	storage/none.hpp		-- stores only in memory
	
// a ranking algorithm we will use to assign a rank to each ndoe in our subgraph
#include <semantic/ranking/spreading_activation.hpp>
	
using namespace semantic;

// define our graph type
typedef SESubgraph<
	SQLite3StoragePolicy,			// the storage policy
	RandomWalkSubgraph,				// create the subgraph with a random walk
	LGWeighting<
		TFWeighting,				// local weighting
		IDFWeighting,				// global weighting
		double>						// this last parameters defines the result type to use after multiplying L and G weights
	>					Graph;
	
int main(int argc, char *argv[]) {
	// first things first: create a graph object, give it a name.
	// the name you give the Graph object defines the collection to access.
	// any storage engine can store multiple collections together.
	Graph g("my_collection");
	
	// tell the graph to use the specified SQLite3 file
	g.set_file("/path/to/file.db");
	// if we were using MySQL5 as the policy, we'd see something like this:
	// g.set_host("localhost");
	// g.set_user("user");
	// g.set_pass("secret");
	// g.set_database("some_database");
	
	// searching, as mentioned earlier is a matter of extracting a subgraph. we do this by requesting
	// from the graph a vertex that exists in the storage engine but not in our subgraph in memory.
	// everything is done by vertex (node) IDs, so we must find the ID of the node first
	se_graph_traits<Graph>::vertex_id_type vertex_id;
	// intuiti = the stemmed version of "intuition". depending on how the collection was indexed,
	// you will request the stemmed or unstemmed version
	vertex_id = g.fetch_vertex_id_by_content_and_type("intuiti", node_type_major_term);
	// to fetch a specific document ID, you would write
	// vertex_id = g.fetch_vertex_id_by_content_and_type("document_name", node_type_major_doc);
	
	// now request the node from the graph
	g.vertex_by_id(vertex_id);
	// now our graph in memory contains the nodes that were reached by extracting a sub-graph with
	// our requested node as the "base node". the semantic graphs can be treated just like a BOOST
	// graph library graph for iterating, etc
	
	boost::graph_traits<Graph>::vertex_iterator vi, vi_end;
	for(boost::tie(vi, vi_end) = vertices(g); vi != vi_end; ++vi) {
		std::cout << "node: " << g[*vi].content << " is of type " << g[*vi].type_major << std::endl;
	}
	
	// now naturally you may want to rank the nodes (aka results) of the subgraph (search). this can
	// be done using a ranking algorithm, but to use a ranking algorithm, we must first
	// get a weightmap of the edges in our graph
	weighting_traits<Graph>::edge_weight_map weights;
	g.populate_weight_map(boost::make_assoc_property_map(weights)); // populate_weight_map takes a BOOST
																	// property map
	
	// to perform ranking, we must create a map of our search nodes. each "search node" has a starting
	// energy (which can be negative for negative weighting)
	std::map<se_graph_traits<Graph>::vertex_id_type, double> search_nodes;
	search_nodes[vertex_id] = 1.0; // start on node vertex_id with a weight of 1.0
	// we must create a map that will hold the vertex ranking. we have a convenience typedef available:
	weighting_traits<Graph>::vertex_weight_map ranks;
	// now perform the algorithm
	spreading_activation(g, search_nodes, make_assoc_property_map(weights), make_assoc_property_map(ranks));
	
	// let's print out our results again
	for(boost::tie(vi, vi_end) = vertices(g); vi != vi_end; ++vi) {
		std::cout << "node: " << g[*vi].content << " has a rank of " << ranks[*vi] << std::endl;
	}	
}