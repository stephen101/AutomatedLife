/*
classes to help with indexing texts (or other things) into a Semantic Graph
*/

#ifndef __SEMANTIC_FILE_READER_HPP__
#define __SEMANTIC_FILE_READER_HPP__

#include <semantic/semantic.hpp>
#include <semantic/filter.hpp>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <cctype>
#if SEMANTIC_HAVE_MSWORD_READER
#include <msword-reader.hpp>
#endif
#if SEMANTIC_HAVE_PDF_READER
#include <pdftotext.h>
#endif
#include <iconv.h>

namespace semantic {


	inline std::string
	convert_to_utf8( const std::string& text, const std::string& encoding )
	{
		std::string lower(encoding);
		std::transform(encoding.begin(),encoding.end(),lower.begin(),tolower);
		
		if( lower == "utf8" || lower == "utf-8" ){
			return text;

		} else if ( encoding.size() > 0 ) {
			iconv_t cd;
			cd = iconv_open( "utf8", encoding.c_str() );
			if( cd != (iconv_t)(-1) && text.size() > 0 ){
				
				std::string::size_type len = text.size();
				char *utfForm = new char[len*3+1];
				std::string mtext = text;
#if ICONV_CONST					
				const char *pin = const_cast<char *>(mtext.c_str());
#else
				char *pin = const_cast<char *>(mtext.c_str());
#endif

				size_t inLeft = len;
				size_t outLeft = len*3+1;
				char *pout = utfForm;
				while( inLeft != 0 ){
					size_t conversion = iconv( cd, &pin, &inLeft, &pout, &outLeft );
					if( conversion == (size_t) -1 ){
						switch( errno ){
							case EILSEQ:
								/* input char encountered that doesn't belong */
								std::cerr << "Character not from source char set: " << strerror(errno) << std::endl;
								delete[] utfForm;
								return text;
							case EINVAL:
								/* incomplete character encountered */
								std::cerr << "Incomplete character or shift sequence: " << strerror(errno) << std::endl;
								delete[] utfForm;
								return text;
							case E2BIG:
								std::cerr << "Ran out of space in the buffer" << std::endl;
							
							default:
								std::cerr << "Iconv error: " << strerror(errno) << std::endl;
								delete[] utfForm;
								return text;
						}
					}
				}
				std::string utf8(utfForm);
				delete[] utfForm;
				iconv_close(cd);
				if( utf8.length() > text.length() ) {
					//std::cout << utf8.substr(0,text.length()+1) << std::endl;
					return utf8.substr(0,text.length()+1 );

				} else {
					return utf8;
				}
				
			}
		}
		return text;
	}
	
	
	
	class file_reader {
		public:
			file_reader(){ pdfLayout = "layout"; }
			
			std::string operator() (const std::string& filename, const std::string& encoding){
				std::string prev_encoding = default_encoding;
				set_default_encoding( encoding );
				std::string text = file_to_text( filename );
				set_default_encoding( prev_encoding );
				smart_quotes_filter filter;
				return filter(text);
			}
			
			std::string operator() (const std::string& filename){
				/* supported mime_types:
				 *		application/msword
				 *      application/rtf
				 *		application/xhtml+xml
				 *		text/plain
				 *		text/html
				 */	
				std::string text = file_to_text( filename );
				smart_quotes_filter filter;
				return filter(text);
			}
			
			std::string operator() (std::istream& filestream, const std::string& mime_type){
				/* supported mime_types:
				 *		application/rtf
				 *		application/xhtml+xml
				 *		text/plain
				 *		text/html
				 */	
				std::string text = stream_to_text( filestream, mime_type );
				smart_quotes_filter filter;
				return filter(text);
			}
			
			std::string operator() (std::istream& filestream, const std::string& mime_type, const std::string& encoding){
				std::string prev_encoding = default_encoding;
				set_default_encoding( encoding );
				std::string text = stream_to_text( filestream, mime_type );
				set_default_encoding( prev_encoding );
				smart_quotes_filter filter;
				return filter(text);
			}

			
			void set_default_encoding(const std::string& encoding )
			{
				default_encoding = encoding;
			}
			
			void set_pdfLayout(const std::string& layout){
				pdfLayout = layout;
			}

			std::string pdfLayout;
			
		private:
			std::string default_encoding;
			
			std::string stream_to_text( std::istream& filestream, const std::string& mime_type ){
				std::string line;
				std::string text;
				std::string encoding;
				while( std::getline( filestream, line )) 
				{
					text.append(line+'\n');
				}
				
				if( mime_type == "application/rtf" ){
					rtf_filter filter;
					text = filter(text);
					encoding = filter.get_encoding();
				
				} else if( mime_type == "text/html" || mime_type == "application/xhtml+xml" ){
					html_filter filter;
					text = filter( text );
					encoding = filter.get_encoding();
				
				} else if( mime_type == "text/plain" ){
					// no filter!
					encoding = default_encoding;

				} else {				
					std::cerr << "Unsupported MIME type: " << mime_type << std::endl;
					std::cerr << "Supported MIME types include:" << std::endl;
					std::cerr << supported_mime_types() << std::endl;
					exit(EXIT_FAILURE);
				}
				if( encoding.size() == 0 && default_encoding.size() == 0 ){
					encoding == "iso-8859-1";
				} else if ( encoding.size() == 0 ){
					encoding = default_encoding;
				} 
				return convert_to_utf8( text, encoding );
			}
			
			
			std::string file_to_text(const std::string& filename){
				std::string::size_type pos;
				std::string ext;
				std::string text;
				std::string encoding;
				std::string utf8;
				
				pos = filename.find_last_of(".");
				ext = filename.substr(pos+1,filename.size()-pos-1);
				std::string lower(ext);
				std::transform(ext.begin(),ext.end(),lower.begin(),tolower);
				ext = lower;
				
				if( ext == "doc" ){

					// MS Word Document
#if SEMANTIC_HAVE_MSWORD_READER
					text = msword::toText( filename );
					encoding = msword::get_encoding();
					return convert_to_utf8( text, encoding );
					
#else
					std::cerr << "Warning: Could not read file: " << filename << std::endl;
					return text;
#endif
				
				} else if ( ext == "pdf" ){

					// PDF Document
#if SEMANTIC_HAVE_PDF_READER
					// no need to convert to UTF-8, as this is already done.
					return pdf::toText( filename, pdfLayout );
#else
					std::cerr << "Warning: Could not read file: " << filename << std::endl;
					return text;
#endif

				} else {

					std::string line;
					std::ifstream file(filename.c_str(), std::ios_base::in);
					
					if( ! file ){
						std::cerr << "Warning: Could not read file!" << std::endl;
						exit(EXIT_FAILURE);
					}
					while( std::getline( file, line )) 
					{
						text.append(line+'\n');
					}
					file.clear();
					file.close();
									
				
				
					if ( ext == "rtf" ){
					
						rtf_filter filter;
						text = filter(text);
						encoding = filter.get_encoding();
	
					} else if ( ext == "html" || ext == "htm" ){
						
						html_filter filter;
						
						text = filter(text);
						encoding = filter.get_encoding();
					
					}
				}
				if( encoding.size() == 0 ){
					if( default_encoding.size() == 0 ){
						encoding = "iso-8859-1";
					} else {
						encoding = default_encoding;
					}
				}
				
				std::string converted = convert_to_utf8( text, encoding );
				
				return converted;
			}
			

			

			std::string supported_mime_types()
			{
				return "ISTREAM:\napplication/rtf\napplication/xhtml+xml\ntext/html\ntext/plain\n\nFILEPATH:\napplication/msword\napplication/rtf\napplication/xhtml+xml\ntext/html\ntext/plain";
			}


	};
}
#endif
