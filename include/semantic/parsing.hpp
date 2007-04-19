
#ifndef __SEMANTIC_PARSING_HPP__
#define __SEMANTIC_PARSING_HPP__

#include <semantic/tagger.hpp>
#include <semantic/stem/english_stem.h>
#include <semantic/filter.hpp>
#include <boost/shared_ptr.hpp>

#include <map>
#include <vector>
#include <string>
#include <cctype>
#include <iostream>

namespace semantic {

/* *******************************************************
        TEXT PARSER
   ******************************************************* */
    class text_parser {

        public:
            typedef std::map<std::string,int> UnstemmedCount;

            
            text_parser( const std::string &lexicon_location="" )
                : parser(lexicon_location) {
                enable_stemming = true;
            }
            text_parser( std::istream& lexicon_stream )
                : parser( lexicon_stream ){
                enable_stemming = true;
            }
            

/* ************************************************************************
    set_parsing_method( std::string ) -- set how the POS tagger gets words
            the value can be:     'noun_phrases', 'adjectives', 'verbs',
                                'proper_nouns', or 'nouns' (default)
   ************************************************************************ */
            
            void set_parsing_method( const std::string& pattern){
                POS_pattern = pattern;
            }
            


/* ************************************************************************
    parse ( stream ) -- parse a text stream, returning stemmed terms
   ************************************************************************ */
            std::map<std::string,int> parse( std::istream& instream ){
                std::map<std::string,UnstemmedCount> unstemmed;
                return parse( instream, unstemmed );
            }

/* ************************************************************************
    parse ( stream, unstemmed_lookup ) -- parse a text stream, returning stemmed terms
   ************************************************************************ */
            std::map<std::string,int> parse(
                           std::istream& instream,
                           std::map<std::string,UnstemmedCount>& unstemmed ){
                std::map<std::string,int> words;
                std::string line;
                while( std::getline(instream,line) ){
                    std::map<std::string,int> mywords = parse( line, unstemmed );
                    std::map<std::string,int>::iterator pos;
                    for( pos = mywords.begin(); pos != mywords.end(); ++pos ){
                        words[pos->first] += pos->second;
                    }
                }
                return words;
            }

/* ************************************************************************
    parse ( string ) -- parse a text string, returning stemmed terms
   ************************************************************************ */
            std::map<std::string,int> parse( const std::string& intext ){
                std::map<std::string,UnstemmedCount> unstemmed;
                return parse( intext, unstemmed );
            }


/* ************************************************************************
    parse ( string, unstemmed_lookup ) -- parse a text string, returning
                                          stemmed terms, keeping original
                                          terms in the unstemmed_lookup
   ************************************************************************ */
            std::map<std::string,int> parse(
                          const std::string& intext,
                          std::map<std::string,UnstemmedCount>& unstemmed ) {

                // POS tag
                std::map<std::string,int> terms;

                try {
                    if( POS_pattern == "noun_phrases"){
                        terms = parser.get_noun_phrases( intext );
                    } else if ( POS_pattern == "adjectives"){
                        terms = parser.get_adjectives( intext );
                    } else if ( POS_pattern == "maximal_noun_phrases"){
                        terms = parser.get_maximal_noun_phrases( intext );
                    } else if ( POS_pattern == "proper_nouns") {
                        terms = parser.get_proper_nouns( intext );
                    } else if ( POS_pattern == "verbs") {
                        terms = parser.get_verbs( intext );
                    } else {
                        terms = parser.get_nouns( intext );
                    }
                    terms = parser.remove_tags( terms );

                } catch ( std::exception& e ){
                    std::string POS = POS_pattern;
                    if( POS.length() == 0 ){
                        POS = "nouns";
                    }
                    std::cerr << "Error getting " << POS << ": " << e.what() << std::endl;
                }

                // pass through a word filter
                
                
                for( std::vector<WordPtr>::iterator ptr = word_filters.begin();
                         ptr != word_filters.end(); ++ptr ){
                     WordPtr filter = *ptr;
                
                    std::vector<std::string> to_erase;  // list of terms to erase
                    for( std::map<std::string,int>::iterator i = terms.begin();
                             i != terms.end(); ++i ){
                        std::string key = i->first;
                        if( ! (*filter)(key) ){ // check filter, erase if false
                            to_erase.push_back( key );
                        }
                    }
                
                    // actually do the deletions
                    for( std::vector<std::string>::iterator i = to_erase.begin();
                             i != to_erase.end(); ++i ){
                        terms.erase(*i);
                    }
                }
                

                return stem ( terms, unstemmed );
            }




/* ************************************************************************
    add_word_filter ( class word_filter ) -- add a word-based filter
                                             to the parser
   ************************************************************************ */
            
            template <class Filter>
            void add_word_filter(Filter f) {
                word_filters.push_back( WordPtr( new Filter(f) ) );
            }
            

            
            void set_stemming(bool val){
                enable_stemming = val;
            }
            

        private:
            typedef boost::shared_ptr<word_filter> WordPtr;

            tagger parser;
            bool enable_stemming;
            std::string POS_pattern;
            std::vector<WordPtr> word_filters;
            std::map<std::string,int> unstemmed;


/* ************************************************************************
    stem ( terms, unstemmed_lookup ) -- stem the words in `terms` map
                                        and keep the original word forms
                                        in the `unstemmed_lookup`
   ************************************************************************ */
            std::map<std::string,int> stem(
                        const std::map<std::string,int>& terms,
                        std::map<std::string,UnstemmedCount>& unstemmed ){

                stemming::english_stem Stemmer;
                std::map<std::string,int> stemmed;
                std::map<std::string,int>::const_iterator pos;
                for( pos = terms.begin(); pos != terms.end(); ++pos ){
                    std::string word( pos->first );
                    std::string original(word);
                    if( enable_stemming ){
                        Stemmer(word);
                    }
                    if( word.length() > 0 ){

                        // Make lower case
                        std::string lower(word);
                        std::transform(word.begin(),word.end(),lower.begin(),tolower);

                        stemmed[lower] += pos->second;
                        unstemmed[lower][original]++;
                        unstemmed[lower]["__count"]++;
                    }
                }
                return stemmed;

            }




    };




}

#endif

