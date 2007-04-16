/*
provides an empty SubgraphPolicy implementation.

this is useful if you are working with a SEGraph that will only reside in memory
for the duration of a single process
*/

#ifndef __SEMANTIC_SUBGRAPH_NONE_HPP__
#define __SEMANTIC_SUBGRAPH_NONE_HPP__



namespace semantic {
	template <class SEBase>
	class NoSubgraphPolicy : public SEBase {
		public:
			typedef SEBase base_type;
			
			template <class Iterator, class WeightingPolicy>
			void want_vertices(Iterator i, Iterator i_end, WeightingPolicy w) {}
			
			void did_clear() {SEBase::did_clear();}
			void will_clear() {SEBase::will_clear();}
	};
}

#endif
