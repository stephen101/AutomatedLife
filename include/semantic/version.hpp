#ifndef __SEMANTIC_VERSION_HPP__
#define __SEMANTIC_VERSION_HPP__

#include <semantic/config.hpp>
#include <iostream>

#define SEMANTIC_VERSION_STRING "2.0.3"
#define SEMANTIC_VERSION_NUMBER 20003 // format Mmmrr (M = Major, m = minor, r = revision)
#define SEMANTIC_PACKAGE_NAME "Semantic Engine"
#define SEMANTIC_SVN_REVISION "$Revision$"

#define SEMANTIC_COPY_YEAR "2003-2007"
#define SEMANTIC_COPY_HOLDERS "Aaron Coburn, Gabriel Schine, John Cuadrado"

namespace semantic
{
	template <class O>
	inline void
	print_version_info(O &o) {
		o << "Version: " << SEMANTIC_PACKAGE_NAME << " v" << SEMANTIC_VERSION_STRING << std::endl << std::endl;

		o << "Copyright: (c) " << SEMANTIC_COPY_YEAR << " " << SEMANTIC_COPY_HOLDERS << std::endl << std::endl;

		o << "Storage engines available: " << SEMANTIC_STORAGE_ENGINES << std::endl;
	}	
} // namespace semantic

#endif
