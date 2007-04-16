#ifndef __SEMANTIC_ANALYSIS_MATH_HPP__
#define __SEMANTIC_ANALYSIS_MATH_HPP__

#include <math.h>

namespace semantic {
	namespace math {
		
		// redefine pow function
		template <typename T, typename T2> inline T pow(T base, T2 exp) {
			return std::pow(base, exp);
			// special cases
/*			if (exp == 0) return 1;
			if (exp == 1) return base;
			if (exp == -1) return 1/base;
			if (exp == 2) return base*base;
			if (exp == -2) return 1/(base*base); // recursive inline - no slowdown
			
			// else return stdlib's pow function
			return (T) std::pow((double) base, (double) exp);*/
		}
		
	} // namespace math
	
} // namespace semantic

#endif
