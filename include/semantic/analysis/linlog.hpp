/*
	The logic in this file is almost entirely appropriated from Alexander Noack's LinLog implementation
*/

//#define __LINLOG_PROFILE

#include <semantic/semantic.hpp>
#include <boost/shared_ptr.hpp>
#include <semantic/analysis/utility.hpp>
#include <semantic/analysis/math.hpp>
#include <math.h>

#include <boost/random/linear_congruential.hpp>
#include <time.h>

#include <numeric>
#include <functional>

#include <iostream>

#ifdef __LINLOG_PROFILE
// timer
#include <boost/timer.hpp>
using boost::timer;
#endif

#ifndef __SEMANTIC_ANALYSIS_LINLOG_HPP__
#define __SEMANTIC_ANALYSIS_LINLOG_HPP__

namespace semantic { namespace analysis {
	
	namespace detail {
		
		template <class point, typename weight_type, typename size_type> class OcTree; // defined below
		
		// a helper class to help with the Barnes-Hut minimizer algorithm
		template <class Maps>
		class MinimizerBarnesHutHelper {
			typedef double float_type;
			typedef typename Maps::weight_type weight_type;
			typedef typename Maps::point_type point;
			typedef typename Maps::size_type size_type;
			typedef boost::shared_ptr<OcTree<point, weight_type, size_type> > OcTreePtr;
			
			private:
				void computeRepuFactor() {
					m_attrSum = 0;
					for(size_type i = 0; i < m_maps.weight.size(); i++) {
						m_attrSum = accumulate(m_maps.weight[i].begin(), m_maps.weight[i].end(), m_attrSum);
					}
					m_repuSum = 0;
					m_repuSum = accumulate(m_maps.repulsion.begin(), m_maps.repulsion.end(), m_repuSum);

//					std::cerr << "m_attrSum = " << m_attrSum << " m_repuSum = " << m_repuSum << std::endl; //debug

					m_finalRepuFactor = 1.0;
					if (m_repuSum > 0 && m_attrSum > 0) {
						m_finalRepuFactor = m_attrSum / m_repuSum / m_repuSum * (float_type) pow(m_repuSum, 0.5 * (m_attrExp - m_repuExp));
					}
				}
				
				float_type getRepulsionEnergy(size_type i, OcTreePtr tree, bool 
					#ifdef __LINLOG_PROFILE
					noTimer
					#endif 
					 = false) {
					if (!tree || (tree->isLeaf() && tree->index() == i)) {return 0;}
					
					#ifdef __LINLOG_PROFILE
					// timer
					timer t_timer;
					timer t_small;
					#endif
					
					float_type dist2 = point3d_dist2(m_maps.position[i], tree->position());
					
					#ifdef __LINLOG_PROFILE
					timer_small_one += t_small.elapsed();
					#endif
					float_type width = 2*tree->width(); // squaring things is faster than sqrt()
					if (!tree->isLeaf() && dist2 < width*width) {
						float_type energy = 0;
						for(int k = 0; k < 8; k++) {
							energy += getRepulsionEnergy(i, tree->child(k), true);
						}
						#ifdef __LINLOG_PROFILE
						if (!noTimer) timer_getRepulsionEnergy += t_timer.elapsed();
						#endif
						return energy;
					}
					
					#ifdef __LINLOG_PROFILE
					t_small.restart();
					count_getRepulsionEnergy++;
					#endif
					
					float_type ret = (m_repuExp == 0)
						? -m_repuFactor * m_maps.repulsion[i] * tree->weight() * (float_type) log(sqrt(dist2))
						: -m_repuFactor * m_maps.repulsion[i] * tree->weight() * (float_type) pow(sqrt(dist2), m_repuExp) / m_repuExp;
/*					float_type ret = -m_repuFactor * m_maps.repulsion[i] * tree->weight() * (float_type) log(sqrt(dist2));*/
					#ifdef __LINLOG_PROFILE
					if (!noTimer) timer_getRepulsionEnergy += t_timer.elapsed();
					timer_small_two += t_small.elapsed();
					#endif
					
					return ret;
				}
				
				float_type getAttractionEnergy(size_type i) {
					#ifdef __LINLOG_PROFILE
					// timer
					timer t_timer;
					#endif
					float_type energy = 0;
					
					for(size_type k = 0; k < m_maps.adjacency[i].size(); k++) {
						if (m_maps.adjacency[i][k] == i) continue;
						float_type dist = point3d_dist(m_maps.position[m_maps.adjacency[i][k]], m_maps.position[i]);	
						energy += m_maps.weight[i][k] * (float_type)pow(dist, m_attrExp) / m_attrExp;
					}
					
					#ifdef __LINLOG_PROFILE
					timer_getAttractionEnergy += t_timer.elapsed();
					#endif
					
					return energy;
				}
				
				float_type getGravitationEnergy(size_type i) {
					#ifdef __LINLOG_PROFILE
					// timer
					timer t_timer;
					#endif
					
					float_type dist = point3d_dist(m_maps.position[i], m_baryCenter);
					float_type ret = m_gravFactor * m_repuFactor * m_maps.repulsion[i] 
										* (float_type) pow(dist, m_attrExp) / m_attrExp;
					#ifdef __LINLOG_PROFILE
					timer_getGraviationalEnergy += t_timer.elapsed();
					#endif
					return ret;
				}
				
				float_type getEnergy(size_type i, OcTreePtr t) {
					#ifdef __LINLOG_PROFILE
					timer t_timer;
					#endif
					float_type ret = getRepulsionEnergy(i,t) + getAttractionEnergy(i) + getGravitationEnergy(i);
					#ifdef __LINLOG_PROFILE
					timer_getEnergy += t_timer.elapsed();
					#endif
					return ret;
				}
				
				float_type addRepulsionDir(size_type i, OcTreePtr t, point &dir, bool 
					#ifdef __LINLOG_PROFILE
					noTimer
					#endif 
					 = false) {
					if (!t || (t->isLeaf() && t->index() == i) || m_maps.repulsion[i] == 0) return 0;
					#ifdef __LINLOG_PROFILE
					timer t_timer;
					#endif
					float_type dist = point3d_dist(m_maps.position[i], t->position());
					if (!t->isLeaf() && dist < 2 * t->width()) {
						float_type dir2 = 0;
						for(int k = 0; k < 8; k++) {
							if (t->child(k)) dir2 += addRepulsionDir(i, t->child(k), dir, true);
						}
						#ifdef __LINLOG_PROFILE
						if (!noTimer) timer_addRepulsionDir += t_timer.elapsed();
						#endif
						return dir2;
					}
					
					if (dist != 0) {
						float_type tmp = m_repuFactor * m_maps.repulsion[i] * t->weight() * (float_type) pow(dist, m_repuExp-2);
						dir -= (t->position() - m_maps.position[i]) * tmp;
						#ifdef __LINLOG_PROFILE
						if (!noTimer) timer_addRepulsionDir += t_timer.elapsed();
						#endif
						return tmp * t_abs(m_repuExp - 1);
					}
					#ifdef __LINLOG_PROFILE
					if (!noTimer) timer_addRepulsionDir += t_timer.elapsed();
					#endif
					return 0;
				}
				
				float_type addAttractionDir(size_type i, point &dir) {
					#ifdef __LINLOG_PROFILE
					timer t_timer;
					#endif
					float_type dir2 = 0;
					
					for(size_type k = 0; k < m_maps.adjacency[i].size(); k++) {
						float_type dist = point3d_dist(m_maps.position[m_maps.adjacency[i][k]], m_maps.position[i]);
						if (dist == 0) continue;
						float_type tmp = m_maps.weight[i][k] * (float_type) pow(dist, m_attrExp - 2);
						dir2 += tmp * t_abs(m_attrExp - 1);
						dir += (m_maps.position[m_maps.adjacency[i][k]] - m_maps.position[i]) * tmp;
					}

					#ifdef __LINLOG_PROFILE
					timer_addAttractionDir += t_timer.elapsed();
					#endif
					return dir2;
				}
				
				float_type addGravitationDir(size_type i, point &dir) {
					#ifdef __LINLOG_PROFILE
					timer t_timer;
					#endif
					float_type dist = point3d_dist(m_maps.position[i], m_baryCenter);
					float_type tmp = m_gravFactor * m_repuFactor * m_maps.repulsion[i] * (float_type) pow(dist, m_attrExp - 2);
					dir += (m_baryCenter - m_maps.position[i]) * tmp;
					#ifdef __LINLOG_PROFILE
					timer_addGravitationDir += t_timer.elapsed();
					#endif
					return tmp * t_abs(m_attrExp - 1);
				}
				
				point getDirection(size_type i, OcTreePtr t) {
					#ifdef __LINLOG_PROFILE
					timer t_timer;
					#endif
					point dir(0,0,0);
					
					float_type dir2 = addRepulsionDir(i, t, dir) + addAttractionDir(i, dir) + addGravitationDir(i, dir);
					
					if (dir2 != 0) {
						// normalize force vector with second derivation of energy
						dir /= dir2;
						
						// ensure that the length of dir is at most 1/8
						// of the maximum Euclidean distance between nodes
						float_type length = point3d_dist(point(0,0,0), dir);
						if (length > t->width()/8) {
							length /= t->width()/8;
							dir /= length;
						}
					} else {
						dir.set(0,0,0);
					}
					#ifdef __LINLOG_PROFILE
					timer_getDirection += t_timer.elapsed();
					#endif
					return dir;
				}
			
				void computeBaryCenter() {
					m_baryCenter.set(0,0,0);
					
					for(size_type i = 0; i < m_maps.repulsion.size(); i++) {
						m_baryCenter += m_maps.position[i] * m_maps.repulsion[i];
					}
					
					if (m_repuSum > 0) {
						m_baryCenter /= m_repuSum;
					}
				}
				
				void buildOcTree(OcTreePtr &p) {
					#ifdef __LINLOG_PROFILE
					timer t_timer;
					#endif
					point3d<float> minPos, maxPos;
					const float max = (std::numeric_limits<float>::max)();
					const float min = (std::numeric_limits<float>::min)();
					minPos.set(max, max, max); maxPos.set(min, min, min);
					
					for(size_type i = 0; i < m_maps.position.size(); i++) {
						if (m_maps.repulsion[i] == 0) continue;
						for(int k = 0; k < 3; k++) {
							if(m_maps.position[i][k] < minPos[k]) minPos[k] = m_maps.position[i][k];
							if(m_maps.position[i][k] > maxPos[k]) maxPos[k] = m_maps.position[i][k];
						}
					}
					
					// add nodes with non-zero repuWeight to the octree
					p = OcTreePtr();
					for(size_type i = 0; i < m_maps.position.size(); i++) {
						if (m_maps.repulsion[i] == 0) continue;
						if (!p)
							p.reset(new OcTree<point, weight_type, size_type>(i, m_maps.position[i], (float)m_maps.repulsion[i], minPos, maxPos));
						else
							p->addNode(i, m_maps.position[i], (float)m_maps.repulsion[i], 0);
						
					}
					#ifdef __LINLOG_PROFILE
					timer_buildOcTree += t_timer.elapsed();
					#endif
				}
				
				void printStatistics(OcTreePtr t) {
				    return;
					weight_type attrSum = m_attrSum;
					
					std::cout << "Number of attracting nodes: " << m_maps.weight.size() << std::endl;
					std::cout << "Overall attraction: " << attrSum << std::endl;
					float_type meanAttrEnergy = 0;

					for(size_type i = 0; i < m_maps.weight.size(); i++) {
						meanAttrEnergy += getAttractionEnergy(i);
					}
					meanAttrEnergy = (float_type) pow(meanAttrEnergy * m_attrExp / attrSum, 1.0f / m_attrExp);
					std::cout << "Weighted mean of attraction energy: " << meanAttrEnergy << std::endl;
					
					float_type repuSum = 0, repuSquareSum = 0;
					for(size_type i = 0; i < m_maps.repulsion.size(); i++) {
						weight_type r = m_maps.repulsion[i]; repuSum += r; repuSquareSum += r*r;
					}
					repuSum = repuSum*repuSum - repuSquareSum;
					std::cout << "Number of repulsing nodes: " << m_maps.repulsion.size() << std::endl;
					std::cout << "Overall repulsion: " << repuSum << std::endl;
					
					float_type meanRepuEnergy = 0;
					for(size_type i = 0; i < m_maps.repulsion.size(); i++) {
						meanRepuEnergy += getRepulsionEnergy(i, t);
					}
					meanRepuEnergy /= m_repuFactor;
					meanRepuEnergy = (m_repuExp == 0)
						? (float_type) exp(-meanRepuEnergy / repuSum)
						: (float_type) pow(-meanRepuEnergy * m_repuExp / repuSum, 1.0 / m_repuExp);
					std::cout << "Weighted mean of repulsion energy: " << meanRepuEnergy << std::endl;
					
					std::cout << "Mean attraction / mean repulsion: " << meanAttrEnergy / meanRepuEnergy << std::endl;
					
					// more to come...
				}
			
			public:
				
				void reset() {
					m_step = 0;
					computeRepuFactor();
					// set the initial repuFactor;
					m_repuFactor = m_finalRepuFactor;
				}
			
				MinimizerBarnesHutHelper(Maps &all_maps, const float_type ae, 
										 const float_type re, const float_type gf) : 
											m_maps(all_maps), m_attrExp(ae), 
											m_repuExp(re), m_gravFactor(gf),
											m_step(0), m_iterations(100), m_noVary(20)
											#ifdef __LINLOG_PROFILE
											// timers
											,timer_total(0), timer_getEnergy(0), timer_getRepulsionEnergy(0),
											timer_getAttractionEnergy(0), timer_getGraviationalEnergy(0),
											timer_getDirection(0), timer_buildOcTree(0), timer_addRepulsionDir(0),
											timer_addGravitationDir(0), timer_addAttractionDir(0),
											printed_timers(false), timer_small_one(0), timer_small_two(0),
											timer_small_three(0), count_getRepulsionEnergy(0)
											#endif
				{
					// build s_repuStrategy
					int i = 0;
					for(float f = 1.0f; f > 0.75f; f -= 0.05f, i++) s_repuStrategy[i] = f;
					for(float f = 0.75f; f < 1.0f; f += 0.05f, i++) s_repuStrategy[i] = f;
					for(float f = 1.0f; f < 1.5f; f+= 0.1f, i++) s_repuStrategy[i] = f;
					for(float f = 1.5; f > 1.0f; f-=0.1f, i++) s_repuStrategy[i] = f;
					/*	s_repuStrategy =
						{ 1.0f, 0.95f, 0.9f, 0.85f, 0.8f, 0.75f, 0.8f, 0.85f, 0.9f, 0.95f, 
						  1.0f, 1.1f, 1.2f, 1.3f, 1.4f, 1.5f, 1.4f, 1.3f, 1.2f, 1.1f };*/
										
					reset();
					
				}
				
				void setIterations(int i) {
					if (i < 0) i = 0;
					m_noVary = i - (m_iterations - m_noVary);
					m_iterations = i;
				}
				
				int getIterations() const {
					return m_iterations;
				}
				
				int getCurrentStep() const {
					return m_step;
				}
				
				void setVariationSteps(int i) {
					if (i < 0) i = 0;
					if (i > m_iterations) i = m_iterations;
					m_noVary = m_iterations - i;
				}
				
				void run() {
					while(m_step < m_iterations) step();
					#ifdef __LINLOG_PROFILE
					printTimers();
					#endif
				}
				
				// returns the current step
				int step(bool force = false) {
					#ifdef __LINLOG_PROFILE
					timer t_timer;
					if (m_step >= m_iterations) printTimers();
					#endif
					if (!force && m_step >= m_iterations) return -1;
					if (m_maps.repulsion.size() <= 1) return -1;

					// yay for OcTrees!
					OcTreePtr ocTree;	
					
					// minimize the energy!!!
					point oldPos, bestDir;
					computeBaryCenter();
					buildOcTree(ocTree);
					
					if (m_step == 0) printStatistics(ocTree);
					
					// except in the last m_noVary iterations, vary the repulsion factor
					m_repuFactor = m_finalRepuFactor;
					if(m_step < (m_iterations-m_noVary)) {
						m_repuFactor *= (float_type)pow((float_type)s_repuStrategy[m_step%s_repuStrategy_length], m_attrExp - m_repuExp);
					}
					
					// move each node
					float_type energySum = 0.0;
					for(size_type i = 0; i < m_maps.weight.size(); i++) {
						const float_type oldEnergy = getEnergy(i, ocTree);
						
						// compute direction of node movement
						bestDir = getDirection(i, ocTree);
						
						// line search: compute length of the move
						point &pos = m_maps.position[i];
						point oldPos(pos);
						float_type bestEnergy = oldEnergy;
						int bestMultiple = 0;
						bestDir /= 32;
						for(	int multiple = 32;
								multiple >= 1 && (bestMultiple==0 || bestMultiple/2==multiple);
								multiple /= 2) 
						{
							pos = oldPos + bestDir * multiple;
							float_type curEnergy = getEnergy(i, ocTree);
							if (curEnergy < bestEnergy) {
								bestEnergy = curEnergy;
								bestMultiple = multiple;
							}
						}
						
						for(	int multiple = 64;
								multiple <= 128 && bestMultiple == multiple/2;
								multiple *= 2)
						{
							pos = oldPos + bestDir * multiple;
							float_type curEnergy = getEnergy(i, ocTree);
							if (curEnergy < bestEnergy) {
								bestEnergy = curEnergy;
								bestMultiple = multiple;
							}
						}
						
						pos = oldPos + bestDir * bestMultiple;
						
						if (bestMultiple > 0) {
							ocTree->moveNode(oldPos, pos, m_maps.repulsion[i]);
						}
							
						energySum += bestEnergy;
					}
					#ifdef __LINLOG_PROFILE
					timer_total += t_timer.elapsed();
					#endif
//					std::cerr << "iteration " << m_step << "    energy " << energySum << "    repulsion " << m_repuFactor << std::endl; //debug
					
					if (m_step == m_iterations) printStatistics(ocTree);
					
					// increment m_step
					return m_step++;					
				}
				
				#ifdef __LINLOG_PROFILE
				
				void printTimers() {
					if (printed_timers) return;
					std::cerr << "******************************" << std::endl;
					std::cerr << "LinLog Profiling" << std::endl;
					std::cerr << "******************************" << std::endl;
					std::cerr << "steps so far: " << m_step << std::endl;
					std::cerr << "Total time: " << timer_total << std::endl;
					std::cerr << std::endl;
					std::cerr << "getEnergy: " << timer_getEnergy << std::endl;
					std::cerr << "getRepulsionEnergy(" << count_getRepulsionEnergy << "): " << timer_getRepulsionEnergy << std::endl;
					std::cerr << "getAttractionEnergy: " << timer_getAttractionEnergy << std::endl;
					std::cerr << "getGravitationEnergy: " << timer_getGraviationalEnergy << std::endl;
					std::cerr << "getDirection: " << timer_getDirection << std::endl;
					std::cerr << "buildOcTree: " << timer_buildOcTree << std::endl;
					std::cerr << "addRepulsionDir: " << timer_addRepulsionDir << std::endl;
					std::cerr << "addAttractionDir: " << timer_addAttractionDir << std::endl;
					std::cerr << "addGravitationDir: " << timer_addGravitationDir << std::endl;
					std::cerr << std::endl;
					std::cerr << "Small timers:" << std::endl;
					std::cerr << "one: " << timer_small_one << std::endl;
					std::cerr << "two: " << timer_small_two << std::endl;
					std::cerr << "three: " << timer_small_three << std::endl;
					std::cerr << std::endl;
					
					printed_timers = true;
				}
				
				#endif
				
			private:
				#ifdef __LINLOG_PROFILE
				// timer values
				double		timer_total, timer_getEnergy, timer_getRepulsionEnergy,
							timer_getAttractionEnergy, timer_getGraviationalEnergy,
							timer_getDirection, timer_buildOcTree, timer_addRepulsionDir,
							timer_addAttractionDir, timer_addGravitationDir;
				double		timer_small_one, timer_small_two, timer_small_three;
				long		count_getRepulsionEnergy;
				bool printed_timers;
				#endif
				
				// member variables
				Maps &m_maps;						// contains all the following maps:
													// vertex to index
													// index to vertex
													// weight
													// repulsion
													// position
													// adjacency

				float_type m_attrExp;				// attraction exponent (1.0f is good for LinLog, 3.0f in Fruchterman-Reingold)
				float_type m_repuExp;				// repulsion exponent (exception: 0.0f = logarithmic repulsion)
				float_type m_repuFactor;			// current factor for repulsion energy. changes through pulsing
				float_type m_gravFactor;			// graviational factor (set to > 0 for disconnected graphs, but still small)
				float_type m_attrSum, m_repuSum, m_finalRepuFactor; // computed constants
				
				point m_baryCenter;					// barycenter of all the verticess
				
				int m_step, m_iterations, m_noVary;
				
				enum { s_repuStrategy_length = 20 };
				float s_repuStrategy[s_repuStrategy_length];
				
		}; // class MinimizerBarnesHutHelper
		
		
		
		// OctTree class for helping with efficient computation
		template <class point, typename weight_type, typename size_type>
		class OcTree {
			typedef typename point::value_type float_type;
			typedef OcTree<point, weight_type, size_type> ThisOcTree;
			// an array of scoped pointers - allows us to dynamically allocate only those children
			// that will be set, while taking advantage of shared_ptr's automatic memory deallocation
			//		having leaf children allocate 8 OctTrees that are not used is very inefficient
			typedef boost::shared_ptr<ThisOcTree> OcTreePtr;
			public:
				OcTree(size_type i, point pos, float_type weight, point minPos, point maxPos) :
					m_pos(pos), m_weight(weight), m_minPos(minPos), m_maxPos(maxPos), m_i(i), m_leaf(true) { recalc(); }
				
				void addNode(size_type i, point pos, float_type weight, int depth) {
					if (depth > 20) {
						std::cerr << "dropping depth > 20 node: " << pos << ", tree: " << m_pos << std::endl;
						return;
					}
					
					if (m_leaf) {
						// add ourselves
						_addNode(m_i, m_pos, (float)m_weight, depth);
						m_leaf = false;
					}
					
					m_pos = (m_pos * m_weight + pos * weight) / (weight + m_weight);
					m_weight += weight;
					
					_addNode(i, pos, weight, depth);
				}
				
				void moveNode(point oldPos, point newPos, weight_type weight) {
					m_pos += (newPos - oldPos) * (weight/m_weight);
					
					int ind = 0;
					for (int i = 0; i < 3; i++) {
						if (oldPos[i] > (m_minPos[i]+m_maxPos[i]) / 2) {
							ind += (1 << i);
						}
					}
					
					if (m_children[ind]) {
						m_children[ind]->moveNode(oldPos, newPos, weight);
					}
				}
				
				float_type width() const {
					return m_width;
				}
				
				size_type index() const { return m_i; }
				
				bool isLeaf() const { return m_leaf; }
				
				weight_type &weight() { return m_weight; }
				
				OcTreePtr child(int i) { return m_children[i]; }
				
				point &position() { return m_pos; }
				
			private:
				void _addNode(size_type index, point pos, float_type weight, int depth) {
					int ind = 0;
					for (int i = 0; i < 3; i++) {
						if (pos[i] > (m_minPos[i]+m_maxPos[i]) / 2) {
							ind += (1 << i);
						}
					}
					
					if (m_children[ind]) {
						m_children[ind]->addNode(index, pos, weight, depth+1);
					} else {
						point minPos, maxPos;
						for(int i = 0; i < 3; i++) {
							if ((ind & (1<<i)) == 0) {
								minPos[i] = m_minPos[i];
								maxPos[i] = (m_minPos[i] + m_maxPos[i]) / 2;
							} else {
								minPos[i] = (m_minPos[i] + m_maxPos[i]) / 2;
								maxPos[i] = m_maxPos[i];
							}
						}
						
						// set the smart pointer's target
						m_children[ind].reset(new ThisOcTree(index, pos, weight, minPos, maxPos));
					}
				}
				
				void recalc() {
					// width
					m_width = 0;
					for(int i = 0; i < 3; i++) {
						if (m_maxPos[i] - m_minPos[i] > m_width)
							m_width = m_maxPos[i] - m_minPos[i];
					}
				}
				
				// member variables
				point m_pos;
				weight_type m_weight;
				point m_minPos, m_maxPos;
				size_type m_i;
				bool m_leaf;
				OcTreePtr m_children[8];
				
				float_type m_width;
		}; // class OcTree
	} // namespace detail
	
	template <class Graph, class WeightPropertyMap>
	struct linlog_traits {
	    typedef linlog_traits<Graph, WeightPropertyMap>                this_type;
	    typedef typename boost::graph_traits<Graph>::vertex_descriptor 			vertex;
		typedef typename boost::graph_traits<Graph>::edge_descriptor 			edge;
		typedef typename boost::graph_traits<Graph>::vertices_size_type 		size_type;
		typedef typename boost::property_traits<WeightPropertyMap>::value_type 	weight_type;

	    typedef point3d<float> 													point;		
		
		typedef maps::unordered<vertex, size_type>                              vertex_index_map_type;
	    typedef std::vector<vertex> 											index_vertex_map_type;
		typedef std::vector<weight_type> 										rep_map_type;
		typedef std::vector<point> 												point_map_type;
		typedef std::vector<std::vector<size_type> > 							adj_map_type;
		typedef std::vector<std::vector<weight_type> > 							weight_map_type;
		
		// for use when directly accessing the position map
		typedef maps::unordered<vertex, point>                                  vertex_position_map;
		
		// convenient holder for everything
		struct all_maps {
			typedef point								point_type;
			typedef typename this_type::weight_type		weight_type;
			typedef typename this_type::size_type		size_type;
			
			adj_map_type 			adjacency;
			weight_map_type			weight;
			vertex_index_map_type	vertex_index;
			index_vertex_map_type	index_vertex;
			point_map_type			position;
			rep_map_type			repulsion;
			void clear() {
				adjacency.clear();
				weight.clear();
				vertex_index.clear();
				index_vertex.clear();
				position.clear();
				repulsion.clear();
			}
		};
	}; // struct linlog_traits
	
	template <class Graph, class WeightPropertyMap>
	class LinLogHelper {
		typedef LinLogHelper<Graph, WeightPropertyMap>		this_type;
		typedef linlog_traits<Graph, WeightPropertyMap>     traits;
		
		typedef typename traits::vertex 			        vertex;
		typedef typename traits::edge 			            edge;
		typedef typename traits::size_type          		size_type;
		typedef typename traits::point 						point;
		typedef typename traits::weight_type 	            weight_type;
		typedef typename traits::vertex_index_map_type      vertex_index_map_type;

		typedef typename traits::index_vertex_map_type      index_vertex_map_type;
		typedef typename traits::rep_map_type				rep_map_type;
		typedef typename traits::point_map_type				point_map_type;
		typedef typename traits::adj_map_type   			adj_map_type;
		typedef typename traits::weight_map_type			weight_map_type;
		
		typedef typename traits::all_maps                   all_maps;
		
		public:
			typedef vertex_index_map_type VertexIndexMap;
			typedef index_vertex_map_type IndexVertexMap;
			typedef rep_map_type RepulsionMap;
			typedef point_map_type PositionMap;
			typedef adj_map_type AdjacencyMap;
			typedef weight_map_type WeightMap;
			typedef point Point;
			typedef all_maps AllMaps;
			typedef detail::MinimizerBarnesHutHelper<AllMaps> Minimizer;
			// change this to shared_ptr<Minimizer>
			typedef Minimizer* MinimizerPtr;

			static void populate_all_maps(Graph &g, WeightPropertyMap w, AllMaps &a, bool threeD = false) {
				populate_index_maps(g, a.vertex_index, a.index_vertex);
				randomize_position_map(g, a.position, threeD);
				populate_adjacency_weight_map(g, a.vertex_index, w, a.adjacency, a.weight);
				populate_repulsion_map(g, a.weight, a.repulsion);
			}

			static void populate_index_maps(Graph &g, VertexIndexMap &vi, IndexVertexMap &iv) {
				iv.clear(); vi.clear();
				size_type i = 0;
				// below line gives compile errors, so we let it do it on its own
//				vi.rehash(num_vertices(g)/vi.max_load_factor()); // speed up inserts
				iv.resize(num_vertices(g)); // ditto
				BGL_FORALL_VERTICES_T(u, g, Graph) { vi[u] = i; iv[i++] = u; }
			}

			static void randomize_position_map(Graph &g, PositionMap &m, bool threeD = false) {
				m.clear();
				m.resize(num_vertices(g)); // speed
				minstd_rand rand;
				rand.seed((boost::int32_t)time(0));
				
				for(size_type i = 0; i < num_vertices(g); i++) {
					point p;
					p.set_rand(rand, 0.5);
					if (!threeD) p.z() = 0;
					
					m[i] = p;
				}
			}
						
			static void populate_adjacency_weight_map(Graph &g, VertexIndexMap &v_id, WeightPropertyMap w, AdjacencyMap &a, WeightMap &wm) {
				a.clear(); wm.clear();
				a.resize(num_vertices(g));
				wm.resize(num_vertices(g));
				
				typename boost::graph_traits<Graph>::vertex_iterator vi, vi_end;
				for(boost::tie(vi, vi_end) = vertices(g); vi != vi_end; ++vi) {
					vertex u = *vi;
					
					typename boost::graph_traits<Graph>::out_edge_iterator ei, ei_end;
					for(boost::tie(ei, ei_end) = out_edges(u, g); ei != ei_end; ++ei) {
						vertex v = target(*ei, g);
						a[v_id[u]].push_back(v_id[v]);
						wm[v_id[u]].push_back(get(w, *ei));
					}
				}
			}
			
			static void populate_repulsion_map(Graph &g, const WeightMap &w, RepulsionMap &r) {
				r.clear();
				r.resize(num_vertices(g));
				for(size_type i = 0; i < num_vertices(g); i++) {
					weight_type total = 0;
					for(size_type t = 0; t < w[i].size(); t++) {
						total += w[i][t];
					}
					r[i] = total;
				}
			}
			
	}; // class LinLogHelper
	
	// a helper function to perform basic linlog layout
	template <class Graph, class WeightMap, typename Counter>
	inline void
	linlog_layout(Graph &g, WeightMap w, int iterations,
	    typename linlog_traits<Graph, WeightMap>::vertex_position_map &positions, Counter &cRef, const bool &stop_if_false,
	    bool threeD = false, bool positions_on_step = false)
	{
	    typedef linlog_traits<Graph, WeightMap> traits;
	    typedef LinLogHelper<Graph, WeightMap> helper;
	    typename traits::all_maps maps;
	    helper::populate_all_maps(g, w, maps, threeD);
	    
	    // create a minimizer
	    detail::MinimizerBarnesHutHelper<typename traits::all_maps>
	        minimizer(maps, 1.0f, 0.0f, 0.01f);
	    
	    minimizer.setIterations(iterations);
	    while(minimizer.step() != -1) {
	        if (positions_on_step) {
	            positions.clear();
	            BGL_FORALL_VERTICES_T(u, g, Graph) {
        	        positions[u] = maps.position[maps.vertex_index[u]];
        	    }
	        }
	        if (!stop_if_false) return;
	        ++cRef;
	        if (!stop_if_false) return;
	    }
	    
	    BGL_FORALL_VERTICES_T(u, g, Graph) {
	        positions[u] = maps.position[maps.vertex_index[u]];
	    }
	}

	template <class Graph, class WeightMap>
	inline void
	linlog_layout(Graph &g, WeightMap w, int iterations,
	    typename linlog_traits<Graph, WeightMap>::vertex_position_map &positions, bool threeD = false) 
	{
	    int counter = 0; // dummy counter
	    bool dummy_bool = true;
	    linlog_layout(g, w, iterations, positions, counter, dummy_bool, threeD);
	}
	
	// aliases
	template <class Graph, class WeightMap>
	inline void
	linlog_layout_2d(Graph &g, WeightMap w, int iterations,
	    typename linlog_traits<Graph, WeightMap>::vertex_position_map &positions) 
	{
        linlog_layout(g, w, iterations, positions, false);
    }
    template <class Graph, class WeightMap, class Counter>
    inline void
    linlog_layout_2d(Graph &g, WeightMap w, int iterations,
	    typename linlog_traits<Graph, WeightMap>::vertex_position_map &positions, Counter &cRef, const bool &stop_if_false= true, bool positions_on_step = false)
	{
        linlog_layout(g, w, iterations, positions, cRef, stop_if_false, false, positions_on_step);
    }
    template <class Graph, class WeightMap>
	inline void
	linlog_layout_3d(Graph &g, WeightMap w, int iterations,
	    typename linlog_traits<Graph, WeightMap>::vertex_position_map &positions) 
	{
        linlog_layout(g, w, iterations, positions, true);
    }
    template <class Graph, class WeightMap, class Counter>
    inline void
    linlog_layout_3d(Graph &g, WeightMap w, int iterations,
	    typename linlog_traits<Graph, WeightMap>::vertex_position_map &positions, Counter &cRef, const bool &stop_if_false = true, bool positions_on_step = false)
	{
        linlog_layout(g, w, iterations, positions, cRef, stop_if_false, true, positions_on_step);
    }
	
} } // namespace semantic::analysis



#endif
