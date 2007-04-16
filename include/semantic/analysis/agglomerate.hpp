#ifndef __SEMANTIC_ANALYSIS_AGGLOMERATE_HPP__
#define __SEMANTIC_ANALYSIS_AGGLOMERATE_HPP__

/*

THIS FILE IS DEPRECATED

*/

namespace semantic { namespace analysis {
	
	namespace detail {
		template <typename Float, typename size_type = unsigned int>
		class symmetric_matrix {
			typedef Float *FloatPtr;
			typedef Float **FloatDPtr;
			public:
				explicit symmetric_matrix(size_type size) : m_size(size) {
					if (m_size < 1) m_size = 1;
					build();
				}
				
				~symmetric_matrix() {
					for(size_type i = 0; i < m_size; i++) delete m_data[i];
					delete m_data;
				}
				
				void zeroAll() {
					setAllTo((Float)0);
				}
				
				void setAllTo(Float v) {
					for(size_type i = 0; i < m_size; i++) {
						for(size_type k = 0; k < i + 1; k++) {
							m_data[i][k] = v;
						}
					}
				}
				
				Float &operator() (size_type row, size_type col) {
					if (col > row) { size_type t = row; row = col; col = t; }
					return m_data[row][col];
				}
			private:
				void build() {
					m_data = new FloatPtr[m_size];
					
					for(size_type i = 0; i < m_size; i++) {
						m_data[i] = new Float[i+1];
					}
				}
				
				FloatDPtr m_data;
				size_type m_size;
		}; // class symmetric_matrix
		
		template <class Vector>
		class agglomerate_gravity_helper {
			typedef typename Vector::size_type 		size_type;
			typedef typename Vector::value_type		point_type;
			public:
				agglomerate_gravity_helper(Vector &points, float t) : m_points(points), m_threshold2(t*t), m_timescale(0.1) {}
				
				float step() {
					// a vector for the locations of our new points
					Vector new_points(m_points.size());
					// distance matrix
					symmetric_matrix<float> distances(m_points.size());
					distances.setAllTo(-1.0f);
					// a value to keep track of the total movement
					float total_movement = 0;
					
					// compute the distance matrix
					build_distance_matrix(distances);
					
					// now perform the movement for each node
					for(size_type i = 0; i < m_points.size(); i++) {
						point_type vec(0,0,0);
						size_type num = 0;
						for(size_type k = 0; k < m_points.size(); k++) {
							if (distances(i, k) < m_threshold2) {
								// within our threshold distance -- it will effect the move
								// from our current point to this new point
								vec += m_points[k] - m_points[i];
								num++;
							}
						}
						
						vec /= num;
						vec *= m_timescale;
						
						new_points[i] = m_points[i] + vec;
						total_movement += point3d_len(vec);
					}
					
					m_points.swap(new_points);
					
					return total_movement;
				}
				
				void set_threshold(float t) {
					m_threshold2 = t * t;
				}
			private:
				void build_distance_matrix(symmetric_matrix<float> &distances) {
					for(size_type i = 0; i < m_points.size(); i++) {
						for(size_type k = 0; k < m_points.size(); k++) {
							if (distances(i, k) == -1) {
								// no value set yet
								distances(i, k) = m_points[i].dist2(m_points[k]);
							}
						}
					}
				}
			
				Vector &m_points;
				float m_threshold2;
				float m_timescale;
		}; // class agglomerate_gravity_helper
	} // namespace detail
	
	
	template <class Vector>
	inline void agglomerate_points_by_gravity(Vector &points, float threshold)
	{
		
	}
	
} } // namespace semantic::analysis


#endif
