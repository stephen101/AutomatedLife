#ifndef __SEMANTIC_FILE_FINDER_HPP__
#define __SEMANTIC_FILE_FINDER_HPP__


#include <iostream>
#include <cctype>
#include <string>
#include <vector>
#include <set>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/exception.hpp>


namespace semantic {

	class file_finder {
		public:
			file_finder(const std::string& node) : starting_node(node) {}
		
			
			void add_file_ext(const std::string& extension)
			{
				file_extensions.insert(extension);
			}
			void add_file_ext(const std::vector<std::string>& extensions )
			{
				std::vector<std::string>::const_iterator pos;
				for( pos = extensions.begin(); pos != extensions.end(); ++pos )
				{
					add_file_ext( *pos );
				}
			}
			
			std::vector<std::string> get_filenames()
			{
				
				boost::filesystem::path full_path(starting_node,boost::filesystem::native);
				bool exists, is_dir;
				try {
					exists = boost::filesystem::exists(full_path);
				} catch (boost::filesystem::filesystem_error &e) {
					std::cerr << "Error fetching filenames: " << e.what() << std::endl;
					return filenames; // get out!
				}
				
				if( exists )
				{
					try {
						is_dir = boost::filesystem::is_directory(full_path);
					} catch (boost::filesystem::filesystem_error &e) {
						std::cerr << "Error reading path: " << full_path.native_file_string() << " (" << e.what() << ")" << std::endl;
						return filenames; // get out
					}
					if( is_dir )
					{
						try {
							recurse_directory( full_path );
						} catch (boost::filesystem::filesystem_error &e) {
							std::cerr << "Error recursing directory: " << full_path.native_file_string() << " (" << e.what() << ")" << std::endl;
							return filenames;
						}
					} else 
					{
						add_file( full_path.string() );
					}
				} else 
				{
					std::cerr << "Not found: " << full_path.native_file_string() << std::endl;
				}					
				return filenames;				
			}
			
			
		
		
		private:
			std::string starting_node;
			std::vector<std::string> filenames;
			std::set<std::string> file_extensions;
			
			void recurse_directory( const boost::filesystem::path& directory )
			{	
				boost::filesystem::directory_iterator end;
				for( boost::filesystem::directory_iterator dir_itr( directory ); dir_itr != end; ++dir_itr )
				{
					if( boost::filesystem::is_directory( *dir_itr ) )
					{
						try {
							recurse_directory( *dir_itr );
						} catch (boost::filesystem::filesystem_error &e) {
							std::cerr << "Error recursing directory: " << (*dir_itr).native_file_string() << " (" << e.what() << ")" << std::endl;
							continue;
						}
					} else 
					{
						add_file( (*dir_itr).native_file_string() );
					}
				}
			}
			
			void add_file( const std::string& filename )
			{
				std::string::size_type pos = filename.find_last_of(".");
				if( pos != std::string::npos )
				{
					std::string ext = filename.substr( pos+1, filename.size()-pos );
					std::string lower(ext);
					std::transform(ext.begin(),ext.end(),lower.begin(),tolower);
				
					if( file_extensions.count( lower ) > 0 )
					{
						filenames.push_back( filename );
					}
				}
			}

	}; // class filesystem

} /* namespace semantic */

#endif
