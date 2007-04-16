
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/positional_options.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/parsers.hpp>

#include <semantic/semantic.hpp>
#include <semantic/version.hpp>
#include <semantic/search.hpp>

// for clustering
#include <semantic/analysis/linlog.hpp>
#include <boost/progress.hpp>
#include <semantic/analysis/agglomerate_clustering/dendrogram.hpp>
#include <semantic/analysis/agglomerate_clustering/mst.hpp>
#include <semantic/analysis/agglomerate_clustering/cluster_helper.hpp>

#if SEMANTIC_HAVE_MYSQL
#include <semantic/storage/mysql5.hpp>
#endif
#if SEMANTIC_HAVE_SQLITE3
#include <semantic/storage/sqlite3.hpp>
#endif

#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <map>
#include <string>

typedef LGWeighting<TFWeighting,IDFWeighting,double> WeightingPolicy;

#if SEMANTIC_HAVE_MYSQL
typedef SESubgraph<MySQL5StoragePolicy, PruningRandomWalkSubgraph, WeightingPolicy > MySQLGraph;
#endif
#if SEMANTIC_HAVE_SQLITE3
typedef SESubgraph<SQLite3StoragePolicy, PruningRandomWalkSubgraph, WeightingPolicy > SQLiteGraph;
#endif

typedef std::pair<std::string,double> single_result;
typedef std::vector<single_result> sorted_results;
typedef std::pair<sorted_results,sorted_results> docs_and_terms;

using namespace semantic;
namespace po = boost::program_options;

#define usage() \
	std::cerr << "Usage: " << argv[0] << " [options] \"query\"" << std::endl << std::endl; \
	std::cerr << "Options: " << std::endl; \
	std::cerr << opts << std::endl; \
	return 0; \


template <class Graph>
void cluster(Graph &g, std::string query_string, int num) {
    typedef analysis::linlog_traits<Graph, boost::associative_property_map<typename weighting_traits<Graph>::edge_weight_map> > linlog;
    typedef typename se_graph_traits<Graph>::vertex_descriptor vertex;
    typedef typename se_graph_traits<Graph>::vertices_size_type size_type;
    // get our weight map first
    typename weighting_traits<Graph>::edge_weight_map w;
    g.populate_weight_map(boost::make_assoc_property_map(w));
    
    // create a point map
    typename linlog::vertex_position_map p;
    
    std::cout << std::endl << "Calculating clusters..." << std::endl;
    
    boost::progress_display counter(300, std::cout);
    analysis::linlog_layout_3d(g, boost::make_assoc_property_map(w), 300, p, counter);

    typedef typename se_graph_traits<Graph>::vertex_pair_edge edge;
    maps::unordered<edge, double> distances;
    BGL_FORALL_VERTICES_T(u, g, Graph) {
        BGL_FORALL_VERTICES_T(v, g, Graph) {
            edge e1(u,v), e2(v,u);
            distances[e1] = distances[e2] = p[u].dist(p[v]);
        }
    }
    
    // now generate an MST
    std::vector<edge> mst;
    minimum_weight_spanning_tree(g, extract_keys(distances.begin()), extract_keys(distances.end()), boost::make_assoc_property_map(distances), back_inserter(mst));
    
    // create the dendrogram
    dendrogram<Graph> dgram(g);
    dendrogram_from_distance_mst(g, mst, boost::make_assoc_property_map(distances), dgram, SingleLinkDistanceCalculator());
    
    // we have a dendrogram!
    dgram.set_num_clusters(num);
    
    // we need a ranking map -- bad planning among other things made this inaccessible,
    // so we're going to use the search query to get the nodes...... and re-create
    // the rank map. ugh
//    search_query query(vm["query"].as<std::string>(), g);
    search_query query(query_string, g);
    std::vector<typename se_graph_traits<Graph>::vertex_id_type> vertex_ids = query.get_vertex_ids(g);
    std::map<typename se_graph_traits<Graph>::vertex_id_type, double> nodes;
    for(unsigned int i = 0; i < vertex_ids.size(); i++) {
        nodes[vertex_ids[i]] = pow((double)10,10);
    }
    typename weighting_traits<Graph>::vertex_weight_map r;
    spreading_activation(g, nodes, boost::make_assoc_property_map(w), boost::make_assoc_property_map(r));
    
    cluster_helper<Graph> helper;
    helper.build(g, dgram, boost::make_assoc_property_map(r));

    size_type num_clusters = helper.count();
    std::cout << std::endl;
    for(unsigned int i = 0; i < num_clusters; i++) {
        // output cluster i
        std::cout << "Cluster: ";
        std::vector<vertex> top_terms = helper.terms(i);
        for(unsigned int t = 0; t < 5 && t < top_terms.size(); t++) {
            if (t != 0) std::cout << ", ";
            std::cout << g[top_terms[t]].content;
        }
        std::cout << std::endl;
        std::vector<vertex> docs = helper.docs(i);
        for(unsigned int d = 0; d < docs.size(); d++) {
            std::cout << "\t" << g[docs[d]].content << std::endl;
        }
        
        std::cout << std::endl; // for good measure
    }
}


int main( int argc, char* argv[]){
	po::options_description opts;
	opts.add_options()
		("help", "produce this help message\n")
		("version", "print version information\n")
		("collection,c", po::value<std::string>()->default_value("My Collection"), "The collection to search\n")
		("summaries", "Print summaries for each document\n")
		("spread", po::value<double>()->default_value(0.3), "a value from 0 to 1, specifying how\nbroad the search. 1 = most broad\n")
		("cluster", "output results in clusters instead\nof a list\n")
		("num_clusters", po::value<int>()->default_value(4), "the number of clusters\n")
//		("num_clusters", po::value<int>(), "override the number of clusters (defaults to 'best fit' using silhouette measure)")
#if SEMANTIC_HAVE_SQLITE3
		("sqlite,s", po::value<std::string>(), "the SQLite 3 database file to use\n")
#endif
#if SEMANTIC_HAVE_MYSQL
		("mysql,m", po::value<std::string>(), "the MySQL database name")
		("mysql_username,u", po::value<std::string>()->default_value(std::getenv("USER")), "the MySQL database username")
		("mysql_password,p", po::value<std::string>()->default_value(""), "the MySQL database password")
		("mysql_hostname,h", po::value<std::string>()->default_value("localhost"), "the MySQL database host\n")
#endif
		;

	
	po::variables_map vm;
	try {
		po::options_description hidden_opts;
		hidden_opts.add_options()
			("query", po::value<std::string>(), "the search query")
			;

		po::options_description all_opts;
		all_opts.add(opts).add(hidden_opts);

		po::positional_options_description positional_opts;
		positional_opts.add("query",1);
		po::store(po::command_line_parser(argc, argv).options(all_opts).positional(positional_opts).run(), vm);
		po::notify(vm);		
	} catch (std::exception &e) {
		std::cerr << "Error: " << e.what() << std::endl;
		usage();
	}


	if (vm.count("help")){
		usage();
	}

	if (vm.count("version")) {
		// print out some version information
		std::cerr << std::endl;
		print_version_info(std::cerr);
		std::cerr << std::endl;
		return 0;
	}

#if !SEMANTIC_HAVE_MYSQL && !SEMANTIC_HAVE_SQLITE3
	std::cerr << "Error: you must compile the indexer with either MySQL or SQLite support!" << std::endl;
	return 0;
#endif

	// check for required options
	if (!vm.count("collection") || !vm.count("query")){
		usage();
	}



	if ( !vm.count("mysql") && !vm.count("sqlite")) {
		std::string error;
#if SEMANTIC_HAVE_MYSQL && SEMANTIC_HAVE_SQLITE3			
		error = "Error: you must supply either a MySQL or SQLite 3 database!";
#elif SEMANTIC_HAVE_MYSQL
		error = "Error: you must supply a MySQL database!";
#else
		error = "Error: you must supply a SQLite 3 database!";
#endif
		std::cerr << error << std::endl << std::endl;
		usage();
	}

	if( vm.count("mysql") && !vm.count("mysql_username")){
		std::cerr << "Error: you must supply a username for the MySQL database: " << vm["mysql"].as<std::string>() << std::endl;
	}
	
	
/* ************************************************** * 
 * 		Connect to the database and run the search 
 * ************************************************** */	
	 
	sorted_results docs, terms;
	std::map<std::string,std::string> summaries;
			
	if( vm.count("sqlite")){ 		// SQLite
#if SEMANTIC_HAVE_SQLITE3
		SQLiteGraph g(vm["collection"].as<std::string>());
		try {
			g.set_file(vm["sqlite"].as<std::string>());
			g.set_trials(100);
			g.set_depth(4);
			g.keep_only_top_edges((float)vm["spread"].as<double>());
		} catch (SQLiteException &e ){
			std::cerr << "Error: " << e.what() << std::endl;
		} catch (std::exception &e ){
			std::cerr << "Error: " << e.what() << std::endl;
		}
		search<SQLiteGraph> engine(g);
		
		docs_and_terms results;
		
		try {
			results = engine.do_better_search(vm["query"].as<std::string>());
		} catch ( std::exception &e ){
			std::cerr << "Error: " << e.what() << std::endl;
		}
		
		docs = results.first;
		terms = results.second;
		if( vm.count("summaries"))
			summaries = engine.summarize_documents(docs);
		
		if (vm.count("cluster")) {
		    cluster(g, vm["query"].as<std::string>(), vm["num_clusters"].as<int>());
		}
#endif
		
	} else if (vm.count("mysql")) { // MySQL
#if SEMANTIC_HAVE_MYSQL
		MySQLGraph g(vm["collection"].as<std::string>());
		try {
		g.set_host(vm["mysql_hostname"].as<std::string>());
		g.set_user(vm["mysql_username"].as<std::string>());
		if (vm.count("mysql_password")) g.set_pass(vm["mysql_password"].as<std::string>());
		g.set_database(vm["mysql"].as<std::string>());
		g.set_trials(100);
		g.set_depth(4);
		g.keep_only_top_edges((float)vm["spread"].as<double>());
		} catch (MySQLException &e ){
			std::cerr << "MySQL Error: " << e.what() << std::endl;
		} catch (std::exception &e ){
			std::cerr << "Error: " << e.what() << std::endl;
		}

		search<MySQLGraph> engine(g);
		docs_and_terms results = engine.do_better_search(vm["query"].as<std::string>());
		docs = results.first;
		terms = results.second;
		if( vm.count("summaries"))
			summaries = engine.summarize_documents(docs);
		
#endif
	}
	
/* ************************************************** * 
 * 		If clustering, don't output results
 * ************************************************** */
 
    if (vm.count("cluster")) {
        return 0;
    }
	
/* ************************************************** * 
 * 		Print out the results!
 * ************************************************** */	
	std::cout << "Found " << docs.size() << " documents" << std::endl << "Similar terms: ";
	int i = 0;
	sorted_results::iterator pos;
	int j = 10;
	for( pos = terms.begin(); pos != terms.end() && i < j; ++pos,++i ){
		std::cout << pos->first;
		if( i < j-1)
			std::cout << ", ";
	}
	std::cout << std::endl;

	i = 0;
	int width = 1;
	for( pos = docs.begin(); pos != docs.end(); ++pos ){
		int size = (int)pos->first.size();
		if( size > width){ width = size;}
	}
	std::cout << std::setw(6) << " " << std::setw(width+2) << std::left << "DOCUMENT" << " RELEVANCE" << std::endl;
	for( pos = docs.begin(); pos != docs.end(); ++pos ){
		std::cout << std::setw(3) << std::right << ++i << ".  " << std::setw(width+2) << std::left << pos->first << " " << pos->second << std::endl;
		if( vm.count("summaries")){	
			std::cout << std::setw(6) << " " << std::setw(width+2) << std::left << summaries[pos->first] << std::endl << std::endl;
		}
	}	
	return 0;	
}
