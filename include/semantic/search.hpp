#ifndef __SEMANTIC_SEARCH_HPP__
#define __SEMANTIC_SEARCH_HPP__

#include <semantic/subgraph.hpp>
#include <semantic/utility.hpp>
#include <semantic/query.hpp>
#include <semantic/weighting/idf.hpp>
#include <semantic/weighting/tf.hpp>
#include <semantic/weighting/lg.hpp>
#include <semantic/pruning.hpp>
#include <semantic/subgraph/pruning_random_walk.hpp>
#include <semantic/ranking/spreading_activation.hpp>
#include <semantic/summarization.hpp>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/iteration_macros.hpp>

#include <map>
#include <vector>
#include <string>
#include <iostream>


namespace semantic {
	
	
	template<class Graph>
	class search {
		public:
		typedef std::vector<std::pair<std::string,double> > sorted_results;
		typedef std::pair<sorted_results,sorted_results> search_results;
		
		search(Graph &g, const int unstem=1) : g(g) {
			if( unstem == 1){
				stemming = true;
			} else {
				stemming = false;
			}
		}
		
		
		std::string unstem_term(const std::string &stem){
			if( !stemming ){
				return stem;
			}
			
			typename se_graph_traits<Graph>::vertex_descriptor u;
			u = g.vertex_by_id(
                     g.fetch_vertex_id_by_content_and_type(
                         stem, node_type_major_term));

			std::string term = g.get_vertex_meta_value(u, "term");
            std::string::size_type pos = term.find_last_of(":");
			if( pos != std::string::npos && pos > 0 ){
				std::string word = term.substr(0,pos);
				return word;
            } else {
				return stem;
			}
		}


/* ************************************* *
 * 		Spreading activation search 
 * ************************************* */
		search_results semantic(const std::string &q_string){
			typedef se_graph_traits<Graph> traits;
			
			g.clear();

			// parse the query
			search_query query(q_string, g);
			std::vector<std::string> q_vector = query.tokenize();
			typename std::vector<typename traits::vertex_id_type> ids = query.get_vertex_ids(g);
			
			return do_search(ids);
		}
		
/* ************************************* *
 * 		Keyword search 
 * ************************************* */
		search_results keyword(const std::string &q_string){
			typedef se_graph_traits<Graph> traits;
						
			// parse the query
			g.clear();
			search_query query(q_string, g);
			query.set_stemming(stemming);
			std::vector<std::string> q_vector = query.tokenize();
			typename std::vector<typename traits::vertex_id_type> ids = query.get_vertex_ids(g);
			g.expand_vertices(ids.begin(), ids.end());
			
			
			
			// set the node list
			std::map<unsigned long, double> nodes;

			typename std::vector<typename traits::vertex_id_type>::const_iterator pos;
			for( pos = ids.begin(); pos != ids.end(); ++pos){
				nodes[*pos] = (double)pow((double)10,10); // starting energy on search
			}
			
			return do_ranking(nodes);
		}
		
		search_results do_better_search(const std::string &q_string){
			typedef se_graph_traits<Graph> traits;
			typedef typename traits::vertex_descriptor Vertex;
			typedef std::vector<typename traits::vertex_id_type> vertices;
			typedef typename traits::vertex_properties_type vertex_properties;
			g.clear();
			
			// process the query
			search_query query(q_string, g);
			std::vector<std::string> q_vector = query.tokenize();
			vertices ids = query.get_vertex_ids(g);
				
			// expand vertices for each term node and populate the 'intersection' map
			std::map<typename traits::vertex_id_type,unsigned int> intersection = g.get_intersection(ids.begin(), ids.end());
			
			// read through the 'intersection' map, and record each node that is adjacent
			// to at least two terms; also record any nodes that contain all the terms
			vertices all_common;
			vertices some_common;
			typename vertices::size_type term_count = ids.size();
			for ( typename std::map<typename traits::vertex_id_type,unsigned int>::iterator i = intersection.begin(); i != intersection.end(); ++i ){
				if( i->second > 1){
					some_common.push_back(i->first);
					if( i->second == term_count ){
						all_common.push_back(i->first);
					}
				}
			}
			
			// now add the nodes to the search vector
			vertices search_nodes;
			if( all_common.size() >= 1 ){
				search_nodes = all_common;
			} else {
				search_nodes = some_common;
				for( typename vertices::iterator i = ids.begin(); i != ids.end(); ++i ){
					search_nodes.push_back( *i );
				}
			}

			return do_search(search_nodes);
		}
		
/* ************************************* *
 * 		Find similar (using document names) 
 * ************************************* */
		search_results similar(const std::string s){
		    return similar(&s, (&s)+1);
		}
		
		
		
		template <class Iterator> // iterator of strings
		search_results similar(Iterator i, Iterator i_end) {
		    typedef se_graph_traits<Graph> traits;
			g.clear();
			std::vector<typename traits::vertex_id_type> vertices;
			for(; i != i_end; ++i ){
				try {
				    vertices.push_back( g.fetch_vertex_id_by_content_and_type(*i, node_type_major_doc) );
			    } catch (std::exception &e) {
			        // something didn't work! oh well...
			    }
			}
			return do_search(vertices);
		}
		

		std::pair<typename weighting_traits<Graph>::edge_weight_map,
					typename weighting_traits<Graph>::vertex_weight_map> get_weight_map(){
			return std::make_pair(m_edge_weights, m_rank_map);
		}

/* ************************************ *
 * 		Get summary terms
 * ************************************ */
		std::map<std::string,double> get_terms_for_summary(){
			return stemmed_terms;
		}
		
		std::string get_document_text(const std::string doc_id){
			return g.get_vertex_meta_value(g.vertex_by_id(g.fetch_vertex_id_by_content_and_type(doc_id, node_type_major_doc)), "body");
		}
		
/* ************************************ *
 *		Summarize this document
 * ************************************ */
		std::map<std::string,std::string> summarize_documents(const sorted_results &docs, const int length=3){
			std::map<std::string,std::string> summaries;
			sorted_results::const_iterator pos;
			for( pos = docs.begin(); pos != docs.end(); ++pos){
				std::string filename = pos->first;
				summaries[filename] = summarize_document(filename,length);
			}
			return summaries;
		}

		std::string summarize_text(const std::string& text, const int length=3){
			summarizer summer(stemmed_terms);
			std::vector<std::string> summaries = summer.summarize(text,length);
			std::vector<std::string>::iterator pos;
			std::string summary;
			unsigned int i = 0;
			for( pos =  summaries.begin(); pos != summaries.end(); ++pos, ++i ){
				summary.append(*pos);
				if( i < summaries.size()-1){
					summary.append(" ");
				}
			}
			
			return summary;
		}
		
		std::string summarize_document(const std::string& id, const int length=3){
			summarizer summer(stemmed_terms);
			
			std::string text = g.get_vertex_meta_value(g.vertex_by_id(g.fetch_vertex_id_by_content_and_type(id, node_type_major_doc)), "body");
			std::vector<std::string> summaries = summer.summarize(text,length);
			std::vector<std::string>::iterator pos;
			std::string summary;
			unsigned int i = 0;
			for( pos =  summaries.begin(); pos != summaries.end(); ++pos, ++i ){
				summary.append(*pos);
				if( i < summaries.size()-1){
					summary.append(" ");
				}
			}
			
			return summary;
		}

				
		
			
		private:
			Graph &g;
			bool stemming;
			std::map<std::string,double> stemmed_terms;
			typedef std::multimap<double,std::string,std::greater<double> > m_sorted_results;
			typedef weighting_traits<Graph> wtraits;
			
			typename weighting_traits<Graph>::edge_weight_map m_edge_weights;
			typename weighting_traits<Graph>::vertex_weight_map m_rank_map;
				

/* ******************************** *
 * 		Do the actual searching 
 * ******************************** */
			search_results do_search(const std::vector<typename se_graph_traits<Graph>::vertex_id_type> &ids){
				typedef se_graph_traits<Graph> traits;
				
				// set the node list
				std::map<unsigned long, double> nodes;
				
				typename std::vector<typename traits::vertex_id_type>::const_iterator pos;
				for( pos = ids.begin(); pos != ids.end(); ++pos){
					nodes[*pos] = pow((double)10,10); // starting energy on search
				}
				
				try {
					g.fetch_subgraph_starting_from( ids.begin(), ids.end() );
				} catch ( std::exception &e ){
					std::cout << "Vertex not found: " << e.what() << std::endl;
				}
				
				return do_ranking(nodes);
			}
			
			
			search_results do_ranking(std::map<unsigned long, double> &nodes) {
				typedef se_graph_traits<Graph> traits;
				typename wtraits::vertex_weight_map rank_map;
				typename wtraits::edge_weight_map weights;

				g.populate_weight_map(boost::make_assoc_property_map(weights));
				spreading_activation(g, nodes, boost::make_assoc_property_map(weights), boost::make_assoc_property_map(rank_map));

				// output le rankmap
			
				m_sorted_results ranked_docs, ranked_terms;
			
				BGL_FORALL_VERTICES_T(u, g, Graph) {
					if (g[u].type_major == node_type_major_doc){
						ranked_docs.insert(std::make_pair(rank_map[u],g[u].content));
					} else {
						ranked_terms.insert(std::make_pair(rank_map[u],g[u].content));
					}
				}
			    m_edge_weights = weights;
				m_rank_map = rank_map;
				
				m_sorted_results::iterator mpos;
				sorted_results docs_list, terms_list;
				stemmed_terms.clear();
								
				for( mpos = ranked_docs.begin(); mpos != ranked_docs.end(); ++mpos){
					double relevance = 1 + 10 * log10(1+mpos->first);
					if(relevance>100)
						relevance--;
					docs_list.push_back(std::make_pair(mpos->second,relevance));
				}
				for( mpos = ranked_terms.begin(); mpos != ranked_terms.end(); ++mpos){
					double relevance = 1 + 10 * log10(1+mpos->first);
					if(relevance>100)
						relevance--;
					
					stemmed_terms.insert(std::make_pair(mpos->second,relevance));
					std::string unstemmed = unstem_term(mpos->second);
					terms_list.push_back(std::make_pair(unstemmed,relevance));
				}
			
				return std::make_pair(docs_list, terms_list);
			}


			
	}; // class search
	
} // namespace semantic

#endif
