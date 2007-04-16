

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/positional_options.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/parsers.hpp>

#include <semantic/semantic.hpp>
#include <semantic/version.hpp>
#include <semantic/file_finder.hpp>
#include <semantic/filter.hpp>
#include <semantic/indexing.hpp>
#include <semantic/subgraph.hpp>
#if SEMANTIC_HAVE_SQLITE3
#include <semantic/storage/sqlite3.hpp>
#endif
#if SEMANTIC_HAVE_MYSQL
#include <semantic/storage/mysql5.hpp>
#endif

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <map>
#include <string>
#include <set>
#include <vector>

using namespace semantic;

namespace po = boost::program_options;


#if SEMANTIC_HAVE_SQLITE3
typedef SEGraph<SQLite3StoragePolicy> SQLiteGraph;
#endif
#if SEMANTIC_HAVE_MYSQL
typedef SEGraph<MySQL5StoragePolicy> MySQLGraph;
#endif

typedef std::map<std::string,int> UnstemmedCount;
std::string encoding;

#define usage() \
	std::cerr << "Usage: " << argv[0] << " [options] <directory to index>" << std::endl << std::endl; \
	std::cerr << "Options: " << std::endl; \
	std::cerr << opts << std::endl; \
	exit(EXIT_SUCCESS); \
	

bool try_loading_stoplist(const std::string &filename, std::set<string> &stoplist) {
    std::ifstream file;
    file.open(filename.c_str(), std::ios_base::in | std::ios_base::binary);
    if (file) {
        std::string line;
        while( std::getline( file, line ) ){
    		stoplist.insert( line );
    	}
    	file.clear();
    	file.close();
    	return true;
    }
    return false;
}
	
std::set<std::string> load_stoplist(const std::string &filename){
    std::set<std::string> stoplist;
    if (try_loading_stoplist(STOPLIST_INSTALL_LOCATION, stoplist)) return stoplist;
    if (try_loading_stoplist(filename, stoplist)) return stoplist;
    
    std::cerr << "Could not load stop list from either '" << filename << "' or '" << STOPLIST_INSTALL_LOCATION << "'!" << std::endl;
    exit( EXIT_FAILURE );
    
}
	
template <class Graph>
bool do_indexing(Graph &g, text_indexer<Graph> &indexer, const std::vector<std::string> &filenames, const std::string term_type, int stemmer = 1, int verbose = 0 ){
	
	
 	std::set<std::string> blacklist = load_stoplist("../share/stoplist_en.txt");
	
	
	std::string max = g.get_meta_value("max_phrase_length","3");
	indexer.add_word_filter(blacklist_filter(blacklist));
 	indexer.add_word_filter(too_many_numbers_filter(6));
 	indexer.add_word_filter(minimum_length_filter(3));
 	indexer.add_word_filter(maximum_word_length_filter(15));
 	indexer.add_word_filter(maximum_phrase_length_filter(atoi(max.c_str())));
	indexer.set_default_encoding( encoding );
	indexer.set_pdf_layout("raw");
	indexer.set_parsing_method(term_type);
	if( stemmer ){
		indexer.set_stemming(false);
	}
	std::vector<std::string>::const_iterator fpos;
	
	unsigned int file_count = 0;
	for( fpos = filenames.begin(); fpos != filenames.end(); ++fpos ){
		if( verbose )
			std::cout << "Indexing " << *fpos << std::endl;

		++file_count;

		try {

			indexer.index( *fpos );
			
		} catch (std::exception &e){
			std::cerr << "Error indexing file: " << *fpos << " (" << e.what() << ")" << std::endl;
			continue;
		}
	}
	if(verbose){
		std::cout << std::endl << "Files: " << file_count << " Graph: " << num_vertices(g) << " vertices, " << num_edges(g) << " edges" << std::endl;
		std::cout << std::endl << "Now storing the graph...";
		std::cout.flush();
	}

	return indexer.finish();
}

int main( int argc, char *argv[]) {

	
	po::options_description opts;
	opts.add_options()
		("help", "Produce this help message\n")
		("version", "Print version information\n")
		("verbose,v", "Verbose output\n")
		("collection,c", po::value<std::string>()->default_value("My Collection"), "The name of the collection to index\n")
		("encoding,e", po::value<std::string>()->default_value("iso-8859-1"), "Set the default encoding\n")
		("term_type", po::value<std::string>()->default_value("nouns"), "Set the term type to be extracted\n")
		("document_minimum", po::value<std::string>()->default_value("1"), "Set the minimum number of times a\nterm must appear in a document to\nbe included in the index\n")
		("collection_minimum", po::value<std::string>()->default_value("3"), "Set the minimum number of times a\nterm must appear across the\ncollection to be included in the\nindex\n")
		("collection_maximum", po::value<std::string>()->default_value("0.2"), "Set the maximum document-frequency\n(between 0 and 1) for a term to be\nincluded in the index\n")
		("disable_stemmer", "Turn off the stemming of terms\n" )
		("file,f", po::value<std::string>(), "Write the term index data to a file\n")
#if SEMANTIC_HAVE_SQLITE3
		("sqlite,s", po::value<std::string>(), "The SQLite 3 database file to use.\nthe file will be created if needed\n")
#endif
#if SEMANTIC_HAVE_MYSQL
		("mysql,m", po::value<std::string>(), "The MySQL database name")
		("mysql_username,u", po::value<std::string>()->default_value(std::getenv("USER")), "The MySQL database username")
		("mysql_password,p", po::value<std::string>()->default_value(""), "The MySQL database password")
		("mysql_hostname,h", po::value<std::string>()->default_value("localhost"), "The MySQL database host\n")
#endif
//		("force,f", po::value<std::string>(), "index all documents, even if they have already been indexed" )
        ;

	po::options_description hidden_opts;
	hidden_opts.add_options()
		("directory", po::value<std::string>(), "the directory to index")
		;
		
	po::options_description all_opts;
	all_opts.add(opts).add(hidden_opts);
		
	po::positional_options_description positional_opts;
	positional_opts.add("directory",1);
	
	po::variables_map vm;
	try {
		po::store(po::command_line_parser(argc, argv).options(all_opts).positional(positional_opts).run(), vm);
		po::notify(vm);		
	} catch (po::unknown_option e) {
		std::cerr << "Error: " << e.what() << std::endl;
		usage();
	}
	
	if (vm.count("help") || !vm.count("directory")) {
		// print a help message
		usage();
	}
	
	if (vm.count("version")) {
		// print out some version information
		std::cerr << std::endl;
		print_version_info(std::cerr);
		std::cerr << std::endl;
		return 0;
	}
	
	encoding = vm["encoding"].as<std::string>();
	


	
	
	if( vm.count("mysql") && !vm.count("mysql_username")){
		std::cerr << "Error: you must supply a username for the MySQL database: " << vm["mysql"].as<std::string>() << std::endl;
	}

/* *************************************** *
 * 	GET THE FILENAMES FROM THE DIRECTORY
 * *************************************** */
	file_finder f(vm["directory"].as<std::string>());
	f.add_file_ext("html");
	f.add_file_ext("htm");
	f.add_file_ext("rtf");
	f.add_file_ext("pdf");
	f.add_file_ext("doc");
	f.add_file_ext("txt");
	std::vector<std::string> filenames = f.get_filenames();


/* ************************** *
 * 	CREATE THE GRAPH OBJECT
 * ************************** */

	if( vm.count("mysql")){
#if SEMANTIC_HAVE_MYSQL
		// check for required options
		if (!vm.count("collection") ) {
			usage();
		}

		MySQLGraph g(vm["collection"].as<std::string>());
		g.set_host(vm["mysql_hostname"].as<std::string>());
		g.set_user(vm["mysql_username"].as<std::string>());
		if (vm.count("mysql_password")) g.set_pass(vm["mysql_password"].as<std::string>());
		g.set_database(vm["mysql"].as<std::string>());

		try {
			g.connect();
		} catch (MySQLException e) {
			std::cerr << "Error during connection: " << e.what() << std::endl;
			return EXIT_SUCCESS;
		}
		g.set_mirror_changes_to_storage(true);

	 	text_indexer<MySQLGraph> indexer(g, "../share/lexicon.txt" );
		if( vm["collection_minimum"].as<std::string>().length() > 0)
			indexer.set_collection_value("min",vm["collection_minimum"].as<std::string>());
		
		if( vm["collection_maximum"].as<std::string>().length() > 0)
			indexer.set_collection_value("max",vm["collection_maximum"].as<std::string>());
		
		if( vm["document_minimum"].as<std::string>().length() > 0)
			indexer.set_collection_value("doc_min",vm["document_minimum"].as<std::string>());
		
		
		do_indexing(g, indexer, filenames, vm["term_type"].as<std::string>(), (int)vm.count("disable_stemmer"), (int)vm.count("verbose"));
#endif
	} else if ( vm.count("sqlite")){
#if SEMANTIC_HAVE_SQLITE3	
		// check for required options
		if (!vm.count("collection") ) {
			usage();
		}
		
		SQLiteGraph g(vm["collection"].as<std::string>());
		g.set_file(vm["sqlite"].as<std::string>());
		try {
			g.open();
		} catch (SQLiteException &e) {
			std::cerr << "Error opening database: " << e.what() << std::endl;
			return EXIT_SUCCESS;
		}
		g.set_mirror_changes_to_storage(true);

	 	text_indexer<SQLiteGraph> indexer(g, "../share/lexicon.txt" );
		if( vm["collection_minimum"].as<std::string>().length() > 0)
			indexer.set_collection_value("min",vm["collection_minimum"].as<std::string>());
		
		if( vm["collection_maximum"].as<std::string>().length() > 0)
			indexer.set_collection_value("max",vm["collection_maximum"].as<std::string>());

		if( vm["document_minimum"].as<std::string>().length() > 0)
			indexer.set_collection_value("doc_min",vm["document_minimum"].as<std::string>());

		do_indexing(g, indexer, filenames, vm["term_type"].as<std::string>(), (int)vm.count("disable_stemmer"), (int)vm.count("verbose"));
		
#endif	
	} else {
		// print everything out to STDOUT
		std::set<std::string> blacklist = load_stoplist("../share/stoplist_en.txt");
	 	

		text_parser parser("../share/lexicon.txt");
		parser.add_word_filter(blacklist_filter(blacklist));
	 	parser.add_word_filter(too_many_numbers_filter(6));
	 	parser.add_word_filter(minimum_length_filter(3));
	 	parser.add_word_filter(maximum_word_length_filter(15));
	 	parser.add_word_filter(maximum_phrase_length_filter(3));
		parser.set_parsing_method(vm["term_type"].as<std::string>());
		if( vm.count("disable_stemmer") ){
			parser.set_stemming(false);
		}

		file_reader reader;
		if( encoding.size() > 0 )
			reader.set_default_encoding( encoding );
		
		reader.set_pdfLayout( "raw" );
		
		std::ofstream output;
		if( vm.count("file") ){
			output.open(vm["file"].as<std::string>().c_str());
			if( !output ){
				std::cerr << "Couldn't open " << vm["file"].as<std::string>() << " for writing!" << std::endl;
				return EXIT_FAILURE;
			}
		}
		
		std::vector<std::string>::const_iterator fpos;
		for( fpos = filenames.begin(); fpos != filenames.end(); ++fpos ){
			std::string filename = *fpos;
			if( vm.count("verbose") )
				std::cerr << "Indexing " << filename << std::endl;
				
			try {
				std::string text = reader( filename );
				if( vm.count("file")){
					output << "Filename: " << filename << std::endl;
				} else {
					std::cout << "Filename: " << filename << std::endl;
				}
				std::map<std::string,int>::iterator tpos;
				std::map<std::string,int> terms = parser.parse( text );
				for( tpos = terms.begin(); tpos != terms.end(); ++tpos ){
					if( tpos->second >= atoi(vm["document_minimum"].as<std::string>().c_str())){
						if( vm.count("file")){
							output << tpos->first << ": " << tpos->second << std::endl;
						} else {
							std::cout << tpos->first << ": " << tpos->second << std::endl;
						}
					
					}
				}
				if( vm.count("file")){
					output << std::endl;
				} else {
					std::cout << std::endl;
				}
				
				

			} catch (std::exception &e){
				std::cerr << "Error indexing file: " << *fpos << " (" << e.what() << ")" << std::endl;
				continue;
			}
		}
		
		
		
		
	}
	
	if( vm.count("verbose"))
		std::cerr << "done!" << std::endl;

	
	return EXIT_SUCCESS;
	
}
	
