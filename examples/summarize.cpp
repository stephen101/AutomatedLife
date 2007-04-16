#include <iostream>
#include <vector>
#include <string>
#include <semantic/summarization.hpp>

int main(int argc, char *argv){
	
	std::map<std::string,double> top_words;
	top_words["chang"] = 1.323;
	top_words["call"] = 1.01;
	
	std::string text;
	text = "  Mr. Alfred P.     Johns has $1.42 in change.  He then called   out to a neighbor, \"hi.\"     ";
	text.append( " (But then he was quiet.)   Silently, he walked past J.C. Penney toward home.    ");
	
	
	semantic::summarizer sum(top_words);
	std::vector<std::string> summary = sum.summarize(text);
	
	
	std::vector<std::string>::iterator pos;
	for( pos = summary.begin(); pos != summary.end(); ++pos ){
		std::cout << *pos << std::endl;
	}
	
	return EXIT_SUCCESS;
}
