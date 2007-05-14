/*
stores the graph in an SQLite 3 database file

this follows the storage policy description found in docs/storage/policy.txt
*/

#if SEMANTIC_HAVE_SQLITE3

#include <map>
#include <semantic/properties.hpp>
#include <sqlite3.h>
#include <semantic/exception.hpp>
#include <semantic/storage/base.hpp>
#include <sstream>
#include <iostream>

#include <boost/algorithm/string/split.hpp>

#ifndef __SEMANTIC_STORAGE_SQLITE3_HPP__
#define __SEMANTIC_STORAGE_SQLITE3_HPP__

#include <semantic/semantic.hpp>

namespace semantic {
	
	struct SQLite3StoragePolicy;
	
	// our exception
	struct SQLiteException : public std::exception {
		SQLiteException(sqlite3 *&con) : number(sqlite3_errcode(con)), msg(std::string(sqlite3_errmsg(con))) {}
		SQLiteException(int n, std::string m) : number(n), msg(m) {}
		SQLiteException(std::string msg) : number(-1), msg(msg) {}
		~SQLiteException() throw() {}
		const char * what() const throw() {
		 	std::string str = "SQLite Error";
			if (number != -1) str += " " + to_string(number);
			str += ": " + msg;
			return str.c_str();
		}
		
		int number;
		std::string msg;
	};
	
	// our custom vertex properties struct - includes the internal DB id of the vertex
	// and a flag saying if it's already there or not
	struct sqlite_vertex_properties : vertex_properties {
		sqlite_vertex_properties() : vertex_properties(), in_db(false) {}
		unsigned long id;
		bool in_db, dirty;
	};
	
	// our custom edge properties - includes an in_db flag and a "dirty" flag
	struct sqlite_edge_properties : edge_properties {
		sqlite_edge_properties() : edge_properties(), dirty(false) {}
		bool dirty;
	};
	
	template <> struct se_storage_traits<SQLite3StoragePolicy> {
		// this is the id type in the SQLite database (NUMERIC)
		typedef unsigned long vertex_id_type; 
		typedef sqlite_vertex_properties vertex_properties_type; // use our home-grown struct
		typedef sqlite_edge_properties edge_properties_type;	  // ditto
	};
	
	template <class SEBase>
	class StoragePolicy<SQLite3StoragePolicy, SEBase> : public SEBase, public StoragePolicyBase<SQLite3StoragePolicy> {
		typedef se_graph_traits<SQLite3StoragePolicy> traits;
		typedef typename traits::base_graph_type base_graph_type;
		typedef typename traits::vertex_descriptor Vertex;
		typedef typename traits::edge_descriptor Edge;

		typedef se_storage_traits<SQLite3StoragePolicy> storage_traits;
		typedef typename storage_traits::vertex_properties_type vertex_properties;
		typedef typename storage_traits::edge_properties_type edge_properties;
		typedef typename storage_traits::vertex_id_type id_type;
		
		public:
			typedef SEBase base_type;
			
			// constructor(s)
			StoragePolicy() : m_clear_all(false), m_con(NULL), m_connected(false), m_collection_id((std::numeric_limits<id_type>::max)()), mirror_flag(false) {  }
			~StoragePolicy() { close(); }
	
			// methods having to do directly with this storage policy implementation
			void set_mirror_changes_to_storage(bool b) { mirror_flag = b; }
			bool get_mirror_changes_to_storage() { return mirror_flag; }
			
			void commit_changes_to_storage() {
				count_cache.clear();
				synchronize();
			}
			
			// methods having to do with structure alteration to the graph
			std::pair<bool, Vertex>
			will_add_vertex(const vertex_properties &vp) {
				// check if it's in our cache
				if (vp.in_db && m_id_vertex_cache.count(vp.id)) {
					Vertex v = m_id_vertex_cache[vp.id];
					// if we are allowing updates, change the properties and flag them as dirty
					if (get_mirror_changes_to_storage()) {
						(*this)[v] = vp;
						(*this)[v].dirty = true;
					}
					
					return std::make_pair(false, v);
				}
				
				return std::make_pair(true, Vertex());
			}
			
			void did_add_vertex(Vertex v, const vertex_properties &vp) {
				// check if we want to cache this...
				if (vp.in_db) {
					if (!m_id_vertex_cache.count(vp.id)) {
						m_id_vertex_cache[vp.id] = v;
					}
				}
				
				// mark as dirty, if we're mutable
				if (get_mirror_changes_to_storage()) (*this)[v].dirty = true;
			}

			void did_add_edge(Edge e, bool, Vertex, Vertex, const edge_properties &) {
				// if mutable, flag as dirty
				if (get_mirror_changes_to_storage()) (*this)[e].dirty = true;
			}

			void did_remove_vertex(const vertex_properties &vp) {
			    if (vp.in_db && get_mirror_changes_to_storage()) {
			        m_to_remove.push_back(vp.id);
			        m_id_vertex_cache.erase(vp.id);
			    }
			}
			
			void did_remove_edge(Vertex u, Vertex v) {
			    if (get_mirror_changes_to_storage()) {
        		    id_type from, to;
        		    from = (*this)[u].id;
        		    to = (*this)[v].id;
		    
        		    m_to_remove_edges.push_back(std::make_pair(from, to));
    		    }
			}
			
			void did_clear() { 
			    m_id_vertex_cache.clear(); 
			    count_cache.clear(); 
			    if (get_mirror_changes_to_storage()) m_clear_all = true; 
			}
			
			void did_clear_vertex(Vertex u) {
			    if ((*this)[u].in_db && get_mirror_changes_to_storage()) {
			        m_to_clear.push_back((*this)[u].id);
			    }
			}
			
			void mark_as_dirty(Vertex u) {
			    (*this)[u].dirty = true;
			}
			
			void mark_as_dirty(Edge e) {
			    (*this)[e].dirty = true;
			}
			
			// methods for fetching graph contents
#ifdef WIN32
			Vertex vertex_by_id(id_type id) throw (...) {
#else
			Vertex vertex_by_id(id_type id) throw (VertexNotFoundException<id_type>) {
#endif
				if (!m_id_vertex_cache.count(id)) {
			        // fetch the vertex, if possible.
			        vertex_properties vp;
			        if (!fetch_vertex_properties(id, vp)) {
			            throw VertexNotFoundException<id_type>(id);
			        }
			        if (will_add_vertex(vp).first) {
			            // ok actually add it
			            Vertex u = boost::add_vertex(vp, *this);
			            did_add_vertex(u, vp);
			        }
			    }
				if (m_id_vertex_cache.count(id)) return m_id_vertex_cache[id];
				throw VertexNotFoundException<id_type>(id);
			}
			
			template <class Iterator, class OutIterator>
#ifdef WIN32
			void vertices_by_id(Iterator i, Iterator i_end, OutIterator out) throw (...) {
#else
			void vertices_by_id(Iterator i, Iterator i_end, OutIterator out) throw (VertexNotFoundException<id_type>) {
#endif
				Iterator save = i;
			    std::set<id_type> to_fetch;
				while(i != i_end) {
					if (!m_id_vertex_cache.count(*i)) to_fetch.insert(*i);
					else *out = std::make_pair(*i, m_id_vertex_cache[*i]);
					++i;
				}
				
				if (!to_fetch.empty()) {
				    std::vector<vertex_properties> vps;
				    fetch_vertex_properties(to_fetch.begin(), to_fetch.end(), back_inserter(vps));
				    for(unsigned int i = 0; i < vps.size(); i++) {
				        if (will_add_vertex(vps[i]).first) {
				            Vertex u = boost::add_vertex(vps[i], *this);
				            did_add_vertex(u, vps[i]);
				            *out = std::make_pair(get_vertex_id(vps[i]), u);
				        }
				        to_fetch.erase(get_vertex_id(vps[i]));
				    }
				}
				
				// in case we didn't get all of them back...
				if (!to_fetch.empty()) {
				    throw VertexNotFoundException<id_type>(*to_fetch.begin());
				}
			}

#ifdef WIN32
			id_type get_vertex_id(const Vertex u) const throw(...) {
#else
			id_type get_vertex_id(const Vertex u) const throw(IdNotFoundException) {
#endif
				if ((*this)[u].in_db) return (*this)[u].id;
				throw IdNotFoundException();
			}

#ifdef WIN32
			id_type get_vertex_id(const vertex_properties &p) const throw(...) {
#else
			id_type get_vertex_id(const vertex_properties &p) const throw(IdNotFoundException) {
#endif
				if (p.in_db) return p.id;
				throw IdNotFoundException();
			}

#ifdef WIN32
			id_type fetch_vertex_id_by_content_and_type(std::string content, int type) throw(...) {
#else
			id_type fetch_vertex_id_by_content_and_type(std::string content, int type) throw(VertexContentNotFoundException) {
#endif
				id_type id;
				std::string q = "select node.id from node, content where content.id = node.fk_content and node.type_major = " + to_string(type) + " and content.content = '" + escape(content) + "' and fk_collection=" + to_string(get_collection_id());
				
				query(q);
				
				if (rows() < 1) throw VertexContentNotFoundException(content);
				id = strtoul(field(0,0), NULL, 10);
				free();
				
				return id;
			}
			
			traits::vertices_size_type get_vertex_count_of_type(int node_type) {
				if (count_cache.count(node_type)) return count_cache[node_type];
				
				id_type cid = get_collection_id();
				std::string q = "select type_major, count from node_count where fk_collection = " + to_string(cid);
				query(q);
				
				for(int row = 0; row < rows(); row++) {
					int type = atoi(field(row,0));
					traits::vertices_size_type cnt = strtoul(field(row, 1), NULL, 10);
					count_cache.insert(std::make_pair(type, cnt));
				}
				
				free();
				
				return count_cache[node_type];
			}
			
			// will populate p with the properties of id or return false on error
			bool fetch_vertex_properties(id_type id, vertex_properties &p) {
				id_type *pt = &id;
				return fetch_vertex_properties(pt, pt+1, &p); // i love templates and iterators!
			}
			
			// will populate out with vertex_properties instances for each id represented
			// in i -> i_end
			template <class IdIterator, class InputIterator>
			bool fetch_vertex_properties(IdIterator i, IdIterator i_end, InputIterator out) {
				if (i == i_end) return false; // nothing to fetch!
				// begin building the query
				std::stringstream q;
				
				q << "select n.id, n.type_major, n.type_minor, c.content from node n" <<
						" left join content c on n.fk_content = c.id" <<
						" where n.id in (" << join(i, i_end, ",") << ")";
								
				// run the query
				query(q.str());
				
				// and start generating the properties
				for(int row = 0; row < rows(); row++) {
					vertex_properties p;
					p.id = strtoul(field(row, 0), NULL, 10);
					p.type_major = atoi(field(row, 1));
					p.type_minor = atoi(field(row, 2));
					p.content = std::string(field(row, 3));
					p.in_db = true;
					
					*out = p;
				}
				
				free();
				return true;
			}
			
			// will populate out with std::pair<edge_properties, vertex_properties> for each
			// edge and adjacent vertex
			//		(an inserter(se_graph_traits::neighbor_list) can be used here)
			template <class InputIterator>
			bool fetch_vertex_neighbors(id_type id, InputIterator out) {
				typename traits::mapped_neighbor_list m;
				id_type *p = &id;
				bool good = fetch_vertex_neighbors(p, p+1, m);
				if (!good || !m.count(id)) return false;
				
				// copy the entries
				copy(m[id].begin(), m[id].end(), out);
				return true;
			}
			
			// will populate the map passed with id -> neighbor list for each id in i -> i_end
			// - the map must be a map of id_type to some Container
			// - the neighbor list is a std::pair<edge_properties, vertex_properties>
			//		(se_graph_traits::mapped_neighbor_list can be used)
			// - this method will create an inserter() for each neighbor list (value_type of the Map)
			template <class IdIterator, class Map>
			bool fetch_vertex_neighbors(IdIterator i, IdIterator i_end, Map &m) {
				if (i == i_end) return false;
				typedef typename Map::value_type::second_type container_type;
				typedef typename container_type::value_type value_type;
				BOOST_STATIC_ASSERT((boost::is_same<typename Map::key_type, id_type>::value));
				
				std::stringstream q;
				 
				q <<	"select q.fk_node_from, q.fk_node_to, q.strength, q.degree_from, q.degree_to," <<
						" q.type_major, q.type_minor, c.content" <<
						" from edge_query q" <<
						" inner join node n on n.id = q.fk_node_to" <<
						" left join content c on n.fk_content = c.id" <<
						" where fk_node_from in (" << join(i, i_end, ",") << ")";
				
				// query!
				query(q.str());
				
//				std::cout << "got " << rows() << " rows back..." << std::endl; // debug
				
				for(int row = 0; row < rows(); row++) {
					edge_properties ep;
					vertex_properties vp;
					
					id_type n_from = strtoul(field(row, 0), NULL, 10);
					vp.id = strtoul(field(row, 1), NULL, 10);
					vp.in_db = true;
					ep.strength = atoi(field(row, 2));
					ep.from_degree = atoi(field(row, 3));
					ep.to_degree = atoi(field(row, 4));
					vp.type_major = atoi(field(row, 5));
					vp.type_minor = atoi(field(row, 6));
					vp.content = std::string(field(row, 7));
					
					inserter(m[n_from], m[n_from].end()) = value_type(ep, vp); 
				}
				
				free();
				
				return true;
			}
			
			// populates a mapped_neighbor_list with all the vertices and edges in our graph
			bool populate_full_graph(bool include_edges = true) {
				// we must fetch & populate the full graph from the db
				
				query("select n.id, n.type_major, n.type_minor, c.content from node n left join content c on n.fk_content = c.id where n.fk_collection = " + to_string(get_collection_id()));
				// process
				for(int i = 0; i < rows(); i++) {
					vertex_properties p;
					p.id = strtoul(field(i, 0), NULL, 10);
					p.type_major = atoi(field(i, 1));
					p.type_minor = atoi(field(i, 2));
					p.content = std::string(field(i,3));
					p.in_db = true;
					
					// add this vertex to the graph
					m_id_vertex_cache[p.id] = add_vertex(p, *this);
				}
				free();
				
				if (!include_edges) return true; // we're done
				
				// now do the edges
				query("select q.fk_node_from, q.fk_node_to, q.strength, q.degree_from, q.degree_to from edge_query q inner join node n on n.id = q.fk_node_from where n.fk_collection = " + to_string(get_collection_id()));
				// process
				for(int i = 0; i < rows(); i++) {
					edge_properties p;
					id_type n_from, n_to;
					Vertex u, v;
					n_from = strtoul(field(i, 0), NULL, 10);
					n_to = strtoul(field(i, 1), NULL, 10);
					p.strength = atoi(field(i, 2));
					p.from_degree = atoi(field(i, 3));
					p.to_degree = atoi(field(i, 4));
					
					try {
						// try to do this, ignore if we can't find the vertex
						u = vertex_by_id(n_from);
						v = vertex_by_id(n_to);
						add_edge(u, v, p, *this); // add the edge
					} catch (VertexNotFoundException<id_type> e) {
//						std::cout << "exception: " << n_from << " " << n_to << " " << e.what() << std::endl;
						continue;
					}
				}
				free();
				
				// done.
				return true;
			}
			
			// returns a list of collections available in this index
			template <class Inserter>
			void get_collections_list(Inserter i) {
				query("SELECT name FROM collection");
				for(int k = 0; k < rows(); k++) {
					*i = std::string(field(k, 0));
				}
			}
			
			// renames a collection in the index from one thing to another,
			// doesn't check for errors
			void rename_collection(std::string oldname, std::string newname) {
			    query("update collection set name = '" + escape(newname) + "' where name = '" + escape(oldname) + "'");
			}
			
			// removes the named collection
			void remove_collection(std::string name) {
			    query("BEGIN TRANSACTION");
			    query("delete from collection where name = '" + escape(name) + "'");
			    query("COMMIT TRANSACTION");
			}
			
			// collection meta data functions
			void set_meta_value(const std::string key, const std::string value) {
				query("insert or replace into collection_meta (fk_collection, key, value) values (" 
					+ to_string(get_collection_id()) + ", '"
					+ escape(key) + "', '" + escape(value) + "')");
			}
			
			std::string get_meta_value(const std::string key, const std::string def = "") {
				query("select value from collection_meta where fk_collection = " + to_string(get_collection_id()) + " and key='" + escape(key) + "'");
				std::string value;
				if (rows()>0) value = field(0,0);
				else value = def;
				free();
				
				return value;				
			}
						
			// vertex meta data functions
			void set_vertex_meta_value(const Vertex u, const std::string key, const std::string value) {
				query("insert or replace into node_meta (fk_node, key, value) values ("
					+ to_string((*this)[u].id) + ", '"
					+ escape(key) + "', '" + escape(value) + "')");
			}
			
			std::string get_vertex_meta_value(const Vertex u, const std::string key, const std::string def = "") {
				query("select value from node_meta where fk_node = " + to_string((*this)[u].id) + " and key='" + escape(key) + "'");
				std::string value;
				if (rows()>0) value = field(0,0);
				else value = def;
				free();
				
				return value;
			}
			
			// specific functions for this storage policy
#ifdef WIN32
			void open() throw (...) {
#else
			void open() throw (SQLiteException) {
#endif
				if (m_connected) return; // already opened
				m_connected = true;
				m_con = NULL;
				int result = sqlite3_open(m_file.c_str(), &m_con);
				// std::cerr << this << " opening new sqlite connection: " << m_file << std::endl;
				
				if (result) {
					m_connected = false;
					throw SQLiteException(m_con);
				}
				
				// check to make sure we have the collection table, which means all the other tables should exist too
				// if we don't, create the tables
				query("select count(*) from sqlite_master where type = 'table' and name = 'collection'");
				int num = atoi(field(0,0));
				free();
				
				if (num == 0) {
					// create the tables
					std::vector<std::string> queries;
					queries.push_back("CREATE TABLE 'collection' ( 'id' integer primary key, 'name' text unique, 'last_update' integer default CURRENT_TIMESTAMP )");
					queries.push_back("CREATE TABLE 'collection_meta' ( 'fk_collection' integer, 'key' text, 'value' text, UNIQUE ('fk_collection','key') )");
					queries.push_back("CREATE TABLE 'degree' ( 'fk_node' integer, 'type_major' integer, 'degree' integer, PRIMARY KEY ('fk_node','type_major') )");
					queries.push_back("CREATE TABLE 'content' ( 'id' integer primary key, 'content' text unique )");
					queries.push_back("CREATE TABLE 'edge' ( 'id' integer primary key, 'fk_node_from' integer, 'fk_node_to' integer, 'strength' real, UNIQUE ('fk_node_from','fk_node_to') )");
					queries.push_back("CREATE TABLE 'edge_query' ( 'id' integer primary key, 'fk_collection' integer, 'fk_node_from' integer, 'fk_node_to' integer, 'strength' integer, 'degree_from' integer, 'degree_to' integer, 'type_major' integer, 'type_minor' integer, UNIQUE ('fk_node_from', 'fk_node_to') )");
					queries.push_back("CREATE TABLE 'node' ( 'id' integer primary key, 'fk_collection' integer, 'type_major' integer, 'type_minor' integer, 'fk_content' integer, UNIQUE('fk_collection', 'type_major', 'type_minor', 'fk_content') )");
					queries.push_back("CREATE TABLE 'node_count' ( 'fk_collection' integer, 'type_major' integer, 'count' integer, PRIMARY KEY ('fk_collection','type_major') )");
					queries.push_back("CREATE TABLE 'node_meta' ( 'fk_node' integer, 'key' text, 'value' text, UNIQUE('fk_node','key') )");
					queries.push_back("create trigger node_ad after delete on node for each row begin delete from degree where fk_node = OLD.id; delete from edge where fk_node_from = OLD.id or fk_node_to = OLD.id; delete from node_meta where fk_node = OLD.id; delete from edge_query where fk_node_from = OLD.id or fk_node_to = OLD.id; end");
					queries.push_back("create trigger collection_ad after delete on collection for each row begin delete from node where fk_collection=OLD.id; delete from node_count where fk_collection=OLD.id; delete from collection_meta where fk_collection=OLD.id; end");
					
					// execute each query
					for(unsigned int i = 0; i < queries.size(); i++) {
						query(queries[i]);
					}
				}
				
				// done.
				m_connected=true;
			}
						
			void close() {
				if (!m_connected) return; // not opened
				sqlite3_close(m_con);
//				std::cerr << "closing connection." << std::endl;
				m_con = NULL;
				m_connected=false;
			}
			
			void reset_collection() {
				query("DELETE FROM node WHERE fk_collection="+to_string(get_collection_id()));
				query("DELETE FROM node_count WHERE fk_collection="+to_string(get_collection_id()));
				query("DELETE FROM collection_meta WHERE fk_collection="+to_string(get_collection_id()));
			}

			void set_file(std::string file) { m_file = file; }
		
			std::string get_file() { return m_file; }
			
		protected:
			id_type create_content_row(std::string content) {
				query("insert or ignore into content (content) values ('" + escape(content) + "')");
				query("select id from content where content = '" + escape(content) + "'");
				id_type id = strtoul(field(0,0), NULL, 10);
				free();
				return id;
			}
			
			id_type create_node(int collection, int type_major, int type_minor, id_type content_id) {
				id_type id;
				query("select id from node where fk_collection = " + to_string(collection)
					+ " and type_major = " + to_string(type_major) + " and type_minor = " + to_string(type_minor) + " and fk_content = " + to_string(content_id));
				if (rows() > 0) {
					// we have it already
					id = strtoul(field(0,0), NULL, 10);
					free();
				} else {
					free();
					query("insert into node (fk_collection, type_major, type_minor, fk_content) values ("
						+ to_string(collection) + ", " + to_string(type_major) + ", " 
						+ to_string(type_minor) + ", " + to_string(content_id) + ")");
					id = (id_type)sqlite3_last_insert_rowid(m_con);
				}
				
				return id;
			}
			
			void indexing_cleanup(id_type collection) {
				// first delete all the edge_query nodes that belong to this collection
				query("delete from edge_query where fk_collection = " + to_string(collection));
				
				// update node counts
				query("delete from node_count");
				query("insert into node_count select fk_collection, type_major, count(id) from node group by fk_collection, type_major");
				
				// update degrees of nodes
				query("delete from degree");
				query("insert into degree select fk_node_from as fk_node, type_major, count(fk_node_to) as degree from edge, node where edge.fk_node_to = node.id group by fk_node, type_major");
				
				int min = atoi(get_meta_value("min", "1").c_str());
				float max_factor = (float)atof(get_meta_value("max_factor", "0.1").c_str());
				
				
				// now execute our edge_query building query
				std::string q = "insert or ignore into edge_query select e.id, n_from.fk_collection, e.fk_node_from, e.fk_node_to, e.strength, d_from.degree as degree_from, d_to.degree degree_to, n_to.type_major, n_to.type_minor from node n_from inner join edge e on e.fk_node_from = n_from.id inner join node n_to on e.fk_node_to = n_to.id inner join degree d_from on d_from.fk_node = e.fk_node_from and d_from.type_major = n_to.type_major left join degree d_to on d_to.fk_node = e.fk_node_to and d_to.type_major = n_from.type_major left join node_count nc on nc.fk_collection = n_from.fk_collection and nc.type_major = n_from.type_major where n_from.fk_collection = " + to_string(collection) + " and ((n_from.type_major = 2) or (d_to.degree >= " + to_string(min) + " and (d_to.degree < " + to_string(min) + "+2 or d_to.degree < nc.count * " + to_string(max_factor) + ")))";
				
				// execute it
				query(q);
			}
			
			void synchronize() {
				// synchronize what we have stored here with the database!
				// std::cerr << "begin committing changes" << std::endl;
				
				open();
				query("PRAGMA synchronous=OFF");
				query("BEGIN TRANSACTION");
				if (m_clear_all) {
				    // we are supposed to clear out the entire collection... we can do that.
				    remove_collection(get_property(*this, graph_name));
				    m_collection_id = (std::numeric_limits<id_type>::max)(); // reset cached id
				    get_collection_id(); // regenerate empty collection
				    
				    m_clear_all = false;
				    m_to_remove.clear();
				    m_to_remove_edges.clear();
				    m_to_clear.clear();
				    query("COMMIT TRANSACTION");
				}
				
				id_type collection = get_collection_id();
			
				// first step is: go through vertices and make sure they're in the graph
				typename traits::vertex_iterator vi, vi_end;

				for(boost::tie(vi, vi_end) = vertices(*this); vi != vi_end; ++vi) {
					if ((*this)[*vi].dirty) {
						// ok, needs an update
						if ((*this)[*vi].in_db) {
							// and it's already in the DB, so we have to update it.
							id_type content_id = create_content_row((*this)[*vi].content);
							
							// update the row
							query("update node set type_major = " + to_string((*this)[*vi].type_major) + ", type_minor = " + to_string((*this)[*vi].type_minor) + ", fk_content = " + to_string(content_id));
							(*this)[*vi].dirty = false;
						} else {
							// just add it
							id_type content_id = create_content_row((*this)[*vi].content);
							id_type node_id = create_node(collection, (*this)[*vi].type_major, (*this)[*vi].type_minor, content_id);

							(*this)[*vi].id = node_id;
							(*this)[*vi].in_db = true;
							(*this)[*vi].dirty = false; // ok, done with this one
							
							// put it in the cache
							m_id_vertex_cache[(*this)[*vi].id] = *vi;
						}
					}
				}
				
				// now go through the edges and batch add them
				typename traits::edge_iterator ei, ei_end;
				// vector for storing our batch jobs

				for(boost::tie(ei, ei_end) = edges(*this); ei != ei_end; ++ei) {
					if (!((*this)[*ei].dirty)) continue; // not dirty - does not need "cleaning"
					
					query("insert or replace into edge (fk_node_from, fk_node_to, strength) values (" + to_string((*this)[source(*ei, *this)].id) + "," + to_string((*this)[target(*ei, *this)].id) + "," + to_string((*this)[*ei].strength) + ")");
					
					(*this)[*ei].dirty = false;
				}
				
				// check for deleted vertices
				if (!m_to_remove.empty()) count_cache.clear(); // clear the vertex count cache so it's reloaded later
			    for(unsigned int i = 0; i < m_to_remove.size(); i++) {
    			    query("delete from node where id = " + to_string(m_to_remove[i]));
			    }
			    m_to_remove.clear();
			    
				// and edges
				for(unsigned int i = 0; i < m_to_remove_edges.size(); i++) {
				    id_type from, to;
				    boost::tie(from, to) = m_to_remove_edges[i];
    			    query("delete from edge where fk_node_from = " + to_string(from) + " and fk_node_to = " + to_string(to));
			    }
			    m_to_remove_edges.clear();
			    
			    for(unsigned int i = 0; i < m_to_clear.size(); i++) {
				    id_type id = m_to_clear[i];
    			    query("delete from edge where fk_node_from = " + to_string(id) + " or fk_node_to = " + to_string(id));
			    }
			    m_to_clear.clear();
				query("COMMIT TRANSACTION");
				query("BEGIN TRANSACTION");
				
				// perform cleanup
				// std::cerr << "doing cleanup" << std::endl;
				indexing_cleanup(collection);
				
				query("COMMIT TRANSACTION");
				// std::cerr << "done committing changes" << std::endl;
			}
			
			id_type get_collection_id() {
				if (m_collection_id == (std::numeric_limits<id_type>::max)()) {
					query("insert or ignore into collection (name) values ('" 
						+ escape(get_property(*this, graph_name)) + "')");
				
					query("select id from collection where name='" + escape(get_property(*this, graph_name)) + "'");
					m_collection_id = strtoul(field(0,0), NULL, 10);
					free();
				}
				
				return m_collection_id;
			}
			
			std::string escape(std::string str) {
				char *res = sqlite3_mprintf("%q", str.c_str());
				std::string res_str = std::string(res);
				sqlite3_free(res);
				
				return res_str;
			}

#ifdef WIN32
			void query(std::string q) throw(...) {
#else
			void query(std::string q) throw(SQLiteException) {
#endif
				open(); // first, in case
				
//				std::cerr << this << " " << q << std::endl;
				
				char *errmsg = 0;
				
				int r;
				int c = 0;
				do {
				    r = sqlite3_get_table(
						m_con,
						q.c_str(),
						&m_result,
						&m_rows,
						&m_cols,
						&errmsg
						);
					if (c>0) std::cerr << ".";
					if (c++ >= 20) break; // only try 20 times
				} while (r == SQLITE_BUSY || r == SQLITE_LOCKED);   // we can wait on these conditions
				if (errmsg) {m_connected = false; throw SQLiteException(errmsg);}
				if (r) {m_connected = false; throw SQLiteException(m_con);}
			}
			
			char *field(int row, int col) {
				// add 1 to row to pass over the column names
				int i = (row+1)*m_cols + col;
				return m_result[i];
			}
			
			int rows() { return m_rows; }
			
			void free() {
				sqlite3_free_table(m_result);
			}
			
		private:
			std::map<int, typename traits::vertices_size_type> count_cache;
			std::map<typename traits::vertex_id_type, typename traits::vertex_descriptor> m_id_vertex_cache;
			std::vector<typename traits::vertex_id_type> m_to_remove, m_to_clear;
			std::vector<std::pair<typename traits::vertex_id_type, typename traits::vertex_id_type> > m_to_remove_edges;
			bool m_clear_all;
			
			std::string m_file;

			sqlite3 *m_con;
			bool m_connected;
			
			id_type m_collection_id;
			
			// results for sql queries
			char **m_result;
			int m_rows, m_cols;
			bool mirror_flag;
	};
	
} // namespace semantic

#endif
#else // !SEMANTIC_HAVE_SQLITE3
	#error "SQLite support was not enabled at install time for the Semantic Engine packge."
#endif
