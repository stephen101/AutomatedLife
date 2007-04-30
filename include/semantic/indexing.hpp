#ifndef __SEMANTIC_INDEXING_HPP__
#define __SEMANTIC_INDEXING_HPP__

/*
classes to help with indexing texts (or other things) into a Semantic Graph
*/



#include <semantic/semantic.hpp>
#include <semantic/parsing.hpp>
#include <semantic/filter.hpp>
#include <semantic/file_reader.hpp>

#include <map>
#include <sstream>
#include <string>
#include <cctype>
#include <utility>
#include <iostream>


using std::string;

namespace semantic {

    template <class Graph>
    class text_indexing_helper {
        typedef se_graph_traits<Graph> graph_traits;
        typedef typename graph_traits::vertex_descriptor Vertex;
        typedef typename graph_traits::edge_descriptor Edge;
        public:
            explicit text_indexing_helper(Graph &graph) : g(graph) {}

            std::pair<Edge, Edge> add_doc_term_edge(const string& doc_string,
                                                    const string& term_string,
                                                    unsigned int strength) {
                Vertex doc, term;
                doc = add_doc_vertex(doc_string);
                term = add_term_vertex(term_string);

                typename graph_traits::edge_properties_type eprop;
                eprop.strength = strength;

                return std::make_pair(add_edge(doc, term, eprop, g).first,
                                      add_edge(term, doc, eprop, g).first);
            }

            Edge add_single_doc_term_edge(const string& doc_string,
                                          const string& term_string,
                                          unsigned int strength) {
                Vertex doc, term;
                doc = add_doc_vertex(doc_string);
                term = add_term_vertex(term_string);

                typename graph_traits::edge_properties_type eprop;
                eprop.strength = strength;

                return add_edge(doc, term, eprop, g).first;
            }

            Graph &graph() { return g; }
            const Graph &graph() const { return g; }


        protected:
            Graph &g;

        private:
            Vertex add_term_vertex(string term) {
                if (term.find_first_of(' ') != std::string::npos) return add_phrase_vertex(term);
                return do_vertex(term, node_type_major_term, node_type_minor_term, term_cache);
            }

            Vertex add_phrase_vertex(string phrase){
                return do_vertex(phrase, node_type_major_term, node_type_minor_phrase, term_cache);
            }

            Vertex add_doc_vertex(string doc) {
                return do_vertex(doc, node_type_major_doc, node_type_minor_undef, doc_cache);
            }

            Vertex do_vertex(string content,
                             int type_major,
                             int type_minor,
                             std::map<string, Vertex> &cache) {
                if (cache.count(content)) return cache[content];
                typename graph_traits::vertex_properties_type vprop;
                vprop.content = content;
                vprop.type_major = type_major;
                vprop.type_minor = type_minor;

                Vertex v = add_vertex(vprop, g);
                cache.insert(std::make_pair<string, Vertex>(content, v));
                return v;
            }

            std::map<string, Vertex> doc_cache;
            std::map<string, Vertex> term_cache;

    };

    template <class Graph>
    class text_indexer: public text_indexing_helper<Graph> {
        typedef std::map<std::string,int> UnstemmedCount;
        typedef text_indexing_helper<Graph> base_type;

        public:
/* **************************************************** *
 *        CONSTRUCTOR
 *
 *        text_indexer<Graph> ( Graph, lexicon_filename or lexicon_istream )
 * **************************************************** */
            text_indexer(Graph & graph, const std::string& lexicon_location="")
                     : base_type(graph), parser(lexicon_location)
            {
                init();
            }
            text_indexer(Graph & graph, std::istream& lexicon_stream)
                     : base_type(graph), parser(lexicon_stream)
            {
                init();
            }

/* **************************************************** *
 *        METHODS
 *
 *        add_word_filter ( class Filter )
 * **************************************************** */
            template <class Filter>
            void add_word_filter( Filter f ){
                parser.add_word_filter( f );
            }

            void set_parsing_method(const std::string& method){
                parser.set_parsing_method(method);
            }

/* **************************************************** *
 *        index ( filename, filestream, [mime_type], [weight=1] )
 * **************************************************** */
             std::string index( const std::string& filename,
                                std::istream& filestream,
                                const std::string& mime_type,
                                const int multiplier=1 )
            {
                file_reader reader;
                if( default_encoding.size() > 0 )
                    reader.set_default_encoding( default_encoding );

                std::string text = reader( filestream, mime_type );
                smart_quotes_filter filter;
                text = filter(text);

                if( text.size() > 10 ){
                    add_to_index( filename, text, multiplier );
                }
                return text;
            }

            std::string index( const std::string& filename,
                               std::istream& filestream,
                               int multiplier=1 )
            {
                std::string mime_type = get_mime_type_from_filename( filename );
                return index( filename, filestream, mime_type, multiplier );
            }





/* **************************************************** *
 *        index ( filename, [weight=1] )
 * **************************************************** */
            std::string index( const std::string& filename, const int multiplier=1 )
            {
                file_reader reader;
                if( default_encoding.size() > 0 )
                    reader.set_default_encoding( default_encoding );

                if( pdfLayout.size() && pdfLayout != "layout")
                    reader.set_pdfLayout( pdfLayout );
				

                std::string text = reader( filename );
				if( text.size() > 10 ){
                    add_to_index( filename, text, multiplier );
                }
                return text;
            }

/* **************************************************** *
 *        index ( doc_id, text, [weight=1] )
 * **************************************************** */
            std::string index( const std::string& doc_id,
                               const std::string& text,
                               const int multiplier=1){
                std::string m_text;
                std::string m_encoding;
                if ( default_encoding.size() > 0 ){
                    m_encoding = default_encoding;
                } else {
                    m_encoding = "iso-8859-1";
                }
                if( m_encoding != "utf8"){
                    m_text = convert_to_utf8(text,m_encoding);
                } else {
                    m_text = text;
                }
                smart_quotes_filter filter;
                add_to_index(doc_id, filter(m_text), multiplier);
                return m_text;
            }



/* **************************************************** *
 *        unindex ( doc_id )
 * **************************************************** */
            void unindex( const std::string& id ){
                typename se_graph_traits<Graph>::vertex_descriptor u =
                        base_type::g.vertex_by_id(
                            base_type::g.fetch_vertex_id_by_content_and_type(
                                id, node_type_major_doc
                            ) );
                clear_vertex(u,base_type::g);
            }


/* **************************************************** *
 *        reindex ( filename, [weight=1] )
 * **************************************************** */
            std::string reindex( const std::string& filename, const int multiplier=1){
                file_reader reader;
                if( default_encoding.size() > 0 )
                    reader.set_default_encoding( default_encoding );

                if( pdfLayout.size() && pdfLayout != "layout")
                    reader.set_pdfLayout( pdfLayout );

                std::string text = reader( filename );
                std::string existing_text =
                        base_type::g.get_vertex_meta_value(
                            base_type::g.vertex_by_id(
                                base_type::g.fetch_vertex_id_by_content_and_type(
                                    filename, node_type_major_doc
                                ) ), "body");
                if( existing_text != text ){
                    unindex(filename);
                    index( filename, text, multiplier );
                }
                return text;
            }


/* **************************************************** *
 *        reindex ( doc_id, text, [weight=1] )
 * **************************************************** */
            std::string reindex( const std::string& doc_id,
                                 const std::string& text,
                                 const int multiplier=1){
                std::string existing_text =
                        base_type::g.get_vertex_meta_value(
                            base_type::g.vertex_by_id(
                                base_type::g.fetch_vertex_id_by_content_and_type(
                                    doc_id, node_type_major_doc
                                ) ), "body");
                if( existing_text != text ){
                    unindex(doc_id);
                    index( doc_id, text, multiplier );
                }
                return text;
            }


/* **************************************************** *
 *        prune_wordlist(min=2)
 *
 *        This will prune out less common unstemmed variations of each word
 *        Any word occurring less often than 'min' will be omitted from the
 *        data structure
 * **************************************************** */
            std::map<std::string,
                     std::pair<std::string,int> > prune_wordlist(const int min_occurrence=2){

                std::map<std::string, std::pair<std::string,int> > new_wordlist;
                typedef std::multimap< int, std::string, std::greater<int> > RevMap;

                std::map<std::string,UnstemmedCount>::iterator pos;
                for( pos = wordlist.begin(); pos != wordlist.end(); ++pos ){
                    std::string stemmed = pos->first;
                    UnstemmedCount unstemmed = pos->second;

                    RevMap lookup;
                    int total = wordlist[stemmed]["__count"];
                    UnstemmedCount::iterator cpos;
                    for( cpos = unstemmed.begin(); cpos != unstemmed.end(); ++cpos ){
                        if( cpos->first != "__count" && total >= min_occurrence ){
                            lookup.insert( make_pair( cpos->second, cpos->first ) );
                        }
                    }
                    if( lookup.size() >= 1 ){
                        std::string word = lookup.begin()->second;
                        std::string lower(word);
                        std::transform(word.begin(),word.end(),lower.begin(),tolower);

                        // skip words that don't have differing stems
                        if( lower != stemmed ){
                            new_wordlist.insert(
                                std::make_pair(stemmed, std::make_pair(word, total) ) );
                        }
                    }
                }
                return new_wordlist;
            }


/* **************************************************** *
 *        store_wordlist(int min=2)
 *
 *        this method takes the wordlist, prunes out the less
 *        commonly occurring variations of words and puts
 *        them into the node_meta table; any words occurring
 *        less often than the 'min' value are excluded
 *
 * **************************************************** */
    void store_wordlist(int min=2){

        typename se_graph_traits<Graph>::vertex_descriptor u;

        std::map<std::string,
                  std::pair<std::string,int>
            > my_wordlist = prune_wordlist(min);
        std::map<std::string, std::pair<std::string, int> >::iterator pos;
        for( pos = my_wordlist.begin(); pos != my_wordlist.end(); ++pos ){
            std::string stem = pos->first;
            std::pair<std::string,int> termCount = pos->second;

            std::string word = termCount.first;
            int count = termCount.second;

            try {
                u = base_type::g.vertex_by_id(
                         base_type::g.fetch_vertex_id_by_content_and_type(
                             stem, node_type_major_term));
                // attach the text
                std::ostringstream oss;
                oss << count;
                word.append(":" + oss.str());
                std::string prev = base_type::g.get_vertex_meta_value(u, "term");
                if( prev.size() > 0 ){
                    std::string::size_type pos = prev.find_last_of(":");
                    if( pos < prev.size() - 1){
                        int prev_cnt = atoi(prev.substr(pos+1,prev.size()-pos-1).c_str());
                        if( prev_cnt > count ){
                            word = prev;
                        }
                    }
                }
                base_type::g.set_vertex_meta_value(u, "term", word);
            } catch ( std::exception &){
                continue;
			} catch ( char * e ){
				std::cerr << "Error: " << e << std::endl;
				continue;
			}
        }
    }

            std::string get_collection_value(const std::string key,
                                             const std::string default_value){
                return base_type::g.get_meta_value(key, default_value);
            }

            void set_collection_value(const std::string key, const std::string& value){
                base_type::g.set_meta_value(key,value);
            }

/* **************************************************** *
 *         commit_changes_to_storage()
 * **************************************************** */
            void commit_changes_to_storage(){
                base_type::g.commit_changes_to_storage();
            }

/* **************************************************** *
 *         finish( min=2 )
 * **************************************************** */
            bool finish(int min=2){

                try {
                    base_type::g.commit_changes_to_storage();
                } catch ( std::exception &e){
                    std::cerr << "Error Indexing to Database: " << e.what() << std::endl;
                    return false;
				} catch ( char * e ){
					std::cerr << "Error: " << e << std::endl;
					return false;
				}

                try {
                     store_wordlist(min);
                }    catch ( std::exception &e ) {
                    std::cerr << "Error storing wordlist" << e.what() << std::endl;
				} catch ( char * e ){
					std::cerr << "Error: " << e << std::endl;
				}


                std::map<std::string,std::string>::iterator pos;
                if( storeText ){
                    for( pos = text_store.begin(); pos != text_store.end(); ++pos ){
                        std::string filename = pos->first;
                        std::string text = pos->second;

                        typename se_graph_traits<Graph>::vertex_descriptor u;
                        try {
                            u = base_type::g.vertex_by_id(
                                    base_type::g.fetch_vertex_id_by_content_and_type(
                                        filename, node_type_major_doc));
                            // attach the text
                            base_type::g.set_vertex_meta_value(u, "body", text);
                        } catch ( std::exception &){
                            //std::cerr << "Error: " << e.what() << std::endl;
                            continue;
						} catch ( char * e ){
							std::cerr << "Error: " << e << std::endl;
							continue;
						}

                    }
                }
                return true;
            }


/* **************************************************** *
 *        set_default_encoding( std::string& encoding )
 * **************************************************** */
            void set_default_encoding( const std::string& encoding )
            {
                default_encoding = encoding;
            }

            void set_pdf_layout( const std::string& layout){
                pdfLayout = layout;
            }

            void set_stemming(bool val){
                parser.set_stemming(val);
            }

            void store_text(bool val){
                storeText = val;
            }
            std::string pdfLayout;
            std::map<std::string,std::string> text_store;


        private:
            std::map<std::string,UnstemmedCount> wordlist;
            text_parser parser;
            std::string default_encoding;
            int files_indexed;
            bool storeText;

            void init(){
                pdfLayout = "layout";
                files_indexed = 0;
                storeText = true;
                text_store.clear();
            }

            void add_to_index( const std::string& doc_id,
                               const std::string& text,
                               const int multiplier )
            {
                files_indexed++;
                //std::cout << "adding: " << doc_id << " => " << text << std::endl;
                if( storeText ){
                    text_store[doc_id] = text;
                }
				std::map<std::string,int> terms = parser.parse( text, wordlist );
                std::map<std::string,int>::iterator tpos;
				
				std::string value = base_type::g.get_meta_value("doc_min","1");
                int min = atoi(value.c_str());
				for( tpos = terms.begin(); tpos != terms.end(); ++tpos ){
                    std::string term = tpos->first;
                    if( tpos->second >= min ){
                        unsigned int weight = tpos->second * multiplier;
                        // std::cout << "adding: " << doc_id << " - " << term << std::endl;
                        base_type::add_doc_term_edge( doc_id, term, weight );
                    }
                }

//                if( !(files_indexed % 5000 ) ){
//                    manage_wordlist();
//                }
            }


/*
            void manage_wordlist(const unsigned int max=3)
            {
                typedef std::multimap<int,std::string,std::less<int> > RevMap;
                std::multimap<std::string,std::string> to_delete;
                std::multimap<std::string,UnstemmedCount>::iterator pos;
                for( pos = wordlist.begin(); pos != wordlist.end(); ++pos ){
                    std::string stemmed = pos->first;
                    UnstemmedCount unstemmed = pos->second;

                    RevMap lookup;
                    if( unstemmed.size() > max + 1 ){
                        UnstemmedCount::iterator cpos;
                        for( cpos = unstemmed.begin(); cpos != unstemmed.end(); ++cpos ){
                            if( cpos->first != "__count" ){
                                lookup.insert( make_pair( cpos->second, cpos->first ) );
                            }
                        }
                        RevMap::iterator spos;
                        unsigned int i = (int)lookup.size();
                        for( spos = lookup.begin(); spos != lookup.end(); ++spos ){
                            if( i-- > max ){
                                to_delete.insert( make_pair( stemmed, spos->second ) );
                            }
                        }
                    }
                }
                std::map<std::string,std::string>::iterator dpos;
                for( dpos = to_delete.begin(); dpos != to_delete.end(); ++dpos ){
                    wordlist[dpos->first].erase(dpos->second);
                }
            }

*/
			std::string get_mime_type_from_filename( const std::string& filename )
            {
                std::string::size_type pos = filename.find_last_of(".");
                std::string ext = filename.substr(pos+1,filename.size()-pos-1);
                if( ext == "doc" ){
                    return "application/msword";
                } else if ( ext == "rtf" ){
                    return "application/rtf";
                } else if ( ext == "html" || ext == "htm" ){
                    return "text/html";
                } else if ( ext == "txt" ){
                    return "text/plain";
                } else {
                    std::cerr << "Couldn't determine MIME type from filename: "
                              << filename << std::endl
                              << "Trying \"text/plain\"..." << std::endl;
                    return "text/plain";
                }
            }



    };


} // namespace semantic



#endif



