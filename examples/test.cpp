#include <semantic/semantic.hpp>
#include <semantic/storage/none.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/iteration_macros.hpp>
#include <semantic/indexing.hpp>
#include <semantic/subgraph.hpp>
#include <semantic/subgraph/none.hpp>
#include <semantic/utility.hpp>

#include <semantic/subgraph/bfs.hpp>
#include <semantic/storage/mysql5.hpp>
#include <semantic/storage/sqlite3.hpp>

#include <iostream>
#include <map>

using namespace semantic;

void vertex_properties_test() {
	typedef SEGraph<NoStoragePolicy> Graph;
	Graph my_graph("name");
	storage_traits<NoStoragePolicy>::vertex_properties_type vp;
	se_graph_traits<Graph>::vertex_descriptor v;
	vp.content = "hi!"; vp.type_major = 1; vp.type_minor = 0;
	v = add_vertex(vp, my_graph);


	assert(get_property(my_graph, graph_name) == "name");
	assert(boost::num_vertices(my_graph) == 1);
	assert(my_graph[v].content == "hi!");
	assert(my_graph[v].type_major == 1);
	assert(my_graph[v].type_minor == 0);
	
	bool caught_no_properties_exception = false;
	try {
		add_vertex(my_graph);
	} catch (...) {
		caught_no_properties_exception = true;
	}
	assert(caught_no_properties_exception);
	assert(boost::num_vertices(my_graph) == 1);
	
	remove_vertex(v, my_graph);
	assert(boost::num_vertices(my_graph) == 0);
}

void indexing_test() {
	typedef SEGraph<NoStoragePolicy> Graph;
	Graph g("name");
	g.set_mirror_changes_to_storage(true);
	text_indexing_helper<Graph> indexer(g);
	indexer.add_doc_term_edge("doc1", "term1", 5);
	g.set_mirror_changes_to_storage(false);
	assert(num_vertices(g) == 2);
	assert(num_edges(g) == 2);
	Graph::edge_iterator ei, ei_end;
	for(boost::tie(ei, ei_end) = edges(g); ei != ei_end; ++ei) {
//		std::cout << g[*ei].from_degree << std::endl;
		assert(g[*ei].from_degree == 1);
//		std::cout << g[*ei].to_degree << std::endl;
		assert(g[*ei].to_degree == 1);
		assert(g[*ei].strength == 5);
	}
}

void simple_subgraph_test() {
	typedef SESubgraph<NoStoragePolicy, NoSubgraphPolicy> Graph;
	typedef se_graph_traits<Graph>::vertex_descriptor Vertex;
	typedef se_graph_traits<Graph>::edge_descriptor Edge;
	typedef se_graph_traits<Graph>::edge_iterator edge_iterator;
	Graph g("name");
	g.set_mirror_changes_to_storage(true);
	text_indexing_helper<Graph> indexer(g);
	indexer.add_doc_term_edge("doc1", "term1", 5);
	g.set_mirror_changes_to_storage(false);
	
	assert(num_vertices(g) == 2);
	assert(num_edges(g) == 2);
	edge_iterator ei, ei_end;
	for(boost::tie(ei, ei_end) = edges(g); ei != ei_end; ++ei) {
		assert(g[*ei].from_degree == 1);
		assert(g[*ei].to_degree == 1);
		assert(g[*ei].strength == 5);
	}
	
	Vertex u = *(vertices(g).first);
	assert(g.vertex_by_id(u) == u);
	
	typedef std::map<Edge, NoWeighting::weight_type> wmap;
	wmap weight_map;
	associative_property_map<wmap> pmap(weight_map);
	
	g.populate_weight_map(pmap);
	
	for(boost::tie(ei, ei_end) = edges(g); ei != ei_end; ++ei) {
		assert(weight_map[*ei] == 1);
	}
}

#include <fstream>
#include <semantic/weighting/idf.hpp>
#include <semantic/weighting/tf.hpp>
#include <semantic/weighting/lg.hpp>
#include <semantic/analysis/harel_simple_random_walk_cluster.hpp>
#include <semantic/analysis/harel_bfs_cluster.hpp>
#include <semantic/analysis/utility.hpp>
#include <semantic/pruning.hpp>

void compass_test() {
	std::ifstream in("/tmp/townhall_index.txt");
	
	typedef LGWeighting<TFWeighting, IDFWeighting, double> weighting;
//	typedef TFWeighting weighting;
	typedef SESubgraph<NoStoragePolicy, NoSubgraphPolicy, weighting> Graph;
//	typedef SEGraph<NoStoragePolicy> Graph;
	Graph g("compass");
	g.set_mirror_changes_to_storage(true);
	text_indexing_helper<Graph> indexer(g);
	
	int i = 0;
	string doc, term;
	char line[1024];
	while (in.getline(line, 1024, '\n')) {
		if (i == 0) doc = line;
		if (i == 1) term = line;
		if (i == 2) {
			indexer.add_doc_term_edge(doc, term, atoi(line));
			i=-1;
		}
		i++;
	}

	std::cout << num_vertices(g) << std::endl;
	std::cout << num_edges(g) << std::endl;
	
	g.apply_to_graph(remove_extremes());

	std::cout << num_vertices(g) << std::endl;
	std::cout << num_edges(g) << std::endl;

	g.apply_to_all_vertices(remove_low_strength_edges(2));
	g.apply_to_graph(remove_low_strength_vertices(2));
	
	g.set_mirror_changes_to_storage(false);
	
	std::cout << num_vertices(g) << std::endl;
	std::cout << num_edges(g) << std::endl;
	
	typedef std::map<se_graph_traits<Graph>::edge_descriptor, weighting::weight_type> WeightMap;
	WeightMap weights;
	typedef associative_property_map<WeightMap> WeightPropertyMap;
	WeightPropertyMap weightmap(weights);
	
	g.populate_weight_map(weightmap);

	typedef std::map<Graph::vertex_descriptor, unsigned long> ComponentMap;
	ComponentMap comp_map;
	typedef associative_property_map<ComponentMap> ComponentPropertyMap;
	ComponentPropertyMap comp_pmap(comp_map);
	
/*	typedef std::map<weighting::weight_type, unsigned long> DistribMap;
	DistribMap distrib;
	typedef associative_property_map<DistribMap> DistribPropertyMap;
	DistribPropertyMap distrib_map(distrib);
	
	semantic::get_weight_distribution(g, weightmap, distrib_map, 1);
	
	for(DistribMap::iterator i = distrib.begin(); i != distrib.end(); ++i) {
		std::cout << (*i).first << "\t" << (*i).second << std::endl;
	}*/
	
	assert(weights.size() == num_edges(g));
	
	keep_top_n_percent_of_edges<WeightPropertyMap> prune(0.2, weightmap);
//	g.apply_to_all_vertices(keep_top_n_edges<WeightPropertyMap>(5, weightmap));

	unsigned long cnt = analysis::weak_components_by_threshold(g, weightmap, comp_pmap, 0);
	std::vector<unsigned long> comp_counts(cnt);
	BGL_FORALL_VERTICES(u, g, Graph) comp_counts[comp_map[u]]++;
	std::cout << "there are " << cnt << " components to start." << std::endl;
	for(unsigned long i = 0; i < comp_counts.size(); i++) {
		std::cout << i << " has " << comp_counts[i] << " components" << std::endl;
	}

	std::cout << num_vertices(g) << std::endl;
	std::cout << num_edges(g) << std::endl;
	
	analysis::harel_bfs_separation(g, weightmap, 2);

	typedef std::map<weighting::weight_type, unsigned long> DistribMap;
	DistribMap distrib;
	typedef associative_property_map<DistribMap> DistribPropertyMap;
	DistribPropertyMap distrib_map(distrib);
	
	semantic::get_weight_distribution(g, weightmap, distrib_map, 1);
	
	for(DistribMap::iterator i = distrib.begin(); i != distrib.end(); ++i) {
		std::cout << (*i).first << "\t" << (*i).second << std::endl;
	}

	std::ofstream outfile("/tmp/compass2.le");
	BGL_FORALL_EDGES(e, g, Graph) outfile << g[source(e, g)].content << " " << g[target(e, g)].content << " " << get(weightmap, e) << std::endl;

	// now perform the clustering
	weighting::weight_type min, max;
	weighting::weight_type threshold;
	unsigned long num;
	boost::tie(min, max) = map_min_max(edges(g).first, edges(g).second, weightmap);
	while(1) {
		string cmd, tmp;
		std::cout << "# "; // prompt
		std::cin >> cmd;
		if (cmd == "c") {
			std::cin >> tmp;
			comp_map.clear();
			double dbl = atof(tmp.c_str());
 			threshold = min + (max-min)*dbl;
			std::cout << "clustering with: " << dbl << " -> " << threshold << std::endl;
			num = analysis::weak_components_by_threshold(g, weightmap, comp_pmap, threshold);
			std::cout << "num clusters: " << num << std::endl;
		} else if (cmd == "d") {
			std::vector<unsigned long> comp_counts(num);
			std::vector<std::vector<std::string> > comp_members(num);
			BGL_FORALL_VERTICES(u, g, Graph) {comp_counts[comp_map[u]]++; if(g[u].type_major == 2) comp_members[comp_map[u]].push_back(g[u].content);}
			for(unsigned long i = 0; i < comp_counts.size(); i++) {
				if (comp_counts[i] > 7) {
					std::cout << i << " has " << comp_counts[i] << " components";
					if (comp_counts[i] < 50) {
						std::cout << ": ";
						copy(comp_members[i].begin(), comp_members[i].end(), std::ostream_iterator<std::string>(std::cout, " "));
					}
					std::cout << std::endl;
				}
			}
		} else if (cmd == ">") {
			std::cin >> tmp;
			std::cout << "writing to file " << tmp << std::endl;
			std::ofstream out(tmp.c_str());
			Graph::vertex_iterator vi, vi_end;
			out << num << std::endl;
			for(boost::tie(vi, vi_end) = vertices(g); vi != vi_end; ++vi) {
				out << comp_map[*vi] << "\t" << g[*vi].content << std::endl;
			}
			out.close();
		} else {
			std::cout << "unknown command: " << cmd << std::endl;
		}
	}
}

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
	
	Graph g("compass");
	g.set_host("localhost");
	g.set_user("root");
	g.set_pass("");
	g.set_database("sem_test2");
	
	g.set_depth(4);
	g.set_trials(40);
	
	// set the node list
	std::map<unsigned long, double> nodes;
	nodes[51346] = 1; // starting energy on search
	wtraits::vertex_weight_map rank_map;
	wtraits::edge_weight_map weights;
	
	g.fetch_subgraph_starting_from(51346);
	
	g.populate_weight_map(boost::make_assoc_property_map(weights));
	BGL_FORALL_EDGES(e, g, Graph) std::cout << e << "\t" << weights[e] << std::endl;
	spreading_activation(g, nodes,  boost::make_assoc_property_map(weights), boost::make_assoc_property_map(rank_map));
	
	std::cout << "graph: " << get_property(g, graph_name) << " (v:" << num_vertices(g) << " e:" << num_edges(g) << ")" << std::endl;
	
	// output le rankmap
	BGL_FORALL_VERTICES(u, g, Graph) std::cout << g[u].content << "\t" << rank_map[u] << std::endl;
	
//	std::cout << g << std::endl;
}

void linlog_test() {
	typedef SESubgraph<
		MySQL5StoragePolicy, 
		RandomWalkSubgraph,
		LGWeighting<TFWeighting, IDFWeighting, double>
		> Graph;
	typedef weighting_traits<Graph> wtraits;
	typedef se_graph_traits<Graph> traits;
	
	Graph g("compass");
	g.set_host("localhost");
	g.set_user("root");
	g.set_pass("");
	g.set_database("sem_test2");
	
	g.set_depth(4);
	g.set_trials(100);
	
	// set the node list
	std::map<unsigned long, double> nodes;
	nodes[51346] = 1; // starting energy on search
	wtraits::vertex_weight_map rank_map;
	wtraits::edge_weight_map weights;
	
	g.fetch_subgraph_starting_from(51346);
	
	g.populate_weight_map(boost::make_assoc_property_map(weights));
	BGL_FORALL_EDGES(e, g, Graph) std::cout << e << "\t" << weights[e] << std::endl;
	spreading_activation(g, nodes,  boost::make_assoc_property_map(weights), boost::make_assoc_property_map(rank_map));
	
	std::cout << "graph: " << get_property(g, graph_name) << " (v:" << num_vertices(g) << " e:" << num_edges(g) << ")" << std::endl;
	
	// output le rankmap
	BGL_FORALL_VERTICES(u, g, Graph) std::cout << g[u].content << "\t" << rank_map[u] << std::endl;
	
	// output a file
	std::ofstream out("/tmp/compass.le");
	
	traits::edge_iterator ei, ei_end;
	for(boost::tie(ei, ei_end) = edges(g); ei != ei_end; ++ei) {
		out << g[source(*ei, g)].content << " " << g[target(*ei, g)].content << " " << weights[*ei] << std::endl;
	}
	
	out.close();
	
//	std::cout << g << std::endl;
}

void linlog_test2() {
	std::ifstream in("/tmp/compass_index.txt");

	typedef LGWeighting<TFWeighting, IDFWeighting, double> weighting;
	typedef SESubgraph<NoStoragePolicy, NoSubgraphPolicy, weighting> Graph;
	typedef se_graph_traits<Graph> traits;
//	typedef SEGraph<NoStoragePolicy> Graph;
	Graph g("compass");
	g.set_mirror_changes_to_storage(true);
	text_indexing_helper<Graph> indexer(g);

	int i = 0;
	string doc, term;
	char line[1024];
	while (in.getline(line, 1024, '\n')) {
		if (i == 0) doc = line;
		if (i == 1) term = line;
		if (i == 2) {
			indexer.add_doc_term_edge(doc, term, atoi(line));
			i=-1;
		}
		i++;
	}

	std::cout << num_vertices(g) << std::endl;
	std::cout << num_edges(g) << std::endl;

	g.apply_to_graph(remove_extremes());

	std::cout << num_vertices(g) << std::endl;
	std::cout << num_edges(g) << std::endl;

	g.apply_to_all_vertices(remove_low_strength_edges(2));

	g.set_mirror_changes_to_storage(false);

	std::cout << num_vertices(g) << std::endl;
	std::cout << num_edges(g) << std::endl;

	typedef std::map<se_graph_traits<Graph>::edge_descriptor, weighting::weight_type> WeightMap;
	WeightMap weights;
	typedef associative_property_map<WeightMap> WeightPropertyMap;
	WeightPropertyMap weightmap(weights);

	g.populate_weight_map(weightmap);
	
	// output a file
	std::ofstream out("/tmp/compass.el");
	
	traits::edge_iterator ei, ei_end;
	for(boost::tie(ei, ei_end) = edges(g); ei != ei_end; ++ei) {
		out << g[source(*ei, g)].content << " " << g[target(*ei, g)].content << " " << weights[*ei] << std::endl;
	}
	
	out.close();
}

void full_graph_extract_test() {
	typedef LGWeighting<TFWeighting, IDFWeighting, double> weighting;
	typedef SESubgraph<SQLite3StoragePolicy, NoSubgraphPolicy, weighting> Graph;
	
	Graph g("compass");
	g.set_file("/Users/gabe/Code/Indexes/compass.db");
	g.populate_full_graph();
	
	std::cout << "graph: " << num_vertices(g) << " " << num_edges(g) << std::endl;
}

void run_all() {
//	vertex_properties_test();
//	indexing_test();
//	simple_subgraph_test();

//	compass_test();
	
//	mysql_test();
	
//	linlog_test2();
	
	full_graph_extract_test();
	
	std::cout << std::endl << "All tests ran and completed successfully." << std::endl << std::endl;
}

int main() {
	run_all();
}
