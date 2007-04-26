#ifndef SE_GRAPH_H
#define SE_GRAPH_H


#include <semantic/semantic.hpp>
#include <semantic/search.hpp>

// version information
#include <semantic/version.hpp> // gives us SEMANTIC_VERSION_STRING and SEMANTIC_PACKAGE_NAME

// subgraph -- searching behavior
#include <semantic/subgraph/pruning_random_walk.hpp>

// weighting -- applies to search & ranking behavior
#include <semantic/weighting/tf.hpp>
#include <semantic/weighting/lg.hpp>
#include <semantic/weighting/idf.hpp>

// ranking
#include <semantic/ranking/spreading_activation.hpp>


// semantic clustering
#include <semantic/analysis/agglomerate_clustering/dendrogram.hpp>
#include <semantic/analysis/agglomerate_clustering/mst.hpp>
#include <semantic/analysis/silhouette.hpp>
#include <semantic/analysis/linlog.hpp>



#if !SEMANTIC_HAVE_MYSQL
	#undef MYSQL_STORAGE
#endif
#if !SEMANTIC_HAVE_SQLITE3
	#undef SQLITE_STORAGE
#endif

#ifdef SQLITE_STORAGE
	#include <semantic/storage/sqlite3.hpp>
	typedef SQLite3StoragePolicy				TheStoragePolicy;
#elif defined MYSQL_STORAGE
	#include <semantic/storage/mysql5.hpp>
	typedef MySQL5StoragePolicy					TheStoragePolicy;
#else
	#error "No Storage Engine Defined!"
	typedef NoStoragePolicy						TheStoragePolicy;
#endif

typedef LGWeighting<TFWeighting, IDFWeighting, double>	WeightingPolicy;
typedef SESubgraph<
	TheStoragePolicy,
	PruningRandomWalkSubgraph,
	WeightingPolicy>									Graph;

typedef StorageInfoQuery<TheStoragePolicy>			StorageInfo;
typedef se_graph_traits<Graph>							GraphTraits;
typedef weighting_traits<TheStoragePolicy,
						WeightingPolicy>				WeightingTraits;

// some typedefs for clustering
typedef boost::associative_property_map<WeightingTraits::edge_weight_map> WeightMapType;
typedef semantic::analysis::linlog_traits<Graph, WeightMapType> LinLog;
typedef semantic::analysis::LinLogHelper<Graph, WeightMapType> LinLogHelper;
typedef LinLog::vertex_position_map PositionMap;


	enum searchType {
		SemanticSearch = 0,
		KeywordSearch = 1,
		SimilarSearch = 2
	};
	



#endif
