/*
stores the graph in a MySQL 5 database

this follows the storage policy description found in docs/storage/policy.txt
*/

#if SEMANTIC_HAVE_MYSQL

#ifdef WIN32
#include <windows.h>
#include <winsock.h>
#endif


#include <map>
#include <semantic/properties.hpp>
#include <mysql.h>
#include <semantic/exception.hpp>
#include <semantic/storage/base.hpp>
#include <sstream>
//#include <iostream>

#ifndef __SEMANTIC_STORAGE_MYSQL5_HPP__
#define __SEMANTIC_STORAGE_MYSQL5_HPP__

namespace semantic {
	struct MySQL5StoragePolicy;
	
	// our exception
	struct MySQLException : std::exception {
		MySQLException(MYSQL *con) : number(mysql_errno(con)), msg(mysql_error(con)) {}
		MySQLException(std::string msg) : number(-1), msg(msg) {}
		~MySQLException() throw() {}
		const char * what() const throw() {
		 	std::string str = "MySQL Error " + to_string(number) + ": " + msg;
			return str.c_str();
		}
		
		int number;
		std::string msg;
	};
	
	// our custom vertex properties struct - includes the internal DB id of the vertex
	// and a flag saying if it's already there or not
	struct mysql_vertex_properties : vertex_properties {
		mysql_vertex_properties() : vertex_properties(), in_db(false) {}
		unsigned long id;
		bool in_db, dirty;
	};
	
	// our custom edge properties - includes an in_db flag and a "dirty" flag
	struct mysql_edge_properties : edge_properties {
		mysql_edge_properties() : edge_properties(), dirty(false) {}
		bool dirty;
	};
	
	template <> struct se_storage_traits<MySQL5StoragePolicy> {
		// this is the id type in the MySQL database (INT(10))
		typedef unsigned long vertex_id_type;
		typedef mysql_vertex_properties vertex_properties_type; // use our home-grown struct
		typedef mysql_edge_properties edge_properties_type;	  // ditto
	};
	
	template <class SEBase>
	class StoragePolicy<MySQL5StoragePolicy, SEBase> : public SEBase, public StoragePolicyBase<MySQL5StoragePolicy> {
		typedef se_graph_traits<MySQL5StoragePolicy> traits;
		typedef traits::base_graph_type base_graph_type;
		typedef traits::vertex_descriptor Vertex;
		typedef traits::edge_descriptor Edge;

		typedef se_storage_traits<MySQL5StoragePolicy> storage_traits;
		typedef storage_traits::vertex_properties_type vertex_properties;
		typedef storage_traits::edge_properties_type edge_properties;
		typedef storage_traits::vertex_id_type id_type;
		
		public:
			typedef SEBase base_type;
			
			// constructor(s)
			StoragePolicy() {
				m_con = NULL;
				m_collection_id = (std::numeric_limits<id_type>::max)();
				m_clear_all = false;
				mirror_flag = false;
			}
			~StoragePolicy() { disconnect(); }
/*			template <class S, class B>
			explicit StoragePolicy(StoragePolicy<S,B> &g) : base_type(g) {
				// rebuild the vertex cache
				BGL_FORALL_VERTICES_T(u, g, Graph) if (g[u].in_db) m_id_vertex_cache[g[u].id] = u;
			}
			template <class B>
			explicit StoragePolicy(StoragePolicy<MySQL5StoragePolicy, B> &g) : base_type(g) {
				// rebuild the vertex cache
				BGL_FORALL_VERTICES_T(u, g, Graph) if (g[u].in_db) m_id_vertex_cache[g[u].id] = u;
				// adopt members
				set_host(g.get_host());
				set_user(g.get_user());
				set_pass(g.get_pass());
				set_port(g.get_port());
				set_database(g.get_database());
				set_socket(g.get_socket());
			}*/
			
			// methods specific to this storage policy
			void set_host(std::string host) { m_host = host; }
			void set_user(std::string user) { m_user = user; }
			void set_pass(std::string pass) { m_pass = pass; }
			void set_port(unsigned int port) { m_port = port; }
			void set_database(std::string db) { m_database = db; }
			void set_socket(std::string socket) { m_socket = socket; }
		
			std::string get_host() { return m_host; }
			std::string get_user() { return m_user; }
			std::string get_pass() { return m_pass; }
			unsigned int get_port() { return m_port; }
			std::string get_database() { return m_database; }
			std::string get_socket() { return m_socket; }
		
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
			
			bool will_add_edge(Vertex , Vertex , const edge_properties &) {
				// if the edge exists, updating the properties will be handled by
				// the engine
				return true;
			}
			
			void did_add_edge(Edge e, bool , Vertex , Vertex , const edge_properties &) {
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
				
				MYSQL_RES *r = result();
				MYSQL_ROW row = mysql_fetch_row(r);
				if (!row) throw VertexContentNotFoundException(content);
				id = strtoul(row[0], NULL, 10);
				mysql_free_result(r);
				
				return id;
			}
			
			traits::vertices_size_type get_vertex_count_of_type(int node_type) {
				if (count_cache.count(node_type)) return count_cache[node_type];
				
				id_type cid = get_collection_id();
				std::string q = "select type_major, count from node_count where fk_collection = " + to_string(cid);
				query(q);
				
				MYSQL_RES *r = result_store();
				MYSQL_ROW row;
				while((row = mysql_fetch_row(r))) {
					int type = atoi(row[0]);
					traits::vertices_size_type cnt = strtoul(row[1], NULL, 10);
					count_cache.insert(std::make_pair(type, cnt));
				}
				
				mysql_free_result(r);
				
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
				
				// get the results
				MYSQL_RES *r = result();
				
				// and start generating the properties
				MYSQL_ROW row;
				while((row = mysql_fetch_row(r))) {
					vertex_properties p;
					p.id = strtoul(row[0], NULL, 10);
					p.type_major = atoi(row[1]);
					p.type_minor = atoi(row[2]);
					p.content = std::string(row[3]);
					p.in_db = true;
					
					*out = p;
				}
				
				mysql_free_result(r);
				return true;
			}
			
			// will populate out with std::pair<edge_properties, vertex_properties> for each
			// edge and adjacent vertex
			//		(an inserter(se_graph_traits::neighbor_list) can be used here)
			template <class InputIterator>
			bool fetch_vertex_neighbors(id_type id, InputIterator &out) {
				typename traits::mapped_neighbor_list m;
				id_type *p = &id;
				bool good = fetch_vertex_neighbors(p, p+1, m);
				if (!m.count(id)) return false;
				
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
				
				MYSQL_RES *r = result();
				MYSQL_ROW row;
				
				while((row = mysql_fetch_row(r))) {
					edge_properties ep;
					vertex_properties vp;
					
					id_type n_from = strtoul(row[0], NULL, 10);
					vp.id = strtoul(row[1], NULL, 10);
					vp.in_db = true;
					ep.strength = atoi(row[2]);
					ep.from_degree = atoi(row[3]);
					ep.to_degree = atoi(row[4]);
					vp.type_major = atoi(row[5]);
					vp.type_minor = atoi(row[6]);
					vp.content = std::string(row[7]);
					
					inserter(m[n_from], m[n_from].end()) = value_type(ep, vp);
				}
				
				mysql_free_result(r);
				
				return true;
			}
			
			// vertex meta data functions
			void set_vertex_meta_values(const std::map<Vertex, std::pair<std::string, std::string> > ){
				// select id from node where fk_collection = collection_id
				
			}
			
			void set_vertex_meta_value(const Vertex u, const std::string key, const std::string value) {
				query("INSERT IGNORE INTO node_meta (fk_node, `key`, value) VALUES ("
					+ to_string((*this)[u].id) + ", '"
					+ escape(key) + "', '" + escape(value) + "')");
			}
			
			std::string get_vertex_meta_value(const Vertex u, const std::string key, const std::string def = "") {
				query("SELECT value from node_meta where fk_node = " + to_string((*this)[u].id) + " and `key`='" + escape(key) + "'");
				std::string value;
				MYSQL_RES *r = result();
				MYSQL_ROW row = mysql_fetch_row(r);
				if (row) {
					value = row[0];
				} else {
					value = def;
				}
				mysql_free_result(r);
				return value;
			}
			
			// renames a collection in the index from one thing to another,
			// doesn't check for errors
			void rename_collection(std::string oldname, std::string newname) {
			    query("update collection set name = '" + escape(newname) + "' where name = '" + escape(oldname) + "'");
			}
			
			// removes the named collection
			void remove_collection(std::string name) {
			    query("delete from collection where name = '" + escape(name) + "'");
			}
			
			// reset the current collection
			void reset_collection(){
				query("DELETE FROM node WHERE fk_collection="+to_string(get_collection_id()));
				query("DELETE FROM node_count WHERE fk_collection="+to_string(get_collection_id()));
				query("DELETE FROM collection_meta WHERE fk_collection="+to_string(get_collection_id()));
			}

			// collection meta data functions
			void set_meta_value(const std::string key, const std::string value) {
				query("INSERT IGNORE INTO collection_meta (fk_collection, `key`, value) VALUES ("
					+ to_string(get_collection_id()) + ", '"
					+ escape(key) + "', '" + escape(value) + "')");
			}
			
			std::string get_meta_value(const std::string key, const std::string def = "") {
				query("SELECT value FROM collection_meta WHERE fk_collection = " + to_string(get_collection_id()) + " AND `key`='" + escape(key) + "'");
				std::string value;
				
				MYSQL_RES *r = result();
				MYSQL_ROW row = mysql_fetch_row(r);
				if (row) {
					value = row[0];
				} else {
					value = def;
				}
				mysql_free_result(r);
				return value;				
			}
			
			template <class Inserter>
			void get_collections_list(Inserter i) {
				query("select name from collection");
				MYSQL_RES *r = result_store();
				MYSQL_ROW row;
				
				while((row = mysql_fetch_row(r))) {
					*i = std::string(row[0]);
				}
				mysql_free_result(r);
			}

#ifdef WIN32
			void connect() throw (...) {
#else
			void connect() throw (MySQLException) {
#endif
				if (m_con != NULL) return; // already connected
				m_con = mysql_init(NULL);
				if (m_con == NULL) throw MySQLException("Could not initialize MYSQL connection struct.");
				// let's actually connect now
				if (mysql_real_connect(
						m_con,
						m_host.c_str(),
						m_user.c_str(),
						m_pass.c_str(),
						m_database.c_str(),
						m_port,
						m_socket.empty()?NULL:m_socket.c_str(),
						0) == NULL) {
					std::string err = mysql_error(m_con);
					m_con = NULL;
					throw MySQLException("Could not connect to MySQL server: " + err);
				}
				
				// done.
			}
			
			void disconnect() {
				if (m_con == NULL) return; // not connected
				mysql_close(m_con);
				m_con = NULL;
			}
			
		protected:
			bool mirror_flag;
			
			void synchronize() {
				// synchronize what we have stored here with the database!
				connect();
				query("start transaction");
				
				if (m_clear_all) {
				    // we are supposed to clear out the entire collection... we can do that.
				    remove_collection(get_property(*this, graph_name));
				    m_collection_id = (std::numeric_limits<id_type>::max)(); // reset cached id
				    get_collection_id(); // regenerate empty collection
				
				    m_clear_all = false;
				    m_to_remove.clear();
				    m_to_remove_edges.clear();
				    m_to_clear.clear();
				    query("commit");
				}
				
				id_type collection = get_collection_id();
				
				// see if we should set batch mode
				query("select count(*) from node where fk_collection = " + to_string(collection));
				MYSQL_RES *r = result();
				MYSQL_ROW row = mysql_fetch_row(r);
				int cnt = atoi(row[0]);
				mysql_free_result(r);
				if (cnt == 0) {
					query("set @batch_mode = 1");
				}
				
				// first step is: go through vertices and make sure they're in the graph
				typename traits::vertex_iterator vi, vi_end;
				for(boost::tie(vi, vi_end) = vertices(*this); vi != vi_end; ++vi) {
					if ((*this)[*vi].dirty) {
						// ok, needs an update
						if ((*this)[*vi].in_db) {
							// and it's already in the DB, so we have to update it.
							query("select create_content_row('" + escape((*this)[*vi].content) + "')");
							MYSQL_RES *r = result();
							MYSQL_ROW row = mysql_fetch_row(r);
							id_type content_id = strtoul(row[0], NULL, 10);
							mysql_free_result(r);
							
							// update the row
							query("update node set type_major = " + to_string((*this)[*vi].type_major) + ", type_minor = " + to_string((*this)[*vi].type_minor) + ", fk_content = " + to_string(content_id));
							(*this)[*vi].dirty = false;
						} else {
							// just add it
							std::string q = "select create_node_with_content(" + to_string(collection) + ", " +
								to_string((*this)[*vi].type_major) + ", " + to_string((*this)[*vi].type_minor) + ", '" +
								escape((*this)[*vi].content) + "')";
								
							query(q);
							// get the ID back
							MYSQL_RES *r = result();
							MYSQL_ROW row = mysql_fetch_row(r);
							(*this)[*vi].id = strtoul(row[0], NULL, 10); // base-10 unsigned long
							(*this)[*vi].in_db = true;
							(*this)[*vi].dirty = false; // ok, done with this one
							mysql_free_result(r);
							
							// put it in the cache
							m_id_vertex_cache[(*this)[*vi].id] = *vi;
						}
					}
				}
				
				// now go through the edges and batch add them
				typename traits::edge_iterator ei, ei_end;
				// vector for storing our batch jobs
				std::vector<std::string> to_add;
				for(boost::tie(ei, ei_end) = edges(*this); ei != ei_end; ++ei) {
					if (!((*this)[*ei].dirty)) continue; // not dirty - does not need "cleaning"
					if (to_add.size() >= 1000) {
						// add 1000 at a time
						do_batch_edges(to_add);
					}
					
					to_add.push_back("(" + to_string((*this)[source(*ei, *this)].id) + "," + to_string((*this)[target(*ei, *this)].id) + "," + to_string((*this)[*ei].strength) + ")");
					
					(*this)[*ei].dirty = false;
				}
				
				do_batch_edges(to_add);
				
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
			
				
				// perform cleanup
				query("call indexing_cleanup (" + to_string(collection) + ")");
				query("commit");
				query("set @batch_mode = NULL");
			}
			
			void do_batch_edges(std::vector<std::string> &to_add) {
				if (to_add.empty()) return;
				std::string q = "insert into edge (fk_node_from, fk_node_to, strength) values " + join(to_add.begin(), to_add.end(), ",") + " on duplicate key update strength=VALUES(strength)";
				
				// execute the query
				query(q);
				
				to_add.clear();
			}
			
			id_type get_collection_id() {
			    if (m_collection_id == (std::numeric_limits<id_type>::max)()) {
    				std::string q = "select get_collection_id('" + get_property(*this, graph_name) + "')";
    				query(q);
				
    				MYSQL_RES *r = result();
    				MYSQL_ROW row = mysql_fetch_row(r);
    				m_collection_id = strtoul(row[0], NULL, 10);
    				mysql_free_result(r);
				}
				
				return m_collection_id;
			}
			
			std::string escape(std::string from) {
				connect();
				char *escaped = new char[(unsigned long)from.size()*2 + 1];
                const char *from_const = const_cast<char *>(from.c_str());
				mysql_real_escape_string(m_con, escaped, from_const, (unsigned long)from.length());
				std::string e(escaped);
				delete [] escaped;
				return e;
			}
			
#ifdef WIN32
			void query(std::string q) throw(...) {
#else
			void query(std::string q) throw(MySQLException) {
#endif
				connect(); // first, in case
				if (mysql_real_query(m_con, q.c_str(), (unsigned long)q.length()))
					throw MySQLException(m_con);
			}
#ifdef WIN32		
			MYSQL_RES *result() throw(...) {
#else
			MYSQL_RES *result() throw(MySQLException) {
#endif
				MYSQL_RES *res = mysql_use_result(m_con);
				if (res == NULL) throw MySQLException(m_con);
				return res;
			}

#ifdef WIN32
			MYSQL_RES *result_store() throw(...) {
#else
			MYSQL_RES *result_store() throw(MySQLException) {
#endif
				MYSQL_RES *res = mysql_store_result(m_con);
				if (res == NULL) throw MySQLException(m_con);
				return res;
			}
		
		private:
			std::map<int, traits::vertices_size_type> count_cache;
			std::map<typename traits::vertex_id_type, typename traits::vertex_descriptor> m_id_vertex_cache;
			std::vector<typename traits::vertex_id_type> m_to_remove, m_to_clear;
			std::vector<std::pair<typename traits::vertex_id_type, typename traits::vertex_id_type> > m_to_remove_edges;
			bool m_clear_all;
			
			id_type m_collection_id;
			
			std::string m_host;
			std::string m_user;
			std::string m_pass;
			std::string m_database;
			unsigned int m_port;
			std::string m_socket;
			
			MYSQL *m_con;
	};
} // namespace semantic

#endif
#else // !SEMANTIC_HAVE_MYSQL
	#error "MySQL support was not enabled at install time for the Semantic Engine package!"
#endif
