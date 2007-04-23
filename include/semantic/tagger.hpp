
#ifndef __SEMANTIC_TAGGER_HPP__
#define __SEMANTIC_TAGGER_HPP__


#include <semantic/semantic.hpp>
#include <semantic/utility.hpp>

#include <semantic/abbreviations.hpp>
#include <boost/algorithm/string.hpp>
#include <cctype>


// STL Includes
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <set>

namespace semantic {
    class tagger {

        public:

/* *********************************************************************
 *        Generic constructor
 *********************************************************************** */
            
            tagger( std::istream& lexicon_stream )
            {
                init();
                load_lexicon( lexicon_stream );
            }
            
            tagger( const std::string& lexicon_location="" )
            {
                init();
                load_lexicon( lexicon_location );
            }
            

/* *********************************************************************
 *        Separate words and punctuation in preparation for tagging
 *********************************************************************** */
            
            std::vector<std::string> tokenize(const std::string& text)
            {
                std::string s(text);
            
                /* load the standard set of abbreviations */
                Abbreviations abbrs;
            
                
                // Remove any HTML-like tags that sneaked in
                std::string::size_type htmlPos = s.find_first_of("<",0);
                std::string::size_type lastHtmlPos = s.find_first_of(">",htmlPos);
                while( htmlPos != std::string::npos && lastHtmlPos != std::string::npos ){
                    s.replace(htmlPos,lastHtmlPos-htmlPos+1," ");
                    htmlPos = s.find_first_of("<",htmlPos+1);
                    lastHtmlPos = s.find_first_of(">",htmlPos);
                }
                
            
                
                // Simple tokenization on whitespace
                std::vector<std::string> coll;
                std::string delim = "\x20\x09\x0a\x0c\x0d\xa0\x85";
                std::string::size_type lastPos = s.find_first_not_of(delim,0);
                std::string::size_type startPos = s.find_first_of(delim,lastPos);
                
                while( startPos != std::string::npos || lastPos != std::string::npos ){
                    coll.push_back( s.substr(lastPos,startPos-lastPos) );
                    lastPos = s.find_first_not_of(delim,startPos);
                    startPos = s.find_first_of(delim,lastPos);
                }
                
                // short circuit...
                if( coll.empty() )
                    return coll;
                
            
                
                // If the final token has a period, remove it
                std::string elem = coll.back();
                unsigned int n = 1;
                std::string lchar = elem.substr(elem.length()-n,1);
                if( lchar == "." ){
                    while(elem.length() >= n && elem.substr(elem.length()-n,1) == "."){
                        n++;
                    }
                
                    coll[coll.size()-1] = elem.substr(0,elem.size()-(n-1));
                    std::string final_period;
                    for( unsigned int j=1;j<n;j++){
                        final_period += ".";
                    }
                    coll.push_back(final_period);
                }
                
            
                
                // Do the real tokenization now by separating off the punctuation
                std::vector<std::string> tokens;
                std::vector<std::string>::iterator pos;
                for( pos = coll.begin(); pos != coll.end(); ++pos ){
                    std::string tok = *pos;
                
                    // only clean punctuation if there's something to clean
                    if( tok.find_first_not_of(letters+numbers,0) != std::string::npos ){
                
                        tok = clean_punctuation( tok );
                
                        std::string::size_type end = tok.find_first_of( " ", 0 );
                        if( end != std::string::npos ){
                
                            // There are multiple tokens here
                            tokens.push_back( tok.substr(0,end) );
                
                            while( end != std::string::npos ){
                                std::string::size_type beg = tok.find_first_not_of( " ", end );
                                end = tok.find_first_of( " ", beg );
                                tokens.push_back( tok.substr(beg,end-beg) );
                            }
                
                        } else {
                            // There's just one token here
                            tokens.push_back( tok );
                        }
                    } else {
                        tokens.push_back( tok );
                    }
                }
                
            
                
                /*    Now deal with all the periods, most of which should be
                 *    separated from the word (i.e. when it indicates the end
                 *    of a sentence), but some should be left on the word
                 *    (i.e. for abbreviations) */
                std::vector<std::string> final;
                std::vector<std::string>::size_type size = tokens.size();
                for( unsigned i=0; i<size; ++i ){
                    std::string token = tokens[i];
                    std::string::size_type len = token.length();
                    if (    len > 1 &&
                            token.substr(len-1,1) == "." &&
                            i + 1 < size &&
                            ( is_upper( tokens[i+1] ) || ! is_alpha( tokens[i+1] ) ) &&
                            !abbrs.is_abbreviation( token ) ) {
                
                        // Separate word and period
                        // Add both to tokens
                        final.push_back( token.substr(0,len-1) );
                        final.push_back( "." );
                    } else {
                        final.push_back( token );
                    }
                }
                
            
                return final;
            }
            

/* **********************************************************************
 *        Add tags to the given std::string,
 *        returns a tagged std::string
 * ********************************************************************** */
            
            std::string add_tags( const std::string& text )
            {
                std::vector<std::string> toks =  tokenize( text );
                std::string tagged;
            
                for( unsigned i = 0; i < toks.size(); ++i ){
                    std::string word = clean_word( toks[i] );
                    std::string tag = assign_tag( word );
            
            
                    tagged.append( toks[i] + "/" + tag + " " );
                }
                return tagged;
            }
            

/* **********************************************************************
 *        Reset the internal HMM record of the previous POS tag
 * ********************************************************************** */
            void reset()
            {
                previous_tag = "PP";
            }

/* **********************************************************************
 *        Get a std::map of the given POS type from a std::string of text
 * ********************************************************************** */
            std::map<std::string,int> get_POS( const std::string& text,
                                               const std::set<std::string>& exps){

                std::vector<std::string> toks =  tokenize( text );
                std::map<std::string,int> terms;

                for( unsigned i = 0; i < toks.size(); ++i ){
                    std::string word = clean_word( toks[i] );
                    std::string tag = assign_tag( word );
                    if( exps.count(tag)){
                        terms[toks[i]]++;
                    }
                }
                return terms;
            }

/* **********************************************************************
 *        Get the proper noun phrases from the given text string
 * ********************************************************************** */
            std::map<std::string,int> get_proper_nouns( const std::string& text ){
                std::map<std::string,int> phrases = get_noun_phrases(text);
                std::map<std::string,int>::iterator pos;
                std::set<std::string> to_erase;
                for( pos = phrases.begin(); pos != phrases.end(); ++pos){
                    std::pair<
                        std::vector<std::string>,
                        std::vector<std::string>
                        > phrase = vectorize_tagged_text(pos->first);
                    std::vector<std::string> words = phrase.first;
                    std::vector<std::string> tags = phrase.second;

                    // erase any noun phrases that have lower case nouns and adjectives
                    for( std::vector<std::string>::size_type i = words.size();
                             i > 0; --i ){
                        std::string t = simplify_tag(tags[i-1]);
                        if( ( t == "N" || t == "M" ) && is_lower(words[i-1]) ){
                            to_erase.insert(pos->first);
                            i = 0; // short circuit
                        }
                    }
                }
                std::set<std::string>::iterator epos;
                for( epos = to_erase.begin(); epos != to_erase.end(); ++epos ){
                    phrases.erase(*epos);
                }

                return phrases;
            }

/* **********************************************************************
 *        Get the maximal noun phrases from the given text string
 * ********************************************************************** */
            std::map<std::string,int> get_maximal_noun_phrases( const std::string& text ){
                std::vector<std::string> toks = tokenize( text );
                std::map<std::string,int> terms;
                std::string simplified;
                std::vector<std::string> words;
                std::vector<std::string> tags;
                for( unsigned i = 0; i < toks.size(); ++i ){
                    std::string tag = assign_tag( clean_word( toks[i] ) );
                    words.push_back( toks[i] );
                    tags.push_back( tag );
                    simplified += simplify_tag( tag );
                }

                std::vector<std::pair<std::string::size_type,
                                      std::string::size_type> > mnps;
                try {
                    mnps = find_mnp(simplified);
                } catch ( std::exception& e){
                    std::cerr << "Error finding Maximal Noun Phrase patterns: "
                              << e.what() << std::endl;
                }
                std::vector<std::pair<std::string::size_type,
                                      std::string::size_type> >::iterator pos;
                for( pos = mnps.begin(); pos != mnps.end(); ++pos ){
                    std::string term;
                    std::string::size_type start = pos->first;
                    std::string::size_type mnp_length = pos->second;
                    for( std::string::size_type i = start;
                             i < start + mnp_length; ++i){
                        term += words[i] + "/" + tags[i];
                        if( i < start+mnp_length-1 ){
                            term += " ";
                        }
                    }
                    terms[term]++;
                }
                return terms;
            }


/* **********************************************************************
 *        Get the noun phrases from the given text string
 * ********************************************************************** */
            std::map<std::string,int> get_noun_phrases( const std::string& text ){
                // get the maximal noun phrases

                std::map<std::string,int> terms;
                try {
                    terms = get_maximal_noun_phrases(text);
                } catch ( std::exception& e ){
                    std::cerr << "Error getting Maximal Noun Phrases: "
                              << e.what() << std::endl;
                }
                std::map<std::string,int> subphrases;
                std::map<std::string,int>::iterator pos;

                // loop through the MNPs and extract subphrases
                for( pos = terms.begin(); pos != terms.end(); ++pos ){

                    // first put the phrase string back into vectors
                    // and make a simplified version of the tag list
                    std::pair<std::vector<std::string>,
                              std::vector<std::string>
                        > words_and_tags = vectorize_tagged_text(pos->first);
                    std::vector<std::string> words = words_and_tags.first;
                    std::vector<std::string> tags = words_and_tags.second;
                    std::vector<std::string>::iterator vpos;
                    std::string tag_phrase;
                    for( vpos = tags.begin(); vpos != tags.end(); ++vpos){
                        tag_phrase += simplify_tag(*vpos);
                    }


                    // split up the noun phrase into subphrases by splitting
                    // on prepositions (P), articles (A), and numbers (D)
                    std::string::size_type ppos = tag_phrase.find_first_not_of("D",0);
                    std::string::size_type lastPpos = 0;
                    ppos = tag_phrase.find_first_of("PAD",ppos);

                        while( ppos != std::string::npos){
                        std::string subphrase = tag_phrase.substr(lastPpos,ppos-lastPpos);

                        // record resulting subphrase
                        std::string phrase;
                        for( unsigned i = 0; i < subphrase.size(); ++i ){
                            phrase += words[lastPpos+i] + "/" + tags[lastPpos+i];
                            if( i < subphrase.size() - 1){
                                phrase += " ";
                            }
                        }
                        subphrases[phrase]++;

                        // if the subphrase is longer than a single word, keep breaking
                        // it down by iteratively removing the leading word
                        if( subphrase.size() > 1){
                            for( unsigned spos = 0; spos < subphrase.size() -1; spos++){
                                // record first word, if it's a noun
                                if( subphrase.substr(spos,1) == "N" ){
                                    std::string term = words[lastPpos+spos] +
                                                       "/" + tags[lastPpos+spos];
                                    subphrases[term]++;
                                }
                                // record remainder of subphrase -- always a valid NP
                                std::string term;
                                for( unsigned i = 0; i < subphrase.size() - spos - 1; ++i ){
                                    term += words[lastPpos+spos+i+1] +
                                            "/" + tags[lastPpos+spos+i+1];
                                    if( i < subphrase.size() - spos - 1){
                                        term += " ";
                                    }
                                }
                                subphrases[term]++;
                            }
                        }
                        ppos = tag_phrase.find_first_not_of("PAD",ppos);
                        lastPpos = ppos;
                        ppos = tag_phrase.find_first_of("PAD",ppos);
                    }

                    // record the last subphrase -- this may be the same as the
                    // tag_phrase if there were no prepositions or articles

                    std::string subphrase = tag_phrase.substr(lastPpos,
                                                              tag_phrase.size()-lastPpos);
                    if( subphrase.size() < tag_phrase.size() ){
                        // record if different than original phrase
                        std::string phrase;
                        for( unsigned i = 0; i < subphrase.size(); ++i ){
                            phrase += words[lastPpos+i] + "/" + tags[lastPpos+i];
                            if( i < subphrase.size() - 1){
                                phrase += " ";
                            }
                        }
                        subphrases[phrase]++;

                    }

                    if( subphrase.size() > 1){
                        for( unsigned spos = 0; spos < subphrase.size() -1; spos++){
                            // record first word, if it's a noun
                            if( subphrase.substr(spos,1) == "N" ){
                                std::string term = words[lastPpos+spos] +
                                                   "/" + tags[lastPpos+spos];
                                subphrases[term]++;
                            }
                            // record remainder of subphrase
                            std::string term;
                            for( unsigned i = 0; i < subphrase.size() - spos - 1; ++i ){
                                term += words[lastPpos+spos+i+1] +
                                        "/" + tags[lastPpos+spos+i+1];
                                if( i < subphrase.size() - spos - 2){
                                    term += " ";
                                }
                            }
                            subphrases[term]++;
                        }
                    }
                }

                // add all the found subphrases onto the termlist
                for( pos = subphrases.begin(); pos != subphrases.end(); ++pos){
                    terms[pos->first] += pos->second;
                }
                return terms;
            }



/* **********************************************************************
 *        Remove POS tags from the std::map of terms
 * ********************************************************************** */
            std::map<std::string,int> remove_tags( const std::map<std::string,int>& wordmap ){
                std::map<std::string,int> terms;
                std::map<std::string,int>::const_iterator pos;

                for( pos = wordmap.begin(); pos != wordmap.end(); ++pos ){
                    std::string term(pos->first);
                    std::string::size_type tpos = term.find_last_of("/");
                    std::string::size_type lastPos = term.size();
                    while( tpos != std::string::npos ){
                        term.replace(tpos,lastPos-tpos,"");
                        tpos = term.find_last_of(" ",tpos-1);
                        lastPos = tpos;
                        tpos = term.find_last_of("/",tpos);
                    }
                    terms[term] += pos->second;
                }
                return terms;
            }


/* **********************************************************************
 *        Get the nouns from the given text string
 * ********************************************************************** */
            std::map<std::string,int> get_nouns( const std::string& text ){
                std::set<std::string> nouns;
                nouns.insert("NN");
                nouns.insert("NNP");
                nouns.insert("NNS");
                nouns.insert("NNPS");
                return get_POS( text, nouns );
            }


/* **********************************************************************
 *        Get the verbs from the given text string
 * ********************************************************************** */
            std::map<std::string,int> get_verbs( const std::string& text ){
                std::set<std::string> verbs;
                verbs.insert("VB");
                verbs.insert("VBD");
                verbs.insert("VBG");
                verbs.insert("VBN");
                verbs.insert("VBP");
                verbs.insert("VBZ");
                return get_POS( text, verbs );
            }


/* **********************************************************************
 *        Get the adjectives from the given text string
 * ********************************************************************** */
            std::map<std::string,int> get_adjectives( const std::string& text ){
                std::set<std::string> adjectives;
                adjectives.insert("JJ");
                adjectives.insert("JJS");
                adjectives.insert("JJR");
                return get_POS( text, adjectives );
            }








/*%#%#%#%#%#%#%#%#%#%#%#%#%#%#%#%#%#%#%#%#%#%#%#%#%#%#%#%#%#%#%#%#%#
%#
#%            PRIVATE FUNCTIONS
%#
#%#%#%#%#%#%#%#%#%#%#%#%#%#%#%#%#%#%#%#%#%#%#%#%#%#%#%#%#%#%#%#%#%*/
        private:
            std::string previous_tag;
            std::string letters;
            std::string upper_case;
            std::string numbers;



            typedef maps::ordered<std::string, unsigned long> LexiconValueMap;
            typedef maps::unordered<std::string, LexiconValueMap> LexiconMap;
            LexiconMap LEXICON;
            LexiconMap HMM;


            void init(){
                reset();
                numbers = "0123456789";
                upper_case = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
                letters = "abcdefghijklmnopqrstuvwxyz"+upper_case;
            }





/* **********************************************************************
 *        Find MNPs within the given tagged text
 * ********************************************************************** */
            std::vector<std::pair<std::string::size_type,
                                  std::string::size_type>
                            > find_mnp( const std::string tag_string ){
                std::vector<std::pair<std::string::size_type,
                                      std::string::size_type> > mnps;
                std::string::size_type pos = tag_string.find_first_of("N",0);
                while( pos != std::string::npos ){
                    std::string::size_type start = pos;
                    std::string::size_type end;
                    if( pos > 0 ){
                        std::string::size_type temp = tag_string.find_last_not_of("M",pos-1);
                        if( temp != std::string::npos && tag_string.substr(temp,1) == "D"){
                            start = temp;
                        } else {
                            start = temp + 1;
                        }
                    }

                    pos = tag_string.find_first_not_of("N",pos);
                    if( pos == std::string::npos ){
                        end = tag_string.size()-1;
                    } else {
                        end = pos - 1;
                    }
                    bool keep_going = true;
                    while( pos != std::string::npos &&
                           tag_string.substr(pos,1) != "X" &&
                           keep_going ){
                        pos = tag_string.find_first_not_of("P",pos);
                        std::string::size_type n = tag_string.find_first_not_of("A",pos) - pos;
                        if( n == 1 || n == 0 ){ // proceed
                            pos += n;
                            n = tag_string.find_first_not_of("D",pos) - pos;
                            if( n == 1 || n == 0 ){ // proceed
                                pos += n;
                                pos = tag_string.find_first_not_of("M",pos);
                                if( pos != std::string::npos &&
                                          tag_string.substr(pos,1) == "N"){ // found Noun head
                                    // go to end of N-head
                                    pos = tag_string.find_first_not_of("N",pos);
                                    if( pos == std::string::npos ){
                                        end = tag_string.size()-1;
                                    } else {
                                        end = pos - 1;
                                    }
                                } else {
                                    keep_going = false;
                                }
                            } else {
                                keep_going = false;
                            }
                        } else {
                            keep_going = false;
                        }
                    }
                    mnps.push_back( std::make_pair(start,end-start+1));
                    pos = tag_string.find_first_of("N",pos);
                }
                return mnps;
            }




/* **********************************************************************
 *        Used by `find_mnp`
 * ********************************************************************** */
            std::string simplify_tag( const std::string t ){
                std::string s;
                if( t.substr(0,2) == "NN"){
                    // noun
                    return "N";
                } else if( t.substr(0,2) == "JJ" || t == "VBN" || t == "VBG"){
                    // modifier
                    return "M";
                } else if( t == "IN" || t == "TO"){
                    // preposition
                    return "P";
                } else if( t == "DT"){
                    // article
                    return "A";
                } else if( t == "CD" ){
                    // number
                    return "D";
                } else {
                    // some other tag
                    return "X";
                }
            }


/* **********************************************************************
 *        Used by `tokenize`
 * ********************************************************************** */
            std::string clean_punctuation( const std::string& s )
            {

                std::string tok(s);

                std::string::size_type pos;

                try {
                    pos = tok.find_first_of("([{}])!?#$;~|%",0);
                    while( pos != std::string::npos ){
                        tok.insert(pos+1," ");
                        tok.insert(pos," ");
                        pos = tok.find_first_of("([{}])!?#$;~|%", pos+3);
                    }
                } catch ( std::exception &e ) {
                    std::cerr << "Parsing Error (1): " << e.what() << std::endl;
                }

                try {
                    pos = tok.find("--",0);
                    while( pos != std::string::npos ){
                        while( pos+1 != std::string::npos && tok.substr(pos+1,1) == "-"){
                            tok.replace(pos+1,1,"");
                        }
                        tok.insert(pos+1," ");
                        tok.insert(pos," ");
                        pos = tok.find("--",pos+3);
                    }

                } catch ( std::exception &e ) {
                    std::cerr << "Parsing Error (2): " << e.what() << std::endl;
                }

                try {

                    pos = tok.find("...",0);
                    while( pos != std::string::npos ){
                        int length = 3;
                        while( pos+length != std::string::npos && tok.substr(pos+length,1) == "."){
                            length++;
                        }
                        tok.replace(pos,length,"...");
                        tok.insert(pos+3," ");
                        tok.insert(pos," ");
                        pos = tok.find("...",pos+3);
                    }

                } catch ( std::exception &e ) {
                    std::cerr << "Parsing Error (3): " << e.what() << std::endl;
                }

                try {


                    pos = tok.find_first_of( "`", 0);
                    while( pos != std::string::npos ){
                        if( tok.substr(pos+1,1) == "`"){
                            tok.insert(pos+2," ");
                        } else {
                            tok.insert(pos+1," ");
                        }
                        tok.insert(pos," ");
                        pos = tok.find_first_of("`",pos+3);
                    }

                } catch ( std::exception &e ) {
                    std::cerr << "Parsing Error (4): " << e.what() << std::endl;
                }

                try {

                    pos = tok.find( "''", 0);
                    while( pos != std::string::npos ){
                        if( pos+2 != std::string::npos ){
                            tok.insert(pos+2," ");
                        }
                        tok.insert(pos," ");
                            pos = tok.find("''",pos+2);
                    }

                } catch ( std::exception &e ) {
                    std::cerr << "Parsing Error (5): " << e.what() << std::endl;
                }

                try {

                    pos = tok.find_first_of(",",0);
                    while( pos != std::string::npos ){
                        std::string::size_type m_pos = tok.find_first_of(numbers,pos);
                        if( pos+1 != std::string::npos && m_pos == pos+1){
                            pos = tok.find_first_of(",",pos+1);
                        } else {
                            tok.insert(pos+1," ");
                            tok.insert(pos," ");
                            pos = tok.find_first_of(",", pos+3);
                        }
                    }

                } catch ( std::exception &e ) {
                    std::cerr << "Parsing Error (6): " << e.what() << std::endl;
                }

                try {

                    pos = tok.find_first_of("\"",0);
                    while( pos != std::string::npos){
                        if( tok.find_first_of(letters,pos+1) != std::string::npos){
                            tok.replace(pos,1," `` ");
                        } else {
                            tok.replace(pos,1," '' ");
                        }
                        pos = tok.find_first_of("\"", pos+4 );
                    }
                } catch ( std::exception &e ) {
                    std::cerr << "Parsing Error (7): " << e.what() << std::endl;
                }



                    if( tok.substr(tok.size()-1,1) == ":"){
                        tok.insert(tok.size()-1," ");
                    }

                    if( tok.size() > 1 && tok.substr(tok.size()-1,1) == "."){
                        std::string::size_type m_pos = tok.find_last_not_of(letters,tok.size()-2);
                        if( m_pos < tok.size() - 2){
                            tok.insert(tok.size()-1," ");
                        }
                    }

                try {

                    pos = tok.find_first_of("'",0);
                    while( pos != std::string::npos){

                        std::string next_char;
                        std::string prev_char;
                        bool prev_is_alphaNum = false;
                        bool next_is_alphaNum = false;
                        if( pos < tok.size()-1){
                            next_char = tok.substr(pos+1,1);
                            if( next_char.find_first_of(letters+numbers,0) != std::string::npos)
                                next_is_alphaNum=true;

                        }

                        if( pos > 0){
                            prev_char = tok.substr(pos-1,1);
                            if( prev_char.find_first_of(letters+numbers,0) != std::string::npos)
                                prev_is_alphaNum=true;

                        }
                        if( prev_char == "."){
                            tok.insert(pos," ");
                            pos = tok.find_first_of("'",pos+2);
                        } else if( pos < tok.size()-1 && tok.substr(pos+1,1) == "'") {
                            // next char is a single quote (')
                            pos = tok.find_first_of("'",pos+2);
                        } else if( next_is_alphaNum && prev_is_alphaNum ){
                            // next and prev char is alpha-numeric
                            // is it a contraction?
                            if( prev_char == "n" && next_char == "t" ){
                                tok.insert(pos-1," ");
                                pos = tok.find_first_of("'",pos+3);
                            } else if ( next_char == "d" || next_char == "m" || next_char == "s" ){
                                if( pos == tok.size()-2 ||
                                    tok.find_first_not_of(letters+numbers,pos+2) == pos+2 ){
                                    tok.insert(pos," ");
                                    pos = tok.find_first_of("'",pos+3);
                                } else {
                                    pos = tok.find_first_of("'",pos+2);
                                }
                            } else if ( tok.find("'ve", pos)==pos ||
                                        tok.find("'ll",pos)==pos ||
                                        tok.find("'re",pos)==pos){
                                tok.insert(pos," ");
                                pos = tok.find_first_of("'",pos+4);
                            } else {
                                pos = tok.find_first_of("'",pos+1);
                            }

                        } else if ( next_is_alphaNum && !prev_is_alphaNum ){ // begins word
                            tok.replace(pos,1," ` ");
                            pos = tok.find_first_of("'",pos+3);
                        } else if ( !next_is_alphaNum && prev_is_alphaNum ){ // ends word
                            tok.replace(pos,1," ' ");
                            pos = tok.find_first_of("'",pos+3);
                        } else {
                            pos = tok.find_first_of("'",pos+1);
                        }

                    }
                } catch ( std::exception &e ) {
                    std::cerr << "Parsing Error (8): " << e.what() << std::endl;
                }
                boost::algorithm::trim(tok);
                return tok;

            }




/* **********************************************************************
 *        Shortcut for taking a tagged string and turning it
 *        back into two vectors of words and tags
 * ********************************************************************** */
            std::pair<std::vector<std::string>,
                      std::vector<std::string>
                      > vectorize_tagged_text(const std::string tagged){
                std::vector<std::string> words;
                std::vector<std::string> tags;
                std::string::size_type word_start = 0;
                std::string::size_type tag_end = tagged.find_first_of(" ",0);
                std::string::size_type divider;
                while( tag_end != std::string::npos ){
                    divider = tagged.find_last_of("/",tag_end);
                    words.push_back(tagged.substr(word_start,divider-word_start));
                    tags.push_back(tagged.substr(divider+1,tag_end-divider-1));
                    word_start = tagged.find_first_not_of(" ",tag_end);
                    tag_end = tagged.find_first_of(" ",word_start);
                }
                divider = tagged.find_last_of("/",tagged.size());
                words.push_back(tagged.substr(word_start,divider-word_start));
                tags.push_back(tagged.substr(divider+1,tagged.size()-divider-1));
                return std::make_pair(words,tags);
            }




/* *********************************************************************
 *        Load the lexicon into a 2-dimensional std::map
 *********************************************************************** */
            void load_lexicon( const std::string& filename="" )
            {
                if (!filename.empty()) {
                    if (try_loading_lexicon(filename)) return;
                    if (try_loading_lexicon(LEXICON_INSTALL_LOCATION)) return;

                    std::cerr << "Couldn't load lexicon from either "
                              << filename << " or "
                              << LEXICON_INSTALL_LOCATION << ".\n";
                    exit(EXIT_FAILURE);
                }

                if (try_loading_lexicon(LEXICON_INSTALL_LOCATION)) return;
                std::cerr << "Couldn't load lexicon from "
                          << LEXICON_INSTALL_LOCATION << ".\n";
                exit(EXIT_FAILURE);
            }

            bool try_loading_lexicon( const std::string &filename ) {
                std::ifstream file;
                file.open(filename.c_str(), std::ios_base::in | std::ios_base::binary);
                if( file ){
                    load_lexicon(file);
                    file.clear();
                    file.close();
                    return true;
                }
                return false;
            }

            void load_lexicon( std::istream& lex_stream ){
                std::string lex = "hmm";
                std::string line;

                while (std::getline(lex_stream,line)) {

                    if( line.compare(0,10,"## Lexicon") == 0 ){
                        lex = "lex";
                    } else if ( line.compare(0,22,"## Hidden Markov Model") == 0){
                        lex = "hmm";
                    }

                    if ( line.compare(0,1,"#") != 0 ) {
                        std::string::size_type pos = line.find_first_of(" ", 0);
                        std::string word = line.substr(0,pos-1);
                        std::string delim = " {},:";
                        std::string::size_type lastPos = line.find_first_not_of(delim,pos);
                        pos = line.find_first_of(delim,lastPos);


                        while( pos != std::string::npos || lastPos != std::string::npos ){

                            // POS TAG
                            std::string tag = line.substr(lastPos,pos-lastPos);
                            lastPos = line.find_first_not_of(delim,pos);
                            pos = line.find_first_of(delim,lastPos);

                            // Frequency
                            unsigned long value;
                            std::string num = line.substr(lastPos,pos-lastPos);
                            value = strtoul( num.c_str(), NULL, 10 );
                            lastPos = line.find_first_not_of(delim,pos);
                            pos = line.find_first_of(delim,lastPos);

                            // Record
                            if( lex == "lex" ){
                                LEXICON[word][tag] = value;
                            } else if ( lex == "hmm" ) {
                                HMM[word][tag] = value;
                            }
                        }

                    }
                }
            }




/* **********************************************************************
 *        Viterbi Algorithm --> Bayesian logic applied to values in lexica
 * ********************************************************************** */
            std::string assign_tag( const std::string& word )
            {
                if( word == "*SYM*")
                    return "SYM";

                
                std::string best_tag( "NN" );
                unsigned long best_so_far = 0;
                
                LexiconValueMap &wordtags = LEXICON[word];
                LexiconValueMap &hmmtags = HMM[previous_tag];
                
                for( LexiconValueMap::iterator pos = wordtags.begin();
                         pos != wordtags.end();
                         ++pos ) {
                    std::string tag = pos->first;
                    if( hmmtags.count( tag ) ){
                        unsigned long probability = hmmtags[tag] * ( pos->second + 1 );
                        if( probability > best_so_far ){
                            best_so_far = probability;
                            best_tag = tag;
                        }
                    }
                }
                previous_tag = best_tag;
                
                return best_tag;
            }

/*        A slightly slower version of the above     */
            std::string assign_tag_slower( const std::string& word )
            {
                if( word == "*SYM*")
                    return "SYM";

                std::string best_tag( "NN" );
                unsigned long best_so_far = 0;

                LexiconValueMap &wordtags = LEXICON[word];
                LexiconValueMap &hmmtags = HMM[previous_tag];

                for(LexiconValueMap::iterator pos = hmmtags.begin(); pos != hmmtags.end(); ++pos) {
                    std::string tag = pos->first;
                    if( wordtags.count( tag ) ){
                        unsigned long probability = pos->second * ( wordtags[tag] + 1 );
                        if( probability > best_so_far ){
                            best_so_far = probability;
                            best_tag = tag;
                        }
                    }
                }
                previous_tag = best_tag;
                return best_tag;
            }




/* *********************************************************************
 *        Tell me whether this is an upper/lower case word or not
 *********************************************************************** */
            bool is_upper( const std::string& s ){
                if( s.size() > 0 &&
                    boost::algorithm::all(s.substr(0,1), boost::algorithm::is_upper()) ){
                    return true;
                } else {
                    return false;
                }
            }

            bool is_lower( const std::string& s ){
                if( s.size() > 0 &&
                    boost::algorithm::all(s.substr(0,1), boost::algorithm::is_lower()) ){
                    return true;
                } else {
                    return false;
                }
            }

            bool is_alpha( const std::string& s ){
                if( s.size() > 0 &&
                    boost::algorithm::all(s.substr(0,1), boost::algorithm::is_alpha()) ){
                    return true;
                } else {
                    return false;
                }
            }



/* *********************************************************************
 *        Decide what form of the word to analyze
 *********************************************************************** */
            std::string clean_word( const std::string& word )
            {

                std::string lower(word);
                std::transform( word.begin(), word.end(), lower.begin(), tolower );

                // if it looks like this word starts a sentence, try lower case first
                if( previous_tag == "PP" && is_upper(word) ){
                    // Does the word exist as a lower case word?
                    if( LEXICON.count( lower ) && word != "I" ){
                        // NOTE: it would be better to have an actual comparison count
                        // here -- use lower if more common than upper...
                        return lower;
                    }
                }

                // Does the word exist in the lexicon as-is?
                if( LEXICON.count( word ) ){
                    return word;
                }

                // Does the word exist as a lower case word?
                if( LEXICON.count( lower ) ){
                    return lower;
                }


                // Otherwise, classify by word morphology
                return classify_unknown_word( word );

            }



/* *********************************************************************
 *        Classify unknown words according to word morphology
 *********************************************************************** */
            std::string classify_unknown_word( const std::string& word )
            {

                std::string upper(word);
                upper[0] = toupper(word[0]);

                std::string::size_type pos;

                bool has_letters = false;
                bool has_numbers = false;
                bool has_hyphen = false;

                if( word.find_first_of("-",0) != std::string::npos)
                    has_hyphen = true;

                if( word.find_first_of(letters,0) != std::string::npos)
                    has_letters = true;

                if( word.find_first_of(numbers,0) != std::string::npos)
                    has_numbers = true;

                
                if( has_numbers ){
                    
                    pos = word.find_first_not_of("-."+numbers);
                    if( pos == std::string::npos)
                        return "*NUM*";
                    
                    if( has_letters ){
                        std::string::size_type m_pos = word.find_first_not_of(letters,pos);
                        if( m_pos == std::string::npos)
                            return "*ORD*";
                    }
                    pos = word.find_first_not_of("/:",pos);
                    if( pos != std::string::npos ){
                        pos = word.find_first_not_of(numbers,pos);
                        if( pos == std::string::npos)
                            return "*NUM*";
                    }
                    
                }
                
                if( has_hyphen && has_letters ){
                    
                    std::string::size_type lastPos = word.size()-2;
                    pos = word.find_last_of("-",lastPos);
                    if( pos > 0 &&
                        word.find_last_of(letters+numbers,pos) == pos-1 &&
                        word.find_first_of(letters+numbers,pos) == pos+1){
                        if( LEXICON.count( word.substr(pos+1,word.size()-1-pos) ) ){
                            return "*HYP-ADJ*";
                        } else {
                            return "*HYP*";
                        }
                    }
                    
                }
                
                
                if( word.find_first_of("[({",0) != std::string::npos ){
                    return "*LRB*";
                } else if ( word.find_first_of("]})",0) != std::string::npos ){
                    return "*RRB*";
                } else if ( word.find_first_not_of(upper_case,0) != 0 &&
                            word.find_first_not_of(upper_case+".-&",1) == std::string::npos ){
                    return "*ABR*";
                } else if ( word.find_first_of(letters+numbers,0) == std::string::npos ){
                    return "*SYM*";
                } else if ( word == upper ){
                    return "*CAP*";
                } else if ( word.size() >= 3 && word.substr(word.size()-3,3) == "ing" ){
                    return "*ING*";
                } else if ( word.substr(word.size()-1,1) == "s" ){
                    return "*S*";
                } else if ( word.size() >= 4 && word.substr(word.size()-4,4) == "tion" ){
                    return "*TION*";
                } else if ( word.size() >= 2 && word.substr(word.size()-2,2) == "ly" ){
                    return "*LY*";
                } else if ( word.size() >= 2 && word.substr(word.size()-2,2) == "ed" ){
                    return "*ED*";
                } else {
                    return "*UNKNOWN*";
                }
                
                
                
            }
    }; // class Tagger
} // namespace semantic

#endif


