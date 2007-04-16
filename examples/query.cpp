#include <semantic/query.hpp>
#include <string>
#include <iostream>
#include <vector>

int main (){

	std::string search_str = "   \"This     is   \" a   sample   query  +	 string ";
	
	
	std::cout << "Search query: " << search_str << std::endl;
	
	semantic::search_query query(search_str);
	std::vector<std::string>::iterator pos;

	
	std::vector<std::string> terms = query.tokenize();
	std::cout << std::endl << "Stemmed terms" << std::endl;
	for( pos = terms.begin(); pos != terms.end(); ++pos ){
		std::cout << *pos << std::endl;
	}


	query.set_stemming(false);
	std::vector<std::string> raw = query.tokenize();
	std::cout << std::endl << "Unstemmed terms" << std::endl;
	for( pos = raw.begin(); pos != raw.end(); ++pos ){
		std::cout << "'" << *pos << "'" << std::endl;
	}
	return EXIT_SUCCESS;
}
