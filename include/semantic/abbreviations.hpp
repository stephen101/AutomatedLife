
#ifndef __SEMANTIC_ABBREVIATIONS_HPP__
#define __SEMANTIC_ABBREVIATIONS_HPP__

#include <string>
#include <set>
#include <cctype>
#include <boost/algorithm/string.hpp>

namespace semantic {

	class Abbreviations {
		public:
		
			Abbreviations(){ load_abbreviations(); };

			
		
/* **********************************************************************
 *		Decide whether the given std::string is an abbreviation
 * ********************************************************************** */
			bool is_abbreviation( const std::string& s )
			{
				std::string lower(s);
				
				// convert to lower case
				std::transform(s.begin(), s.end(), lower.begin(), tolower );
				
				// remove any trailing period
				std::string::size_type pos = lower.find_last_not_of(".");
				if( pos < lower.size() -1 ){
					lower.erase(pos+1,lower.size()-pos-1);
				}
				// Does the abbreviation have a letter-dot-letter combination (e.g. j.c. penny's)
				bool repeat = false;
				pos = lower.find_first_of(".",0);
				
				if( pos != std::string::npos && pos > 0 && pos < lower.size()-1){
					if( boost::algorithm::all(lower.substr(pos-1,1),boost::algorithm::is_lower() ) &&
						boost::algorithm::all(lower.substr(pos+1,1),boost::algorithm::is_lower() ) ){
						repeat = true;
					}
				}
				if( abbreviations.find(lower) == abbreviations.end() && 
						lower.length() > 1 && 
						! repeat ){
					// doesn't occur in abbreviations list and is longer than 1 letter
					// and doesn't have a repeating letter-dot-letter combination (e.g. U.S.A.)
					// therefore, not an abbreviation
					return false;
				} else {
					return true;
				}
			}


			
			void add_abbreviatons( const std::set<std::string>& abbrs ){
				std::set<std::string>::const_iterator pos;
				for( pos = abbrs.begin(); pos != abbrs.end(); ++pos ){
					add_abbreviation( *pos );
				}
			}


			
			void add_abbreviation( const std::string& a ){
				std::string lower(a);
				
				// convert to lower case
				std::transform(a.begin(), a.end(), lower.begin(), tolower );
				
				// remove any trailing period
				std::string::size_type pos = lower.find_last_not_of(".");
				lower.erase(pos,lower.size()-pos-1);

				abbreviations.insert( lower );
			}
		
		
		private:
			typedef std::set<std::string> AbbreviationSet;
			AbbreviationSet abbreviations;
			

/* *********************************************************************
 *		Load a bunch of common abbreviations into a std::set
 *********************************************************************** */
			void load_abbreviations()
			{
				AbbreviationSet a;
				
				std::string abbrs[] = { 
					"mr",  "prof",  "mrs",  "dr",  "sr",  "sen",  "sens",  "rep",  "reps",  "gov",  "atty",  "attys",  "mssrs",  "rev",  "supt",  
					"col",  "gen",  "lt",  "cmdr",  "adm",  "capt",  "sgt",  "cpl",  "maj",  "brig",  
					"dept",  "univ",  "assn",  "bros",  "ph.d",  
					"al",  "ave",  "blvd",  "bld",  "ct",  "exp",  "expy",  "dist",  "mt",  "mtn",  "ft",  "fy",  "fwy",  "hwy",  "pde",  "pd",  "plz",  "pl",  "rd",  "st",  
					"mfg",  "inc",  "ltd",  "co",  "corp",  
					"ala",  "ariz",  "ark",  "cal",  "calif",  "colo",  "col",  "ida",  "conn",  "id",  "del",  "ukk",  "fed",  "ind",  "fla",  "ia",  "ga",  "kans",  "kan",  "minn",  "ken",  "miss",  "ky",  "mo",  "la",  "mont",  "me",  "neb",  "md",  "nebr",  "is",  "nev",  "mass",  "mex",  "mich",  "okla",  "ok",  "penn",  "ore",  "pa",  "dak",  "tenn",  "wis",  "tex",  "wisc",  "ut",  "wy",  "vt",  "wyo",  "va",  "usafa",  "wash",  "alta",  "ont",  "sask",  "que",  "yuk",  					
					"jan",  "jun",  "feb",  "jul",  "mar",  "aug",  "apr",  "sep",  "sept",  "nov",  "oct",  "dec",  
					"etc",  "esp" 
				};
				
				a.insert(abbrs, abbrs+126);
				abbreviations = a;
			}
	};
}

#endif
