
#ifndef __SEMANTIC_FILTER_HPP__
#define __SEMANTIC_FILTER_HPP__

/*
    Classes for creating text and word filters
*/


#include <string>
#include <set>
#include <cctype>
#include <iostream>


namespace semantic {

/* *******************************************************
        WORD FILTERS
   ******************************************************* */
    class word_filter {
        public:
            virtual ~word_filter() {}
            virtual bool operator()(const std::string& word) = 0;
    };

    
    class blacklist_filter : public word_filter {
        public:
            blacklist_filter( const std::set<std::string>& m_words )
                : blacklist_words(m_words) {}
    
            bool operator()(const std::string& word) {
                std::string lower(word);
                std::transform(word.begin(),word.end(),lower.begin(),tolower);
                return blacklist_words.count(lower)>0?false:true;
            }
    
        private:
            std::set<std::string> blacklist_words;
    };
    

    
    class whitelist_filter : public word_filter {
        public:
            whitelist_filter( const std::set<std::string>& words ) : words(words) {}
    
            bool operator()(const std::string& word) {
                std::string lower(word);
                std::transform(word.begin(),word.end(),lower.begin(),tolower);
                return words.count(lower)>0?true:false;
            }
    
        private:
            std::set<std::string> words;
    };
    
	class allowable_characters_filter : public word_filter {
		public:
			allowable_characters_filter( const std::string& str ) : allowed(str) {}
			
			bool operator()(const std::string& word) {
				return word.find_first_not_of(allowed)==std::string::npos?true:false;
			}
			
		private:
			std::string allowed;
	};
	
	class illegal_characters_filter : public word_filter {
		public:
			illegal_characters_filter( const std::string& str) : illegal(str) {}
			
			bool operator()(const std::string& word){
				return word.find_first_of(illegal)==std::string::npos?true:false;
			}
		private:
			std::string illegal;
	};
    
    class maximum_word_length_filter : public word_filter {
        public:
            maximum_word_length_filter( const unsigned int len ) : length(len) {}
    
            bool operator()(const std::string& word) {
    
                // look for spaces
                std::string::size_type pos = word.find_first_of( " ", 0 );
                std::string::size_type lastPos = 0;
                    while( pos != std::string::npos ){
                        if( pos - lastPos > length ){
                            return false;
                        }
                        lastPos = pos;
                        pos = word.find_first_of( " ", pos+1 );
                    }
                    if( word.size() - lastPos > length ){
                        return false;
                    }
                    return true;
            }
    
        private:
            unsigned int length;
    };
    

    
    class minimum_length_filter : public word_filter {
        public:
            minimum_length_filter( const unsigned int len ) : length(len) {}
    
            bool operator()(const std::string& word ) {
                return word.length()<length?false:true;
            }
    
        private:
            unsigned int length;
    };
    

    
    class maximum_phrase_length_filter : public word_filter {
        public:
            maximum_phrase_length_filter( const unsigned int len ) : length( len ){}
    
            bool operator()( const std::string& word ){
                std::string::size_type pos = word.find_first_of( " ", 0 );
                unsigned words = 1;
    
                // count the number of words in the phrase
                while( pos != std::string::npos ){
                    words++;
                    pos = word.find_first_not_of( " ", pos );
                    pos = word.find_first_of( " ", pos );
                }
                return words>length?false:true;
            }
    
        private:
            unsigned int length;
    };
    

    
    class minimum_phrase_length_filter : public word_filter {
        public:
            minimum_phrase_length_filter( const unsigned int len ) : length( len ){}
    
            bool operator()( const std::string& word ){
                std::string::size_type pos = word.find_first_of( " ", 0 );
                unsigned words = 1;
    
                // count the number of words in the phrase
                while( pos != std::string::npos ){
                    words++;
                    pos = word.find_first_not_of( " ", pos );
                    pos = word.find_first_of( " ", pos );
                }
                return words<length?false:true;
            }
    
        private:
            unsigned int length;
    };
    

    
    class too_many_numbers_filter : public word_filter {
        public:
            too_many_numbers_filter( const unsigned int num ) : number(num) {}
    
            bool operator()(const std::string& word) {
                std::string digits = "0123456789";
                std::string::size_type pos = word.find_first_of( digits, 0 );
                unsigned int count = 0;
                while( pos != std::string::npos ){
                    count++;
                    pos = word.find_first_of( digits, pos+1 );
                }
                return count>=number?false:true;
            }
    
        private:
            unsigned int number;
    };
    


/* *******************************************************
        TEXT FILTERS
   ******************************************************* */

    class text_filter  {
        public:
            std::string encoding;
//          text_filter(){}
            virtual ~text_filter() {}
            virtual std::string operator()(const std::string& filename) = 0;

            std::string get_encoding()
            {
                return encoding;
            }


    };

    class smart_quotes_filter : public text_filter {

        public:
            std::string operator()( const std::string& text ){
                std::string cleaned(text);
                std::string delim;
                std::string replace;
                std::string::size_type nextPos;
                std::string::size_type pos;

#ifndef WIN32
				delim = "\u2015";
                replace = "";
                
                pos = cleaned.find_first_of(delim,0);
                while( pos != std::string::npos ){
                    nextPos = cleaned.find_first_not_of(delim,pos);
                    if( nextPos != std::string::npos ){
                        cleaned.replace(pos,nextPos-pos,replace);
                    } else {
                        cleaned.replace(pos,1,replace);
                    }
                    pos = cleaned.find_first_of(delim,pos);
                }
                

                delim = "\u2010\u2011";
                replace = "-";
                
                pos = cleaned.find_first_of(delim,0);
                while( pos != std::string::npos ){
                    nextPos = cleaned.find_first_not_of(delim,pos);
                    if( nextPos != std::string::npos ){
                        cleaned.replace(pos,nextPos-pos,replace);
                    } else {
                        cleaned.replace(pos,1,replace);
                    }
                    pos = cleaned.find_first_of(delim,pos);
                }
#endif                

#ifdef WIN32
                delim = "\u2013\u2014";
#else
				delim = "\u2012\u2013\u2014";
#endif
                replace = "--";
                
                pos = cleaned.find_first_of(delim,0);
                while( pos != std::string::npos ){
                    nextPos = cleaned.find_first_not_of(delim,pos);
                    if( nextPos != std::string::npos ){
                        cleaned.replace(pos,nextPos-pos,replace);
                    } else {
                        cleaned.replace(pos,1,replace);
                    }
                    pos = cleaned.find_first_of(delim,pos);
                }
                

                delim = "\u2018\u2019";
                replace = "'";
                
                pos = cleaned.find_first_of(delim,0);
                while( pos != std::string::npos ){
                    nextPos = cleaned.find_first_not_of(delim,pos);
                    if( nextPos != std::string::npos ){
                        cleaned.replace(pos,nextPos-pos,replace);
                    } else {
                        cleaned.replace(pos,1,replace);
                    }
                    pos = cleaned.find_first_of(delim,pos);
                }
                

                delim = "\u201d\u201c";
                replace = "\"";
                
                pos = cleaned.find_first_of(delim,0);
                while( pos != std::string::npos ){
                    nextPos = cleaned.find_first_not_of(delim,pos);
                    if( nextPos != std::string::npos ){
                        cleaned.replace(pos,nextPos-pos,replace);
                    } else {
                        cleaned.replace(pos,1,replace);
                    }
                    pos = cleaned.find_first_of(delim,pos);
                }
                

                delim = "\u2022";
                replace = "*";
                
                pos = cleaned.find_first_of(delim,0);
                while( pos != std::string::npos ){
                    nextPos = cleaned.find_first_not_of(delim,pos);
                    if( nextPos != std::string::npos ){
                        cleaned.replace(pos,nextPos-pos,replace);
                    } else {
                        cleaned.replace(pos,1,replace);
                    }
                    pos = cleaned.find_first_of(delim,pos);
                }
                

                return cleaned;
            }
    };

    class html_filter : public text_filter {
        public:

            std::string operator()( const std::string& text ){

                std::string cleaned(text);
                std::map<std::string,std::string> entities = load_entity_map();

                if( encoding.size() < 1 )
                    encoding = identify_encoding( text );

                
                std::string::size_type htmlPos = cleaned.find_first_of("<",0);
                std::string::size_type lastHtmlPos = cleaned.find_first_of(">",htmlPos);
                while( htmlPos != std::string::npos && lastHtmlPos != std::string::npos )
                {
                    cleaned.replace(htmlPos,lastHtmlPos-htmlPos+1," ");
                    htmlPos = cleaned.find_first_of("<",htmlPos+1);
                    lastHtmlPos = cleaned.find_first_of(">",htmlPos);
                }
                


                
                htmlPos = cleaned.find_first_of("&",0);
                lastHtmlPos = cleaned.find_first_of(";",htmlPos);
                while( htmlPos != std::string::npos && lastHtmlPos != std::string::npos ){
                    std::string entity = cleaned.substr(htmlPos+1,lastHtmlPos-htmlPos-1);
                    if( entity.find_first_of(" .&",0) == std::string::npos ){
                        std::string m_character;
                        if( entity.substr(0,1) == "#" ) {
                            if( entity.substr(1,1) == "x"){ // hex
                                m_character = convert_character_code(
                                                  entity.substr(2,entity.size()-2),16);
                            } else { //  decimal
                                m_character = convert_character_code(
                                                  entity.substr(1,entity.size()-1),10);
                            }
                        } else { // html entity
                            if( entities.count(entity) ){
                                m_character = convert_character_code(entities[entity],10);
                            } else {
                                m_character = " * ";
                            }
                        }
                        if( m_character.length() ){
                            cleaned.replace(htmlPos,lastHtmlPos-htmlPos+1,m_character);
                        }
                    }
                    htmlPos = cleaned.find_first_of("&",htmlPos+1);
                    lastHtmlPos = cleaned.find_first_of(";",htmlPos);
                }
                

                return cleaned;
            }



        private:

            std::string convert_character_code(const std::string& code, const int base ){

                char str[4];
                int i_code = static_cast<int>(strtol(code.c_str(),NULL,base));
                sprintf( str, "%c", i_code );
                std::string character = str;
                return character;
            }

            std::map<std::string,std::string> load_entity_map(){
                std::map<std::string,std::string> entities;

                entities.insert(std::make_pair("nbsp",  "32"));
                entities.insert(std::make_pair("ensp",  "32"));
                entities.insert(std::make_pair("emsp",  "32"));
                entities.insert(std::make_pair("thinsp","32"));
                entities.insert(std::make_pair("rdquo", "34"));
                entities.insert(std::make_pair("ldquo", "34"));
                entities.insert(std::make_pair("quot",  "34"));
                entities.insert(std::make_pair("amp",   "38"));
                entities.insert(std::make_pair("lsquo", "39"));
                entities.insert(std::make_pair("rsquo", "39"));
                entities.insert(std::make_pair("endash","45"));
                entities.insert(std::make_pair("emdash","45"));
                entities.insert(std::make_pair("gt",    "62"));
                entities.insert(std::make_pair("lt",    "60"));
                entities.insert(std::make_pair("tilde", "126"));
                entities.insert(std::make_pair("iexcl", "161"));
                entities.insert(std::make_pair("cent",  "162"));
                entities.insert(std::make_pair("pound", "163"));
                entities.insert(std::make_pair("curren","164"));
                entities.insert(std::make_pair("yen",   "165"));
                entities.insert(std::make_pair("brvbar","166"));
                entities.insert(std::make_pair("sect",  "167"));
                entities.insert(std::make_pair("uml",   "168"));
                entities.insert(std::make_pair("copy",  "169"));
                entities.insert(std::make_pair("ordf",  "170"));
                entities.insert(std::make_pair("laquo", "171"));
                entities.insert(std::make_pair("not",   "172"));
                entities.insert(std::make_pair("shy",   "173"));
                entities.insert(std::make_pair("reg",   "174"));
                entities.insert(std::make_pair("macr",  "175"));
                entities.insert(std::make_pair("deg",   "176"));
                entities.insert(std::make_pair("plusmn","177"));
                entities.insert(std::make_pair("sup2",  "178"));
                entities.insert(std::make_pair("sup3",  "179"));
                entities.insert(std::make_pair("acute", "180"));
                entities.insert(std::make_pair("micro", "181"));
                entities.insert(std::make_pair("para",  "182"));
                entities.insert(std::make_pair("middot","183"));
                entities.insert(std::make_pair("cedil", "184"));
                entities.insert(std::make_pair("sup1",  "185"));
                entities.insert(std::make_pair("ordm",  "186"));
                entities.insert(std::make_pair("raquo", "187"));
                entities.insert(std::make_pair("frac14","188"));
                entities.insert(std::make_pair("frac12","189"));
                entities.insert(std::make_pair("frac34","190"));
                entities.insert(std::make_pair("iquest","191"));
                entities.insert(std::make_pair("Agrave","192"));
                entities.insert(std::make_pair("Aacute","193"));
                entities.insert(std::make_pair("Acirc", "194"));
                entities.insert(std::make_pair("Atilde","195"));
                entities.insert(std::make_pair("Auml",  "196"));
                entities.insert(std::make_pair("Aring", "197"));
                entities.insert(std::make_pair("AElig", "198"));
                entities.insert(std::make_pair("Ccedil","199"));
                entities.insert(std::make_pair("Egrave","200"));
                entities.insert(std::make_pair("Eacute","201"));
                entities.insert(std::make_pair("Ecirc", "202"));
                entities.insert(std::make_pair("Euml",  "203"));
                entities.insert(std::make_pair("Igrave","204"));
                entities.insert(std::make_pair("Iacute","205"));
                entities.insert(std::make_pair("Icirc", "206"));
                entities.insert(std::make_pair("Iuml",  "207"));
                entities.insert(std::make_pair("ETH",   "208"));
                entities.insert(std::make_pair("Ntilde","209"));
                entities.insert(std::make_pair("Ograve","210"));
                entities.insert(std::make_pair("Oacute","211"));
                entities.insert(std::make_pair("Ocirc", "212"));
                entities.insert(std::make_pair("Otilde","213"));
                entities.insert(std::make_pair("Ouml",  "214"));
                entities.insert(std::make_pair("times", "215"));
                entities.insert(std::make_pair("Oslash","216"));
                entities.insert(std::make_pair("Ugrave","217"));
                entities.insert(std::make_pair("Uacute","218"));
                entities.insert(std::make_pair("Ucirc", "219"));
                entities.insert(std::make_pair("Uuml",  "220"));
                entities.insert(std::make_pair("Yacute","221"));
                entities.insert(std::make_pair("THORN", "222"));
                entities.insert(std::make_pair("szlig", "223"));
                entities.insert(std::make_pair("agrave","224"));
                entities.insert(std::make_pair("aacute","225"));
                entities.insert(std::make_pair("acirc", "226"));
                entities.insert(std::make_pair("atilde","227"));
                entities.insert(std::make_pair("auml",  "228"));
                entities.insert(std::make_pair("aring", "229"));
                entities.insert(std::make_pair("aelig", "230"));
                entities.insert(std::make_pair("ccedil","231"));
                entities.insert(std::make_pair("egrave","232"));
                entities.insert(std::make_pair("eacute","233"));
                entities.insert(std::make_pair("ecirc", "234"));
                entities.insert(std::make_pair("euml",  "235"));
                entities.insert(std::make_pair("igrave","236"));
                entities.insert(std::make_pair("iacute","237"));
                entities.insert(std::make_pair("icirc", "238"));
                entities.insert(std::make_pair("iuml",  "239"));
                entities.insert(std::make_pair("eth",   "240"));
                entities.insert(std::make_pair("ntilde","241"));
                entities.insert(std::make_pair("ograve","242"));
                entities.insert(std::make_pair("oacute","243"));
                entities.insert(std::make_pair("ocirc", "244"));
                entities.insert(std::make_pair("otilde","245"));
                entities.insert(std::make_pair("ouml",  "246"));
                entities.insert(std::make_pair("divide","247"));
                entities.insert(std::make_pair("oslash","248"));
                entities.insert(std::make_pair("ugrave","249"));
                entities.insert(std::make_pair("uacute","250"));
                entities.insert(std::make_pair("ucirc", "251"));
                entities.insert(std::make_pair("uuml",  "252"));
                entities.insert(std::make_pair("yacute","253"));
                entities.insert(std::make_pair("thorn", "254"));
                entities.insert(std::make_pair("yuml",  "255"));
                entities.insert(std::make_pair("OElig", "338"));
                entities.insert(std::make_pair("oelig", "339"));
                entities.insert(std::make_pair("Scaron","352"));
                entities.insert(std::make_pair("scaron","353"));
                entities.insert(std::make_pair("Yuml",  "376"));
                entities.insert(std::make_pair("circ",  "710"));
                entities.insert(std::make_pair("dagger","8224"));
                entities.insert(std::make_pair("Dagger","8225"));
                entities.insert(std::make_pair("permil","8240"));
                entities.insert(std::make_pair("lsaquo","8249"));
                entities.insert(std::make_pair("rsaquo","8250"));
                entities.insert(std::make_pair("euro",  "8364"));

                return entities;
            }



            std::string identify_encoding( const std::string& text ){
                std::string::size_type pos = text.find("<meta ",0);
                std::string::size_type last = text.find_first_of(">",pos);
                while( pos != std::string::npos ){
                    std::string tag = text.substr(pos,last-pos+1);
                    std::string::size_type chset = tag.find("charset=");
                    std::string::size_type lastChset = tag.find_first_of("\"",chset+8);
                    if( chset != std::string::npos && lastChset != std::string::npos){
                        return tag.substr(chset+8,lastChset-chset+8);
                    }
                    pos = text.find("<meta ",last);
                    last = text.find_first_of(">",pos);
                }
                return "";
            }

    };



    class rtf_filter: public text_filter {
        public:
            std::string operator() ( const std::string& text )
            {
                if( encoding.size() < 1 )
                    encoding = identify_encoding( text );

                std::string cleaned(text);
                remove_newlines(cleaned);
                remove_escaped_braces(cleaned);
                remove_destination_groups(cleaned);
                remove_command_group( "pict", cleaned );
                remove_command_group( "filetbl", cleaned );
                remove_command_group( "fonttbl", cleaned );
                remove_command_group( "colortbl", cleaned );
                remove_command_group( "stylesheet", cleaned );
                remove_control_commands( cleaned );
                remove_parentheses( cleaned );
                replace_hex( cleaned );
                return cleaned;
            }


        private:
            std::string identify_encoding ( const std::string& rtf ){
                std::string::size_type begin = rtf.find_first_of("\\",0);
                std::string::size_type end = rtf.find_first_not_of(
                                                  "abcdefghijklmnopqrstuvwxyz", begin + 1 );

                std::string character_set;
                std::string ansi_code_page;

                while( begin != std::string::npos )
                {
                    std::string control = rtf.substr(begin,end-begin);
                    try {
                        std::string::size_type cpg = control.find_last_not_of("0123456789");
                        if( control.substr(1,cpg) == "ansicpg"){
                            ansi_code_page = control.substr(cpg+1,control.size()-cpg-1);
                        }

                        if( character_set.size() > 0 ){
                            begin = std::string::npos;
                        } else {
                            begin = rtf.find_first_of("\\",end );
                            end = rtf.find_first_not_of(
                                         "abcdefghijklmnopqrstuvwxyz", begin + 1 );
                            if( rtf.substr(end,1) == "-" &&
                                         rtf.find_first_of("1234567890",end) == end+1){
                                end = rtf.find_first_not_of( "1234567890", end+1 );
                            } else if( rtf.find_first_of("1234567890",end) == end ){
                                end = rtf.find_first_not_of( "1234567890", end );
                            }
                        }

                        if( control == "\\ansi" ){
                            character_set = "ansi";
                        } else if ( control == "\\mac" ){
                            character_set = "mac";
                        } else if ( control == "\\pc" || control == "\\pca" ){
                            character_set = "pc";
                        }
                    } catch ( std::exception &e ){
                        std::cerr << "error while getting encoding: ("
                                  << begin << ") " << e.what() << std::endl;
                        exit(EXIT_FAILURE);
                    }

                }

                switch( atoi( ansi_code_page.c_str() ) )
                {
                    case 437:
                    case 850:
                    case 852:
                    case 855:
                    case 857:
                    case 860:
                    case 861:
                    case 863:
                    case 865:
                    case 866:
                    case 869:
                        return "IBM" + ansi_code_page;
                    case 737:
                    case 858:
                    case 932:
                    case 936:
                    case 949:
                    case 950:
                        return "CP"+ ansi_code_page;
                    case 1250:
                    case 1251:
                    case 1252:
                    case 1253:
                    case 1254:
                    case 1255:
                    case 1256:
                    case 1257:
                    case 1258:
                        return "windows-"+ansi_code_page;
                    case 10000:
                        return "macintosh";
                    case 10007:
                        return "maccyrillic";
                    case 10029:
                        return "maccentraleurope";
                    case 65001:
                        return "utf8";
                }

                if( character_set == "mac" ){
                    return "macintosh";
                } else if ( character_set == "pc" || character_set == "pca" ){
                    return "windows-1252";
                } else {
                    return "iso-8859-1";
                }
            }

            void remove_newlines( std::string& rtf )
            {
                try {
                    std::string::size_type pos = rtf.find("\n",0);
                    while(pos != std::string::npos ){
                        if( pos > 0 && rtf.substr(pos-1,1) == "\\" ){
                            rtf.replace(pos-1,2," ");
                            pos = rtf.find("\n",pos);
                        } else {
                            rtf.erase(pos,1);
                            pos = rtf.find("\n",pos-1);
                        }
                    }

                } catch( std::exception & e ){
                    std::cerr << "Error while removing newlines: "
                              << e.what() << std::endl;
                    exit(EXIT_FAILURE);
                }

            }

            void remove_escaped_braces ( std::string& rtf )
            {
                try {
                    std::string::size_type pos = rtf.find( "\\{",0 );
                    while( pos != std::string::npos )
                    {
                        rtf.erase(pos,2);
                        pos = rtf.find( "\\{", pos );
                    }
                    pos = rtf.find( "\\}",0 );
                    while( pos != std::string::npos )
                    {
                        rtf.erase(pos,2);
                        pos = rtf.find( "\\}", pos );
                    }
                } catch ( std::exception &e ) {

                    std::cerr << "error while removing unescaped braces: "
                              << e.what() << std::endl;
                    exit(EXIT_FAILURE);
                }
            }

            void remove_parentheses( std::string& rtf )
            {
                try {
                    std::string::size_type pos = rtf.find( "{",0 );
                    while( pos != std::string::npos ){
                        rtf.erase(pos,1);
                        pos = rtf.find( "{", pos );
                    }
                    pos = rtf.find( "}",0 );
                    while( pos != std::string::npos ){
                        rtf.erase(pos,1);
                        pos = rtf.find( "}", pos );
                    }
                } catch ( std::exception & e ) {

                    std::cerr << "error while removing parentheses: "
                              << e.what() << std::endl;
                    exit(EXIT_FAILURE);
                }
            }

            void replace_hex( std::string& rtf ){

                std::string::size_type pos = rtf.find( "\\'", 0 );
                while( pos != std::string::npos ){
                    try {
                        char str[4];
                        std::string h = "0x" + rtf.substr(pos+2,2);
                        sprintf( str, "%c", static_cast<int>(strtol(h.c_str(),NULL,16)) );
                        rtf.erase(pos,4);
                        rtf.insert(pos,str);
                        pos = rtf.find( "\\'", pos+4 );
                    } catch ( std::exception & e ){
                        std::cerr << "error while converting hexidecimal notation: "
                                  << e.what() << std::endl;
                        exit(EXIT_FAILURE);
                    }
                }
            }

            void remove_control_commands( std::string& rtf )
            {
                std::string::size_type begin = 0;
                std::string::size_type end = 0;
                while( end != std::string::npos && begin != std::string::npos )
                {
                    try {
                        std::string control = rtf.substr(begin,end-begin);
                        if ( control == "\\pard" || control == "\\par" )
                        { // can change this if needed
                            rtf.erase(begin,end-begin);
                            //begin = end;
                        } else if (rtf.substr(begin,2) == "\\'" )
                        {
                            begin = end;
                        } else
                        {
                            rtf.erase(begin,end-begin);
                        }
                        begin = rtf.find_first_of("\\",begin );
                        end = rtf.find_first_not_of(
                                  "abcdefghijklmnopqrstuvwxyz", begin + 1 );
                        if( rtf.substr(end,1) == "-" &&
                                  rtf.find_first_of("1234567890",end) == end+1)
                        {
                            end = rtf.find_first_not_of( "1234567890", end+1 );
                        } else if( rtf.find_first_of("1234567890",end) == end )
                        {
                            end = rtf.find_first_not_of( "1234567890", end );
                        }
                    } catch ( std::exception & e ) {

                        std::cerr << "error while removing control commands: "
                                  << e.what() << std::endl;
                        exit(EXIT_FAILURE);
                    }
                }

            }

            void remove_destination_groups ( std::string& rtf )
            {
                // first remove all the simple stuff   {\*\ctr ... }
                std::string::size_type cruft = rtf.find("{\\*\\",0);
                while( cruft != std::string::npos )
                {
                    try {
                        if( rtf.find_first_not_of(
                                 "abcdefghijklmnopqrstuvwxyz", cruft+4 ) > cruft+4 )
                        {
                            std::string::size_type pos = rtf.find_first_of( "}", cruft+4 );
                            std::string::size_type ppos = rtf.find_first_of( "{", cruft+4 );
                            while( ppos != std::string::npos && ppos < pos )
                            {
                                pos = rtf.find_first_of( "}", pos+1 );
                                ppos = rtf.find_first_of( "{", ppos+1 );
                            }
                            rtf.erase(cruft,pos-cruft+1);
                        }
                        cruft = rtf.find("{\\*\\",cruft);
                    } catch ( std::exception & e ) {

                        std::cerr << "error while removing destination groups: "
                                  << e.what() << std::endl;
                        exit(EXIT_FAILURE);
                    }
                }
            }

            void remove_command_group ( const std::string& erase, std::string& rtf )
            {
                std::string str( "{\\" + erase );
                std::string::size_type cruft = rtf.find(str,0);
                std::string::size_type len = str.size();
                while( cruft != std::string::npos )
                {
                    try {
                        std::string::size_type pos = rtf.find_first_of( "}", cruft+len );
                        std::string::size_type ppos = rtf.find_first_of( "{", cruft+len );
                        while( ppos != std::string::npos && ppos < pos )
                        {
                            pos = rtf.find_first_of( "}", pos+1 );
                            ppos = rtf.find_first_of( "{", ppos+1 );
                        }
                        rtf.erase(cruft,pos-cruft+1);
                        cruft = rtf.find(str,cruft);
                    } catch ( std::exception & e ) {

                        std::cerr << "error while removing command group ("
                                  << erase << "): " << e.what() << std::endl;
                        exit(EXIT_FAILURE);
                    }
                }
            }
    };
}
#endif

