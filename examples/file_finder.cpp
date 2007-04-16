#include <semantic/file_finder.hpp>

#include <string>
#include <vector>
#include <iostream>

using namespace semantic;



int main( int argc, char *argv[]) {

	if( argc < 2 ){
		exit(0);
	}
	
	std::string directory = argv[1];
	file_finder finder(directory);
	finder.add_file_ext("pdf");
	finder.add_file_ext("rtf");
	finder.add_file_ext("doc");
	finder.add_file_ext("htm");
	finder.add_file_ext("html");
	finder.add_file_ext("txt");
	
	std::vector<std::string> filenames = finder.get_filenames();
	std::vector<std::string>::iterator pos;
	for( pos = filenames.begin(); pos != filenames.end(); ++pos ){
		std::cout << *pos << std::endl;
	}
	
	exit(EXIT_SUCCESS);
	

}
