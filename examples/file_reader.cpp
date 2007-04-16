#include <semantic/file_reader.hpp>
#include <semantic/file_finder.hpp>
#include <semantic/tagger.hpp>

#include <string>
#include <vector>
#include <map>
#include <iostream>

using namespace semantic;



int main( int argc, char *argv[]) {

	if( argc < 2 ){
		exit(0);
	}
	
	std::string directory = argv[1];
	file_finder finder(directory);
	finder.add_file_ext("pdf");
	finder.add_file_ext("html");
	finder.add_file_ext("doc");
	finder.add_file_ext("htm");
	finder.add_file_ext("txt");
	std::vector<std::string> filenames = finder.get_filenames();
	
	tagger parser("../data/lexicon.txt");
	
	file_reader reader;
	std::vector<std::string>::iterator pos;
	for( pos = filenames.begin(); pos != filenames.end(); ++pos ){
		std::string filename = *pos;
		std::string text = reader( filename );
		// POS tag
		std::cout << filename << std::endl;
		std::cout << text << std::endl;
		std::cout << parser.add_tags( text ) << std::endl;
		
		
		//std::map<std::string,int> terms = parser.remove_tags( parser.get_noun_phrases( text ) );
		//std::map<std::string,int>::iterator tpos;
		//for( tpos = terms.begin(); tpos != terms.end(); ++tpos ){
		//	std::cout << tpos->first << ": " << tpos->second << std::endl;
		//}
		
//		parser.tokenize( text );
//		std::string tagged = parser.add_tags( text );
	}
	
	exit(EXIT_SUCCESS);
	

}
