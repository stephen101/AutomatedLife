


#ifndef __SEMANTIC_ANALYSIS_UTILITY_HPP__
#define __SEMANTIC_ANALYSIS_UTILITY_HPP__

#include <boost/graph/properties.hpp>
#include <map>

#include <iostream>

namespace semantic {
	template <typename T>
	inline T square(T x) {
		return x*x;
	}
	
	// returns the weight distribution of edges in Graph as a histogram
	// with resolution as passed. results are placed into resultMap:
	//
	//	key: 0
	//		represents the # of edges with weight 0-resultion
	//	key: 1*resultion
	//		represents the # of edges with weight resolution-(2*resolution)
	// 	and so on...
	//
	//	keys for WeightMap must be of edge_descriptor type for Graph
	// 	keys for ResultMap must be WeightMap::value_type
	template <class Graph, class WeightMap, class ResultMap>
	inline void
	get_weight_distribution(const Graph &g, const WeightMap w, ResultMap r, typename property_traits<WeightMap>::value_type res)
	{
		typedef typename property_traits<ResultMap>::value_type count_type;
		typedef typename property_traits<WeightMap>::value_type weight_type;
		typedef typename graph_traits<Graph>::edge_descriptor edge;
		typedef typename graph_traits<Graph>::edge_iterator edge_iterator;
		
		weight_type min, max;
		boost::tie(min,max) = map_min_max(edges(g), w);
		
		// first go from 0-max in res incremenents and initialize our ResultMap to zeros
		// - go to max+res to ensure max inclusive in range
		for(weight_type i = 0; i < max + res; i += res) put(r, i, 0);
		
		// now go through all the edges in this graph
		edge_iterator ei, eend;
		for(boost::tie(ei, eend) = edges(g); ei != eend; ++ei) {
			weight_type weight = get(w, *ei);
			if (weight > max) weight = max;
			int pos = (int) floor(weight/res);
			weight_type key = pos * res;
			put(r, key, get(r, key)+1); // increment the value in r
		}
	}
	
	// accumulates all the values in a map
	template <class Iterator, class Map>
	inline typename property_traits<Map>::value_type
	map_accumulate(Iterator i, Iterator end, Map &m) 
	{
		typename property_traits<Map>::value_type sum = 0;
		for(; i != end; ++i) {
			sum += get(m, *i);
		}
		
		return sum;
	}
	
	template <class Iterator, class Map>
	inline typename property_traits<Map>::value_type
	map_accumulate(std::pair<Iterator, Iterator> p, Map &m) 
	{
		return map_accumulate(p.first, p.second, m);
	}
	
	// min and max functions that operate on a map
	template <class Iterator, class Map>
	inline std::pair<
			typename property_traits<Map>::value_type,
			typename property_traits<Map>::value_type
		>
	map_min_max(Iterator i, Iterator end, const Map &m) 
	{
		typedef typename property_traits<Map>::value_type value_type;
		value_type min, max;
		min = (std::numeric_limits<value_type>::max)();
		max = 0;
		
		for(; i != end; ++i) {
			value_type &t = get(m, *i);
			if (t < min) min = t;
			if (t > max) max = t;
		}
		
		return std::make_pair(min, max);
	}
	
	template <class Iterator, class Map>
	inline std::pair<
			typename property_traits<Map>::value_type,
			typename property_traits<Map>::value_type
		>
	map_min_max(std::pair<Iterator, Iterator> i, const Map &m) 
	{
		return map_min_max(i.first, i.second, m);
	}
	
	// allows us to extract the top n items from a result map or pass back
	// a sorted list
	template <class AssociativeContainer, class Inserter>
	void extract_keys_sorted_by_value(const AssociativeContainer &c, Inserter out, int num = 0)
	{
		typedef typename AssociativeContainer::mapped_type 	value_type;
		typedef typename AssociativeContainer::key_type 	key_type;
		typedef std::multimap<value_type, key_type, std::greater<value_type> > reverse_map;
		reverse_map m; 	// reverse (greatest keys first)
		// populate our multimap
		for(typename AssociativeContainer::const_iterator it = c.begin(); it != c.end(); ++it) {
			m.insert(std::pair<value_type, key_type>(it->second, it->first));
		}
		
		if (num == 0) num = c.size(); // if num = 0, we want *all* elements sorted
		
		for(typename reverse_map::iterator it = m.begin(); num > 0 && it != m.end(); num--, ++it) {
			// start putting values into our output
			*out = it->second;
		}
	}
	
	// a 3d point class
	// - this class could be replaced with boost::ublas::bounded_vector<D, 3>
	template <typename D>
	class point3d {
		typedef point3d<D> T;
		public:
			typedef D value_type;
		
			point3d() {set(0,0,0);}
			point3d(const D &x, const D &y, const D &z) {set(x,y,z);}
			template <class Dec> explicit point3d(const point3d<Dec> &p) {set(p);}
			
			void set(const D &x, const D &y, const D &z) { m_pt[0] = x; m_pt[1] = y; m_pt[2] = z; }
			template <class Dec> void set(const point3d<Dec> &p) { set(p.x(), p.y(), p.z()); }
			
			template <class Rand> void set_rand(Rand &r, D range = 1.0) {
				for(int i = 0; i < 3; i++) {
					m_pt[i] = ((D) ((D)(r() + Rand::min_value)/(D)(Rand::max_value - Rand::min_value)) - (D)0.5) * (D)2.0 * range;
				}
			}
			
			template <class NPoint, typename DType>
			NPoint as_2d_point_type() {
				return NPoint((DType) x(), (DType) y());
			}
			
			D operator[] (int i) const { return m_pt[i]; } // should we throw an index out of range?
			D &operator[] (int i) { return m_pt[i]; }
			D &x() { return m_pt[0]; }
			D &y() { return m_pt[1]; }
			D &z() { return m_pt[2]; }
			D x() const { return m_pt[0]; }
			D y() const { return m_pt[1]; }
			D z() const { return m_pt[2]; }
			
			// unary
			T operator-() { return T(-x(), -y(), -z()); }
			T operator+() { return T(*this); }
			
			// assignment/binary
			T operator+(const T p) const { return T(p.x() + x(), p.y() + y(), p.z() + z()); }
			T operator-(const T p) const { return T(x() - p.x(), y() - p.y(), z() - p.z()); }
			T& operator -=(const T p) { set(*this - p); return *this; }
			T& operator +=(const T p) { set(*this + p); return *this; }
			template <typename N> T operator*(const N n) const { return T((float)x()*(float)n, (float)y()*(float)n, (float)z()*(float)n); }
			template <typename N> T operator/(const N n) const { return T((float)x()/(float)n, (float)y()/(float)n, (float)z()/(float)n); }
			template <typename N> T& operator*=(const N n) { set(*this*n); return *this; }
			template <typename N> T& operator/=(const N n) { set(*this/n); return *this; }
			T& operator=(const T p) { set(p); return *this; }
			
			// tests
			bool operator==(const T &p) const { return (p.x() == x() && p.y() == y() && p.z() == z()); }
			bool operator!=(const T &p) const { return !(*this == p); }
		
			template <typename N>
			D dist(const point3d<N> &p) const {
				return (D) sqrt(dist2(p));
			}
			
			template <typename N>
			D dist2(const point3d<N> &p) const {
				D t[3];
				for(int i = 0; i < 3; i++) t[i] = m_pt[i] - p[i];
				return (D) (t[0]*t[0] + t[1]*t[1] + t[2]*t[2]);				
			}
			
			T absolute() const {
				return T(t_abs(x()), t_abs(y()), t_abs(z()));
			}
			
			D length() const {
				T p(0,0,0);
				return dist(p);
			}
			
			T normalize() {
				return (*this)/length();
			}
			
		private:
			D m_pt[3];
	}; // class point3d
	
	// convenience function
	template <typename D>
	inline const D
	point3d_dist(const point3d<D> &p1, const point3d<D> &p2) {
		return p1.dist(p2);
	}
	template <typename D>
	inline const D
	point3d_dist2(const point3d<D> &p1, const point3d<D> &p2) {
		return p1.dist2(p2);
	}
	template <typename D>
	inline const D
	point3d_len(const point3d<D> &p) {
		return p.length();
	}
	
	// output function
	template <typename D> inline
	std::ostream &operator<<(std::ostream &o, const point3d<D> &pt) {
		o << "(" << pt.x() << ", " << pt.y() << ", " << pt.z() << ")";
		return o;
	}
	
	// returns a pair of points indicating the bounding rect min and max points
	template <typename D, class Map>
	inline void 
	get_bounding_rect(const Map &pt, point3d<D> &min, point3d<D> &max) {
		get_bounding_rect(pt.begin(), pt.end(), min, max);
	}
	
	template <typename D, class Iterator>
	inline void
	get_bounding_rect(Iterator i, Iterator i_end, point3d<D> &min, point3d<D> &max) {
		D min_v = (std::numeric_limits<D>::min) (), max_v = (std::numeric_limits<D>::max) ();
		min.set(max_v, max_v, max_v), max.set(min_v, min_v, min_v);
		for(; i != i_end; ++i) {
			for(int k = 0; k < 3; k++) {
				if ((*i)[k] > max[k]) max[k] = (*i)[k];
				if ((*i)[k] < min[k]) min[k] = (*i)[k];
			}			
		}
	}
	
	// this class returns true once the value that is passed to it
	// stops changing (based on a delta-factor)
	template <typename ValueType>
	class value_stopped_changing {
		public:
			explicit value_stopped_changing(double delta = 0.001, unsigned int count = 2) : m_delta(delta), m_count(count) {}
			bool operator () (ValueType val) {
				return check_all(val);
			}
			
			bool check_all(ValueType val) {
				if (m_last.size() < m_count) { m_last.push_front(val); return false; }
				for(typename std::list<ValueType>::iterator i = m_last.begin(); i != m_last.end(); ++i) {
					if (t_abs(val - *i) > t_abs(*i * m_delta)) return false;
				}
				m_last.push_front(val);
				if (m_last.size() > m_count) m_last.pop_back();
				return true;
			}
		private:
			std::list<ValueType> m_last;
			double m_delta;
			unsigned int m_count;
	}; // class value_stopped_changing
}


#endif
