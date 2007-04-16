#include <semantic/semantic.hpp>
#include <semantic/storage/sqlite3.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/iteration_macros.hpp>
#include <semantic/subgraph.hpp>
#include <semantic/subgraph/none.hpp>
#include <semantic/utility.hpp>

#include <iostream>
#include <map>
#include <mysql.h>

using namespace semantic;

#include <fstream>
#include <semantic/weighting/none.hpp>
#include <semantic/analysis/utility.hpp>

void mysql_test() {
}




int main() {
	typedef SESubgraph<
		SQLite3StoragePolicy, 
		NoSubgraphPolicy,
		NoWeighting
		> Graph;
	typedef se_graph_traits<Graph> traits;
	
	Graph g("columnists");
	g.set_file("/Users/gabe/Code/Indexes/columnists/columnists.db");
	
	g.populate_full_graph();
	
	std::cout << "v: " << num_vertices(g) << " e: " << num_edges(g) << std::endl;
	
	// connect to MySQL db and get the data
	MYSQL *con;
	con = mysql_init(NULL);

	mysql_real_connect(
			con,
			"localhost",
			"root",
			"",
			"supreme_court",
			3306,
			NULL,
			0);
	
	// execute the query
	mysql_query(con, "SELECT s.source_id, s.title, f.item_id, f.title, s.parser, s.encoding FROM feed_items f INNER JOIN sources s ON f.source_id = s.source_id WHERE followed = 1 and pubDate > '11-9-2005' and s.format = 'columnist'");
	
	MYSQL_RES *r = mysql_use_result(con);
	MYSQL_ROW row;
	int cnt = 0;
	while(row = mysql_fetch_row(r)) {
		std::string author, id, title, body;
		author = std::string(row[1]);
		id = std::string(row[2]);
		title = std::string(row[3]);
		
		// read in the file
		std::ifstream in(("/Users/gabe/Code/Indexes/columnists/data/all/" + id).c_str());
		std::string line;
		while(std::getline(in, line)) {
			body = body + line;
		}
		
		traits::vertex_id_type internal_id = g.fetch_vertex_id_by_content_and_type(id, node_type_major_doc);
		traits::vertex_descriptor u = g.vertex_by_id(internal_id);
		g.set_vertex_meta_value(u, "title", title);
		g.set_vertex_meta_value(u, "author", author);
		g.set_vertex_meta_value(u, "body", body);
		std::cout << ++cnt << std::endl;
	}
}
