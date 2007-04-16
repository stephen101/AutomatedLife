/*

this file contains exceptions that may be thrown during SE operation

*/

#ifndef __SEMANTIC_EXCEPTIONS_HPP__
#define __SEMANTIC_EXCEPTIONS_HPP__

#include <semantic/utility.hpp>
#include <string>

namespace semantic
{
	
	template <typename T>
	struct VertexNotFoundException : std::exception {
		const T &id;
		char message[1024];
		VertexNotFoundException(const T &i) : id(i) {
			sprintf(message, "%s", ("The requested vertex was not found in the graph: " + to_string(id)).c_str());
		}
		const char* what() const throw() { return message; }
	};
	// when does this come up?
	// - when you ask for a node's id by content and type and it can't be found in the index
	struct VertexContentNotFoundException : public VertexNotFoundException<std::string> {
		VertexContentNotFoundException(std::string s) : VertexNotFoundException<std::string>(s) {}
	};
	
	// - when you ask for a specific vertex ID and it isn't found in the index -- this really shouldn't happen
	struct IdNotFoundException : std::exception {
		const char* what() const throw() { return "The vertex Id requested was not found"; }
	};
	
} // namespace semantic

#endif
