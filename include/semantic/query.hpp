

#ifndef __SEMANTIC_QUERY_HPP__
#define __SEMANTIC_QUERY_HPP__

#include <string>
#include <vector>
#include <set>
#include <cctype>
//#include <iostream>

#include <semantic/semantic.hpp>
#include <semantic/stem/english_stem.h>


namespace semantic {

	class search_query {
		public:
			template <class Graph>
			search_query(const std::string& str, Graph &g) : query_string(str) {
				// get the phrase length from the graph

				std::string phrase_length_str = g.get_meta_value("max_phrase_length", "1");
				phrase_length = atoi(phrase_length_str.c_str());
				// parse!
				parse();
				
				set_stemming(true);
			}
			
			std::string as_string(){
				return query_string;
			}

			std::vector<std::string> tokenize(){
				if( stemming ){
					return stemmed_terms;
				} else {
					return raw_terms;
				}
			}
			
			void set_stemming(const bool stem){
				stemming = stem;
			}
			
			
			template <class Graph>
			std::vector<typename se_graph_traits<Graph>::vertex_id_type>
			get_vertex_ids(Graph &g)  {
				std::vector<typename se_graph_traits<Graph>::vertex_id_type> node_ids;
				
				std::vector<std::string> terms = tokenize();
				std::vector<std::string>::iterator pos;
				for( pos = terms.begin(); pos != terms.end(); ++pos ){
					
					try {
						node_ids.push_back( g.fetch_vertex_id_by_content_and_type(*pos, node_type_major_term) );
					//	std::cout << "Vertex found: " << *pos << std::endl;
					} catch ( std::exception & ){
						// vertex not found.
						//std::cerr << "Vertex not found: " << e.what() << std::endl;
						continue;
					}
				}
				
				return node_ids;
			}
			

			
		
		
		private:
			const std::string query_string;
			bool stemming;
			int phrase_length;
			std::vector<std::string> stemmed_terms;
			std::vector<std::string> raw_terms;
				
			void parse(){
			
				stemming::english_stem Stemmer;
				
				std::vector<std::string> non_quotes;
				std::string my_query(query_string);
				std::string lower(my_query);
				std::transform(my_query.begin(),my_query.end(),lower.begin(),tolower);
				my_query = lower;
				
				// replace any delimiters with spaces
				std::string::size_type dpos1 = my_query.find_first_of("+\t,", 0);
				std::string::size_type dpos2 = my_query.find_first_not_of("+\t,", dpos1);
				
				while( dpos1 != std::string::npos ){
					if( dpos2 != std::string::npos ){
						my_query.replace(dpos1, dpos2-dpos1, " ");
					} else {
						my_query.replace(dpos1, 1, " ");
					}
					dpos1 = my_query.find_first_of("+\t,", 0);
					dpos2 = my_query.find_first_not_of("+\t,", dpos1);
				}
				
				// treat anything in quotes as a phrase
				std::string::size_type lastPos = 0;
				std::string::size_type pos1 = my_query.find_first_of('"', 0);
				std::string::size_type pos2 = my_query.find_first_of('"', pos1+1);
				while( pos1 != std::string::npos ){

					// don't forget about any terms appearing before the quotes
					if( pos1 > lastPos ){
						non_quotes.push_back( my_query.substr(lastPos,pos1-lastPos) );
					}
					
					if( pos2 != std::string::npos ){
						std::string phrase = my_query.substr(pos1+1, pos2-pos1-1);
						std::string::size_type ws = phrase.find("  ", 0);
						while( ws != std::string::npos ){
							phrase.erase(ws,1);
							ws = phrase.find("  ", ws);
						}
						if( phrase.substr(0,1) == " " ){
							phrase.erase(0,1);
						}
						if( phrase.substr(phrase.size()-1,1) == " " ){
							phrase.erase(phrase.size()-1,1);
						}
						

						raw_terms.push_back( phrase );
						
						// stem 
						Stemmer(phrase);
						stemmed_terms.push_back(phrase);
						
						// go to the next quote
						lastPos = pos2+1;
						pos1 = my_query.find_first_of('"', pos2+1);	
						pos2 = my_query.find_first_of('"', pos1+1);

					} else {
						pos1 = std::string::npos;
						non_quotes.push_back( my_query.substr(pos1+1,my_query.size()-pos1-1) );
					}
				}

				// pick up any terms appearing after the quotes
				if( lastPos < my_query.size() - 1 ){
					non_quotes.push_back( my_query.substr(lastPos, my_query.size()-lastPos) );
				}

				std::vector<std::string>::iterator pos;
				for( pos = non_quotes.begin(); pos != non_quotes.end(); ++pos ){
					std::string raw_string = *pos;
					pos1 = raw_string.find_first_not_of(" ",0);
					pos2 = raw_string.find_first_of(" ",pos1);
					std::vector<std::string> terms;
					std::vector<std::string> stems;
					
					while( pos1 != std::string::npos ){
						
						if( pos2 == std::string::npos ){
							std::string term = raw_string.substr(pos1,raw_string.size()-pos1);
							terms.push_back( term );
							Stemmer(term);
							stems.push_back(term);

							pos1 = std::string::npos;
							
						} else {
							std::string term = raw_string.substr(pos1,pos2-pos1);

							terms.push_back( term );
							Stemmer(term);
							stems.push_back(term);
							
							pos1 = raw_string.find_first_not_of(" ", pos2);
							pos2 = raw_string.find_first_of(" ", pos1);
						
						}
					}
					
					for( int i = 0; i < phrase_length; ++i ){
						for( unsigned int j = 0; j+i < terms.size(); ++j ){
							std::string term;
							for( int k = 0; k < i; ++k ){
								term.append(terms[j+k] + " ");
							}
							raw_terms.push_back( term + terms[j+i] );
							stemmed_terms.push_back( term + stems[j+i] );
						}
					}
				}
			}


	};

}

#endif
