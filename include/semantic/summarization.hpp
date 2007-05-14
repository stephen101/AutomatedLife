#ifndef __SEMANTIC_SUMMARIZATION_HPP__
#define __SEMANTIC_SUMMARIZATION_HPP__

#include <semantic/semantic.hpp>
#include <semantic/abbreviations.hpp>
#include <semantic/stem/english_stem.h>

#include <map>
#include <string>
#include <iostream>
#include <cctype>


namespace semantic {

	class summarizer {
		
		typedef std::pair<int,double> IDpair;
				
		public: 
			summarizer(  std::map<std::string,double>& words ) : top_words(words) {}
			
/* ****************************************************************
 *		Rank the sentences of the given text according to 
 *		the top_words provided in the constructor
 * **************************************************************** */
			std::vector<std::string> summarize( const std::string& text, const unsigned max_sentences = 3 ){
				
				std::vector<std::string> sentences = tokenize_sentences( text );
				std::vector<std::string>::iterator pos;
				std::map<int,termMap> textData;
				
				// loop through the sentences
				unsigned i = 0;
				for( pos = sentences.begin(); pos != sentences.end(); ++pos ){
					
					std::vector<std::string> terms = tokenize_words(*pos);
					std::vector<std::string>::iterator wpos;
					
					// loop through the terms
					for( wpos = terms.begin(); wpos != terms.end(); ++wpos ){
						std::string term = *wpos;
						if( top_words.count(term) ){
							textData[i][term] += top_words[term];
						}
					}
					i++;
				}
				
				std::set<std::string> used_words;
				std::set<int> seen_sentences;
				std::vector<std::string> summary;
				
				// loop through the max number of sentences to use for a summary
				for( unsigned j = 0; j < max_sentences && j < sentences.size(); ++j ){
					
					std::set< IDpair, std::less<IDpair> > sentence_weights;
					
					// loop through the data for each sentence
					std::map<int,termMap>::iterator tpos;
					for( tpos = textData.begin(); tpos != textData.end(); ++tpos ){
						int sentenceIdx = tpos->first;
						
						// proceed if we haven't already used this sentence
						if( !seen_sentences.count(sentenceIdx) ){
							termMap terms = tpos->second;
							termMap::iterator tmpos;
							double weight = 0.00;
							
							// loop through all the countable terms in the text,
							// omitting any terms that appeared in docs already registered.
							for( tmpos = terms.begin(); tmpos != terms.end(); ++tmpos ){
								if( j == 0 || used_words.count(tmpos->first) == 0 ){
									weight += tmpos->second;
								}
							}

							// don't count sentences whose words have all already been used
							if( weight > 0 ){
								sentence_weights.insert( std::make_pair( sentenceIdx, weight ) );
							}
						}
					}
					

					// grab the top sentence					
					std::set<IDpair>::iterator spos = sentence_weights.begin();
					if( spos != sentence_weights.end() ){
					
						int top_sent = spos->first;
//						double w = spos->second;
						seen_sentences.insert(top_sent);
						termMap top_sent_terms = textData[top_sent];
						termMap::iterator epos;
						for( epos = top_sent_terms.begin(); epos != top_sent_terms.end(); ++epos ){
							used_words.insert( epos->first );
						}
						textData.erase(top_sent);
						if( sentences[top_sent].size() > 10){
							summary.push_back( sentences[ top_sent ] );
						} else {
							j--;
						}
					}
				}
				
				return summary;
			}


		private:
			std::map<std::string,double>& top_words;
			typedef std::map<std::string,double> termMap;
			
		
/* *************************************************
 *		Trim whitespace from the ends of a string
 * ************************************************* */
			void trim( std::string& str ){
				
				std::string::size_type pos = str.find_last_not_of(" ");
				if( pos != std::string::npos ){
					str.erase(pos + 1 );
					pos = str.find_first_not_of( " " );
					if( pos != std::string::npos ) str.erase(0, pos );
				} else {
					str.erase(str.begin(), str.end());
				}
			}

			
/* **************************************************************
 *		This tells whether the word or character is upper case
 * ************************************************************** */
			bool is_upper ( const std::string& letter ){
			
				std::string upper(letter);
				std::transform(letter.begin(),letter.end(),upper.begin(),toupper);
				if ( letter == upper ){
					return true;
				} else {
					return false;
				}
			}


/* ****************************************************************
 *		Tokenize a sentence into stemmed, punctuation-free words
 * **************************************************************** */
			std::vector<std::string> tokenize_words( const std::string& sentence ){
				
				stemming::english_stem Stemmer;
				std::vector<std::string> terms;
				std::string letters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
				std::string numbers = "0123456789";
				// tokenize on whitespace 
				std::string delim = " ";
				std::string::size_type lastPos = sentence.find_first_not_of(delim,0);
				std::string::size_type startPos = sentence.find_first_of(delim,lastPos);
				
				while( startPos != std::string::npos && lastPos != std::string::npos ){
					std::string::size_type pos;
					
					std::string tok = sentence.substr(lastPos,startPos-lastPos);
					
					// English-specific contractions 
					
					std::vector<std::string> contractions;
					std::vector<std::string>::iterator cpos;
					contractions.push_back("n't");
					contractions.push_back("'ll");
					contractions.push_back("'ve");
					contractions.push_back("'re");
					contractions.push_back("'d");
					contractions.push_back("'m");
					contractions.push_back("'s");
					for( cpos = contractions.begin(); cpos != contractions.end(); ++cpos ){
						std::string cont = *cpos;
						pos = tok.find(cont,0);
						while( pos != std::string::npos ){
							std::string post;
							if( pos + cont.size() == tok.size() ){
								tok.erase( pos, cont.size());
							} else if( pos + cont.size() < tok.size() ){
								std::string post = tok.substr(pos+cont.size(),1);
								if( post.find_first_of(letters,0) != std::string::npos ){
									tok.erase( pos, cont.size());
								} else {
									pos += cont.size();
								}
							}
							pos = tok.find(cont,pos);
						}
					}
					
					pos = tok.find_first_of(letters);
					if( pos != 0 && pos != std::string::npos ){
						tok = tok.substr(pos,tok.size()-pos);
					}
					pos = tok.find_last_of(letters);
					if( pos != tok.size()-1 && pos != std::string::npos ){
						tok = tok.substr(0,pos+1);
					}

					if( tok.size() > 2 && tok.find_first_of(letters,0) != std::string::npos ){
						Stemmer(tok);
						if( tok.length() > 0 ){
							// make lower case
							std::string lower(tok);
							std::transform(tok.begin(),tok.end(),lower.begin(),tolower);
							terms.push_back( lower );
						}
					}

					lastPos = sentence.find_first_not_of(delim,startPos);
					startPos = sentence.find_first_of(delim,lastPos);
				}
				return terms;
			}


/* *******************************************
 *		Tokenize a text into sentences
 * ******************************************* */
			std::vector<std::string> tokenize_sentences( const std::string& intext ){
				std::string text(intext);
				std::vector<std::string> sentences;
				
				// standardize whitespace and trim the text of extra whitespace
				std::string delim = "\x09\x0a\x0c\x0d\xa0\x85";
				std::string::size_type wspos = text.find_first_of(delim,0);
				while( wspos != std::string::npos ){
					text.replace(wspos,1," ");
					wspos = text.find_first_of(delim,wspos+1);
				}
				trim( text );
				
				Abbreviations abbrs;
				std::string::size_type pos = text.find_first_of(".?!", 0);
				std::string::size_type lastPos = 0;
				while( pos != std::string::npos ){
				
					pos = text.find_first_not_of(".?!'\")\u2019\u201d", pos+1);
					
					// easy case, were're at the end of the text
					if( pos == std::string::npos ){
						std::string sent = text.substr(lastPos, text.size()-lastPos);
						trim( sent );
						sentences.push_back(sent);
					
					// common case, we're at the end of a word
					} else if( text.substr(pos,1) == " " ) {
						
						// is next word capitalized?
						std::string::size_type next = text.find_first_not_of( " ", pos );
						if( is_upper( text.substr(next,1))){
						
							// now check if the last word was an abbr....
							std::string::size_type lastWordIdx = text.find_last_of(" ",pos-1);
							std::string lastWord;
							if( lastWordIdx == std::string::npos ){
								// beginning of text
								lastWord = text.substr(0,pos);	
							} else {
								lastWord = text.substr(lastWordIdx+1,pos-lastWordIdx-1);
							}
							
							// it is not an abbreviation or short word.
							if( !abbrs.is_abbreviation( lastWord ) ){
								std::string sent = text.substr(lastPos, pos-lastPos);
								trim( sent );
								sentences.push_back(sent);
								lastPos = pos + 1;
							}
						} 
						pos = text.find_first_of(".?!", pos);
					
					// not at the end of a word
					} else {
						pos = text.find_first_of(".?!", pos);
					}
				}
				return sentences;
			}
	};
}

inline bool operator > (const std::pair<std::string,double>& pair1, const std::pair<std::string,double>& pair2 ){
	return pair1.second>pair2.second?true:false;
}

inline bool operator < (const std::pair<std::string,double>& pair1, const std::pair<std::string,double>& pair2 ){
	return pair1.second<pair2.second?true:false;
}

#endif
