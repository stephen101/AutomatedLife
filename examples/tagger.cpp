#include <semantic/semantic.hpp>
#include <semantic/tagger.hpp>


#include <boost/program_options/options_description.hpp>
#include <boost/program_options/positional_options.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/parsers.hpp>

#include <semantic/version.hpp>

#include <iostream>
#include <fstream>
#include <map>
#include <string>

using namespace semantic;

namespace po = boost::program_options;

#define usage() \
	std::cerr << "Usage: " << " [options] <input file>" << std::endl; \

int main( int argc, char *argv[]) {


	po::options_description opts;
	opts.add_options()
		("help,h", "produce this help message")
		("version,v", "print version information")
		;
		
	po::options_description hidden_opts;
	hidden_opts.add_options()
		("input-file", po::value<std::string>(), "the text file to parse")
		;
		
	po::options_description all_opts;
	all_opts.add(opts).add(hidden_opts);
	
	po::positional_options_description positional_opts;
	positional_opts.add("input-file",1);
	
	
	po::variables_map vm;
	try {
		po::store(po::command_line_parser(argc, argv).options(all_opts).positional(positional_opts).run(), vm);
		po::notify(vm);		
	} catch ( po::unknown_option e) {
		std::cerr << "Error: " << e.what() << std::endl;
		usage();
		return 0;
	}
	
	if (vm.count("help")){
		usage();
		std::cerr << std::endl << opts << std::endl;
		return 0;
	}
	
	if (vm.count("version")){
		std::cerr << std::endl;
		print_version_info(std::cerr);
		std::cerr << std::endl;
		return 0;
	}
	
	if (!vm.count("input-file")){
		usage();
		return 0;
	}
	
	
	// Read the contents of the file into a buffer
	std::ifstream infile ( vm["input-file"].as<std::string>().c_str(), std::ios_base::in);
	if( ! infile ){
		std::cerr << "can't open file: \"" << vm["input-file"].as<std::string>() << "\" for reading!" << std::endl;
		exit(EXIT_FAILURE);
	}
				
	
	std::string line;
	tagger parser( "../data/lexicon.txt" );

	
	std::string text;
	while (std::getline(infile, line)){
		
		
		 std::string tagged;
		 tagged = parser.add_tags(line);
		 std::cout << line << std::endl;
		 std::cout << tagged << std::endl << std::endl;
		
		
		text.append(line+" ");
		
	}
	
	std::map<std::string,int> nouns = parser.get_noun_phrases( text );
	std::map<std::string,int>::iterator pos;
	for( pos = nouns.begin(); pos != nouns.end(); ++pos ){
		std::cout << pos->first << ": " << pos->second << std::endl;
	}
	
	
	
	return EXIT_SUCCESS;
	
}
	
