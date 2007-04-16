#include <semantic/semantic.hpp>
#include <semantic/storage/none.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/iteration_macros.hpp>
#include <semantic/indexing.hpp>
#include <semantic/subgraph.hpp>
#include <semantic/subgraph/none.hpp>
#include <semantic/utility.hpp>
#include <semantic/pruning.hpp>
#include <semantic/analysis/linlog.hpp>

#include <boost/random/linear_congruential.hpp>

#include <semantic/io/linlog.hpp>

#include <semantic/weighting/lg.hpp>
#include <semantic/weighting/idf.hpp>
#include <semantic/weighting/tf.hpp>

#include <fstream>

#include <iostream>
#include <map>

#include <time.h>

using namespace semantic;
using std::string;

typedef SESubgraph<NoStoragePolicy, NoSubgraphPolicy, LGWeighting<TFWeighting, IDFWeighting, double> > Graph;
typedef weighting_traits<Graph> wtraits;

int main(int argc, char *argv[]) {
/*	typedef point3d<float> pt;
	pt P(1,0.5,0.25);
	assert(P * 3 == pt(3, 1.5, 0.75));
	P *= 3;
	assert(P == pt(3, 1.5, 0.75));
	P += pt(0,0,0.25);
	assert(P == pt(3, 1.5, 1.0));
	P -= pt(1,1,2);
	assert(P == pt(2, 0.5, -1.0));
	assert(P.dist(pt(2, 0, -1.0)) == 0.5);
	exit(0);
*/	
	if (argc < 2) {
		std::cerr << "Usage: " << argv[0] << " <input file>" << std::endl;
		exit(1);
	}
	
	Graph g("test");
	
//	g.set_mirror_changes_to_storage(true);
	std::ifstream in(argv[1]);
	
	std::map<std::string, se_graph_traits<Graph>::vertex_descriptor> vertex_names;
	wtraits::edge_weight_map weights;
	read_linlog_graph(g, boost::make_assoc_property_map(weights), in, vertex_names);
	
//	g.set_mirror_changes_to_storage(false); avoids re-weighting
		
	std::cout << num_vertices(g) << std::endl;
	std::cout << num_edges(g) << std::endl;
		
	// try some linlog stuff
	boost::minstd_rand r;
	r.seed(time(0));

	std::map<Graph::vertex_descriptor, double> repWeights;
	std::map<Graph::vertex_descriptor, point3d<float> > points;
	
/*	std::ifstream inp(argv[2]);
	std::string name;
	float x, y;
	while(!inp.eof()) {
		inp >> name >> x >> y;
		points[vertex_names[name]] = point3d<float>(x, y, 0);
	}*/
	
	// compute the repWeights and a random point
	se_graph_traits<Graph>::vertex_iterator vi, vi_end;
	for(boost::tie(vi, vi_end) = vertices(g); vi != vi_end; ++vi) {		
		point3d<float> p;
		p.set_rand(r);
		p.z() = 0;
		p /= 2; // -0.5 - 0.5
		points[*vi] = p;
		
		// ensure symmetry of weights
		se_graph_traits<Graph>::out_edge_iterator ei, ei_end;
		std::set<Graph::edge_descriptor> seen;
		for(boost::tie(ei, ei_end) = out_edges(*vi, g); ei != ei_end; ++ei) {
			if (seen.count(*ei)) continue;
			if (edge(target(*ei, g), source(*ei, g), g).second) {
				seen.insert(*ei); seen.insert(edge(target(*ei, g), source(*ei, g), g).first);
				weights[*ei] += weights[edge(target(*ei, g), source(*ei, g), g).first];
				weights[*ei] /= 2;
				weights[edge(target(*ei, g), source(*ei, g), g).first] = double(weights[*ei]);
			}
		}
	}
	
	// now ensure bidirectionality
	ensure_bidirectionality<boost::associative_property_map<wtraits::edge_weight_map> > prune(boost::make_assoc_property_map(weights));
	g.apply_to_all_edges(prune);
	
	
	for(boost::tie(vi, vi_end) = vertices(g); vi != vi_end; ++vi) {
		double t = 0;
		BGL_FORALL_OUTEDGES(*vi, e, g, Graph) t += weights[e];
		repWeights[*vi] = t;
//		std::cerr << *vi << " repuWeight = " << t << std::endl;
	}
	
	typedef analysis::LinLogHelper<Graph, boost::associative_property_map<wtraits::edge_weight_map> > LinLog;
	LinLog::AllMaps maps;
	LinLog::populate_all_maps(g, boost::make_assoc_property_map(weights), maps);
	
	analysis::detail::MinimizerBarnesHutHelper<LinLog::AllMaps> helper(maps, 1.0f, 0.0f, 0.01f);
	helper.setIterations(100);
	helper.run();
	
/*	analysis::detail::MinimizerBarnesHutHelper<
		Graph,
		boost::associative_property_map<wtraits::edge_weight_map>,
		std::map<Graph::vertex_descriptor, point3d<float> >,
		boost::associative_property_map<std::map<Graph::vertex_descriptor, double> >
		> helper(g, boost::associative_property_map<wtraits::edge_weight_map>(weights), points, boost::associative_property_map<std::map<Graph::vertex_descriptor, double> >(repWeights), 1.0f, 0.0f, 0.01f);
		
	helper.runIterations(50);*/
	
	
/*	for(boost::tie(vi, vi_end) = vertices(g); vi != vi_end; ++vi) {
		std::cout << g[*vi].content << " is at " << points[*vi] << std::endl;
	}*/
}