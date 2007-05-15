
#include <semantic/semantic.hpp>
#include <semantic/search.hpp>
#include <semantic/filter.hpp>
#include <semantic/indexing.hpp>
#include <semantic/subgraph.hpp>
#include <semantic/subgraph/pruning_random_walk.hpp>
#include <semantic/subgraph/bfs.hpp>
#if SEMANTIC_HAVE_MYSQL
#include <semantic/storage/mysql5.hpp>
#endif
#if SEMANTIC_HAVE_SQLITE3
#include <semantic/storage/sqlite3.hpp>
#endif
#include <semantic/weighting/lg.hpp>
#include <semantic/weighting/tf.hpp>
#include <semantic/weighting/idf.hpp>
#include <semantic/utility.hpp>
#include <semantic/ranking/spreading_activation.hpp>

#include <cstdlib>
#include <vector>
#include <string>
#include <fstream>
#include <iostream>

#ifdef __cplusplus
extern "C" {
#endif
#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"
#ifdef __cplusplus
}
#endif

#undef list

using namespace semantic;

#if SEMANTIC_HAVE_MYSQL

typedef SEGraph<MySQL5StoragePolicy> MySQLGraph;
typedef text_indexer<MySQLGraph> MySQLIndexer;
typedef SESubgraph<MySQL5StoragePolicy, PruningRandomWalkSubgraph, LGWeighting<TFWeighting, IDFWeighting, double> > MySQLSubgraph;
typedef search<MySQLSubgraph> MySQLSearchEngine;
typedef MySQLSearchEngine::search_results MySQLsearch_results;
typedef MySQLSearchEngine::sorted_results MySQLsorted_results;
#endif



#if SEMANTIC_HAVE_SQLITE3

typedef SEGraph<SQLite3StoragePolicy> SQLiteGraph;
typedef text_indexer<SQLiteGraph> SQLiteIndexer;
typedef SESubgraph<SQLite3StoragePolicy, PruningRandomWalkSubgraph, LGWeighting<TFWeighting, IDFWeighting, double> > SQLiteSubgraph;
typedef search<SQLiteSubgraph> SQLiteSearchEngine;
typedef SQLiteSearchEngine::search_results SQLitesearch_results;
typedef SQLiteSearchEngine::sorted_results SQLitesorted_results;
#endif


MODULE = Semantic::API		PACKAGE = Semantic::API

	
I32
have_sqlite()
	CODE:
		RETVAL = SEMANTIC_HAVE_SQLITE3;
	OUTPUT:
		RETVAL

I32
have_mysql()
	CODE:
		RETVAL = SEMANTIC_HAVE_MYSQL;
	OUTPUT:
		RETVAL
		
SV*
stoplist_location()
	PREINIT:
		std::string location;
	CODE:
		location = STOPLIST_INSTALL_LOCATION;
		RETVAL = newSVpv(location.c_str(), location.size());
	OUTPUT:
		RETVAL



MODULE = Semantic::API		PACKAGE = Semantic::API::MySQLSearch

#if SEMANTIC_HAVE_MYSQL

MySQLSearchEngine*
MySQLSearchEngine::new(...)
	PREINIT:
		MySQLSubgraph *g;
		MySQLSearchEngine *s;
		std::string collection, host, user, pass, db;
		unsigned int depth, trials, stemming;
		double top_edges;
		
	CODE:
		stemming = 1;
		top_edges = 0.3;
		depth = 4;
		trials = 100;
		// parse the options
		for(int i = 1; i < items; i++) {
			std::string key = std::string(SvPV_nolen(ST(i)));
			if (key == "collection")
				collection = std::string(SvPV_nolen(ST(i+1)));
			else if (key == "host")
				host = std::string(SvPV_nolen(ST(i+1)));
			else if (key == "username")
				user = std::string(SvPV_nolen(ST(i+1)));
			else if (key == "password")
				pass = std::string(SvPV_nolen(ST(i+1)));
			else if (key == "database")
				db = std::string(SvPV_nolen(ST(i+1)));
			else if ( key == "trials")
				trials = SvIV(ST(i+1));
			else if ( key == "depth")
				depth = SvIV(ST(i+1));
			else if ( key == "keep_top_edges")
				top_edges = SvNV(ST(i+1));
			else if ( key == "stemming")	
				stemming = SvIV(ST(i+1));
			i++;
		}
		
		
		
		if( db.empty() ){
			std::cerr << "A `database` must be specified in the constructor" << std::endl;
			XSRETURN_UNDEF;
		} else if( collection.empty() ){
			std::cerr << "A `collection` name must be specified in the constructor" << std::endl;
			XSRETURN_UNDEF;
		
		} else if( user.empty() ){
			std::cerr << "A `user` for the database must be specified in the constructor" << std::endl;
			XSRETURN_UNDEF;
		}
		
		g = new MySQLSubgraph(collection);
		if( host.size() ){
			g->set_host(host);
		} else {
			g->set_host("localhost");
		}
		g->set_user(user);
		if( pass.size() ){
			g->set_pass(pass);
		}
		g->set_database(db);
		g->clear();
		g->keep_only_top_edges(top_edges);
		g->set_depth(depth);
		g->set_trials(trials);
		
		s = new MySQLSearchEngine(*g, stemming);
		
		
		
		RETVAL = s;
		
	OUTPUT:
		RETVAL

#else 

void
new()
	CODE:
		std::cerr << "Semantic::API was not compiled with MySQL support!" << std::endl;
		exit(EXIT_FAILURE);


#endif

#if SEMANTIC_HAVE_MYSQL

void
MySQLSearchEngine::DESTROY()
	CODE:
		delete THIS;



SV*
MySQLSearchEngine::get_document_text(SVdoc)
	SV* SVdoc
	PREINIT:
		std::string text;
		SV* ret;

	CODE:
		text = THIS->get_document_text(SvPV_nolen(SVdoc));
		ret = newSVpv(text.c_str(), text.length());
		RETVAL = ret;

	OUTPUT:
		RETVAL


SV*
MySQLSearchEngine::summarize_text(SVtext)
	SV* SVtext
	PREINIT:
		std::string summary, text;

	CODE:
		text = SvPV_nolen(SVtext);
		summary = THIS->summarize_text(text);
		RETVAL = newSVpv(summary.c_str(),summary.length());

	OUTPUT:
		RETVAL		

			

AV*
MySQLSearchEngine::_summarize(AVids)
	AV*		AVids
	PREINIT:
		std::string summary, doc_id;
		AV *ret;
		int i;

	CODE:
		ret = newAV();
		for( i = 0; i <= av_len(AVids); ++i){
			doc_id = SvPV_nolen(*av_fetch(AVids,i,0));
			summary = THIS->summarize_document(doc_id);
			av_push(ret,newSVpv(summary.c_str(),summary.length()));
		}
		RETVAL = ret;

	OUTPUT:
		RETVAL

AV*
MySQLSearchEngine::_find_similar(AVids)
	AV*		AVids
	PREINIT:
		HV *terms, *docs;
		AV *ret;
		I32 i;
		MySQLsearch_results results;
		MySQLsorted_results doc_results, term_results;
		MySQLsorted_results::iterator pos;
		std::string content;
		std::vector<std::string> ids;

	CODE:
		for( i = 0; i <= av_len(AVids); ++i){
			ids.push_back(SvPV_nolen(*av_fetch(AVids,i,0)));
		}

		results = THIS->similar(ids.begin(),ids.end());
		doc_results = results.first;
		term_results = results.second;
		terms = newHV(); docs = newHV(); ret = newAV();
		sv_2mortal((SV*)terms);
		sv_2mortal((SV*)docs);
		sv_2mortal((SV*)ret);

		// run through the graph and add the docs and terms to their respective hashes
		for(pos = doc_results.begin(); pos != doc_results.end(); ++pos) {
			content = pos->first;
			hv_store(docs, content.c_str(), content.length(), newSVnv(pos->second), 0 );
		}
		for(pos = term_results.begin(); pos != term_results.end(); ++pos) {
			content = pos->first;
			hv_store(terms, content.c_str(), content.length(), newSVnv(pos->second), 0 );
		}
		av_store(ret,0,newRV((SV*)docs));
		av_store(ret,1,newRV((SV*)terms));

		RETVAL = ret;

	OUTPUT:
		RETVAL



AV*
MySQLSearchEngine::_keyword_search(SVquery)
	SV*		SVquery
	PREINIT:
		HV *terms, *docs;
		AV *ret;
		MySQLsearch_results results;
		MySQLsorted_results doc_results, term_results;
		MySQLsorted_results::iterator pos;
		std::string content;
		
	CODE:
		
		results = THIS->keyword(SvPV_nolen(SVquery));
		doc_results = results.first;
		term_results = results.second;
		terms = newHV(); docs = newHV(); ret = newAV();
		sv_2mortal((SV*)terms);
		sv_2mortal((SV*)docs);
		sv_2mortal((SV*)ret);
	
		// run through the graph and add the docs and terms to their respective hashes
		for(pos = doc_results.begin(); pos != doc_results.end(); ++pos) {
			content = pos->first;
			hv_store(docs, content.c_str(), content.length(), newSVnv(pos->second), 0 );
		}
		for(pos = term_results.begin(); pos != term_results.end(); ++pos) {
			content = pos->first;
			hv_store(terms, content.c_str(), content.length(), newSVnv(pos->second), 0 );
		}
		av_store(ret,0,newRV((SV*)docs));
		av_store(ret,1,newRV((SV*)terms));
		
		RETVAL = ret;
		
	OUTPUT:
		RETVAL

AV*
MySQLSearchEngine::_better_semantic_search(SVquery)
	SV*		SVquery
	PREINIT:
		HV *terms, *docs;
		AV *ret;
		MySQLsearch_results results;
		MySQLsorted_results doc_results, term_results;
		MySQLsorted_results::iterator pos;
		std::string content;

	CODE:
		results = THIS->do_better_search(SvPV_nolen(SVquery));
		doc_results = results.first;
		term_results = results.second;

		terms = newHV(); docs = newHV(); ret = newAV();
		sv_2mortal((SV*)terms);
		sv_2mortal((SV*)docs);

		// run through the graph and add the docs and terms to their respective hashes
		for(pos = doc_results.begin(); pos != doc_results.end(); ++pos) {
			content = pos->first;
			hv_store(docs, content.c_str(), content.length(), newSVnv(pos->second), 0 );
		}
		for(pos = term_results.begin(); pos != term_results.end(); ++pos) {
			content = pos->first;
			hv_store(terms, content.c_str(), content.length(), newSVnv(pos->second), 0 );
		}
		av_push(ret,newRV((SV*)docs));
		av_push(ret,newRV((SV*)terms));

		RETVAL = ret;

	OUTPUT:
		RETVAL


AV*
MySQLSearchEngine::_semantic_search(SVquery)
	SV*		SVquery
	PREINIT:
		HV *terms, *docs;
		AV *ret;
		MySQLsearch_results results;
		MySQLsorted_results doc_results, term_results;
		MySQLsorted_results::iterator pos;
		std::string content;

	CODE:
		results = THIS->semantic(SvPV_nolen(SVquery));
		doc_results = results.first;
		term_results = results.second;

		terms = newHV(); docs = newHV(); ret = newAV();
		sv_2mortal((SV*)terms);
		sv_2mortal((SV*)docs);
		
		// run through the graph and add the docs and terms to their respective hashes
		for(pos = doc_results.begin(); pos != doc_results.end(); ++pos) {
			content = pos->first;
			hv_store(docs, content.c_str(), content.length(), newSVnv(pos->second), 0 );
		}
		for(pos = term_results.begin(); pos != term_results.end(); ++pos) {
			content = pos->first;
			hv_store(terms, content.c_str(), content.length(), newSVnv(pos->second), 0 );
		}
		av_push(ret,newRV((SV*)docs));
		av_push(ret,newRV((SV*)terms));

		RETVAL = ret;

	OUTPUT:
		RETVAL

#endif

MODULE = Semantic::API		PACKAGE = Semantic::API::MySQLIndex

#if SEMANTIC_HAVE_MYSQL

MySQLIndexer*
MySQLIndexer::new(...)
	PREINIT:
		MySQLGraph* g;
		MySQLIndexer *index;
		std::string collection, host, user, pass, db, lexicon, home, parser, min, max, doc_min, encoding, store, stemming;
		std::ifstream file;
		
	CODE:
		// parse the options
		for(int i = 1; i < items; i++) {
			std::string key = std::string(SvPV_nolen(ST(i)));
			std::string val = std::string(SvPV_nolen(ST(i+1)));
			i++;

			if (key == "collection")
				collection = val;
			else if (key == "host")
				host = val;
			else if (key == "username")
				user = val;
			else if (key == "password")
				pass = val;
			else if (key == "database")
				db = val;
			else if (key == "lexicon")
				lexicon = val;
			else if (key == "term_type")
				parser = val;
			else if (key == "collection_miniumum")
				min = val;
			else if (key == "collection_maximum")
				max = val;
			else if (key == "document_minimum")
				doc_min = val;
			else if (key == "stemming")
				stemming = val;
			else if ( key == "encoding")
				encoding = val;
			else if ( key == "store_text")
				store = val;
		}
		
		if( db.empty() ){
			std::cerr << "A `database` must be specified in the constructor" << std::endl;
			XSRETURN_UNDEF;
		} else if( collection.empty() ){
			std::cerr << "A `collection` name must be specified in the constructor" << std::endl;
			XSRETURN_UNDEF;
		
		} else if( user.empty() ){
			std::cerr << "A `user` for the database must be specified in the constructor" << std::endl;
			XSRETURN_UNDEF;
		}
		
		
		g = new MySQLGraph(collection);
		if( host.size() ){
			g->set_host(host);
		} else {
			g->set_host("localhost");
		}
		g->set_user(user);
		if( pass.size() ){
			g->set_pass(pass);
		}
		g->set_database(db);
		g->set_mirror_changes_to_storage(true);
		
		index = new MySQLIndexer(*g, lexicon);
		if( !parser.empty() )
			index->set_parsing_method(parser);
		
		if( !min.empty() )
			index->set_collection_value("min",min);
		
		if( !max.empty() )
			index->set_collection_value("max",max);
		
		if( !doc_min.empty() )
			index->set_collection_value("doc_min",doc_min);
		
		if( !store.empty() && store == "0" )
			index->store_text(false);
			
		if( !stemming.empty() && stemming == "0" )
			index->set_stemming(false);
		
		RETVAL = index;

	OUTPUT:
		RETVAL

#else 

void
new()
	CODE:
		std::cerr << "Semantic::API was not compiled with MySQL support!" << std::endl;
		exit(EXIT_FAILURE);


#endif

#if SEMANTIC_HAVE_MYSQL

void
MySQLIndexer::set_parsing_method(SVmethod)
	SV* SVmethod;
	CODE:
		THIS->set_parsing_method(SvPV_nolen(SVmethod));

void
MySQLIndexer::set_default_encoding(SVencoding)
	SV* SVencoding;
	CODE:
		THIS->set_default_encoding(SvPV_nolen(SVencoding));
		
void
MySQLIndexer::set_pdf_layout(SVlayout)
	SV* SVlayout;
	CODE:
		THIS->set_pdf_layout(SvPV_nolen(SVlayout));


void
MySQLIndexer::prune_wordlist(Imin)
	SV* Imin;
	CODE:
		THIS->prune_wordlist(SvIV(Imin));

		
	
	
void
MySQLIndexer::add_word_filters(...)
	PREINIT:
		AV* array_tmp;
		std::string key;
		int val;
		std::set<std::string> list;
	CODE:
		// parse the options
		val = 1;
		for(int i = 1; i < items; i++) {
			key = SvPV_nolen(ST(i));
			if( key != "blacklist" && key != "whitelist"){
				val = SvIV(ST(i+1));
			} else if( SvROK(ST(i+1))) {
				array_tmp = (AV*)SvRV(ST(i+1));
				for( int j = 0; j <= av_len(array_tmp); ++j){
					list.insert( std::string(SvPV_nolen(*av_fetch(array_tmp,j,0))));
				}
			} else {
				XSRETURN_UNDEF;
			}
			
			i++;

			if (key == "too_many_numbers")
				THIS->add_word_filter(too_many_numbers_filter(val));
			else if (key == "minimum_length")
				THIS->add_word_filter(minimum_length_filter(val));
			else if (key == "maximum_word_length")
				THIS->add_word_filter(maximum_word_length_filter(val));
			else if (key == "maximum_phrase_length")
				THIS->add_word_filter(maximum_phrase_length_filter(val));
			else if (key == "minimum_phrase_length")
				THIS->add_word_filter(minimum_phrase_length_filter(val));
			else if (key == "blacklist")
				THIS->add_word_filter(blacklist_filter(list));
			else if (key == "whitelist")
				THIS->add_word_filter(whitelist_filter(list));
		}
		
		
		
void
MySQLIndexer::_index(SVid, SVtext, Iweight)
		SV* SVtext;
		SV* SVid;
		SV* Iweight;
		
	PREINIT:
		std::string id, text, encoding, m_text;
		int weight;
		
	CODE:
		id = SvPV_nolen(SVid);
		text = SvPV_nolen(SVtext);
		weight = SvIV(Iweight);
		m_text = THIS->index(id, text, weight);

void
MySQLIndexer::unindex(SVid)
		SV* SVid;
	PREINIT:
		std::string id;
	CODE:
		id = SvPV_nolen(SVid);
		THIS->unindex(id);

			
void
MySQLIndexer::DESTROY()
	CODE:
		delete THIS;

SV*
MySQLIndexer::_index_file(SVfilename, Iweight)
		SV* SVfilename;
		SV* Iweight;
	PREINIT:
		std::string filename, text;
		int weight;
		
	CODE:
		weight = SvIV(Iweight);
		filename = SvPV_nolen(SVfilename);
		text = THIS->index(filename, weight);
		
		RETVAL = newSVpv(text.c_str(), text.length());
		
	OUTPUT:
		RETVAL
	
bool
MySQLIndexer::_finish(Imin)
	SV* Imin
	CODE:
		RETVAL = THIS->finish(SvIV(Imin));

	OUTPUT:
	    RETVAL

#endif
		
MODULE = Semantic::API		PACKAGE = Semantic::API::SQLiteIndex

#if SEMANTIC_HAVE_SQLITE3

SQLiteIndexer*
SQLiteIndexer::new(...)
	PREINIT:
		SQLiteGraph* g;
		SQLiteIndexer *index;
		std::string collection, db, lexicon, home, parser, min, max;
		std::ifstream file;

	CODE:
		// parse the options
		for(int i = 1; i < items; i++) {
			std::string key = std::string(SvPV_nolen(ST(i)));
			std::string val = std::string(SvPV_nolen(ST(i+1)));
			i++;

			if (key == "collection")
				collection = val;
			else if (key == "database")
				db = val;
			else if (key == "lexicon")
				lexicon = val;
			else if ( key == "node_minimum")
				min = val;
			else if ( key == "node_maximum")
				max = val;
			else if ( key == "parser")
				parser = val;
		}

		if( db.empty() ){
			std::cerr << "A `database` must be specified in the constructor" << std::endl;
			XSRETURN_UNDEF;
		} else if( collection.empty() ){
			std::cerr << "A `collection` name must be specified in the constructor" << std::endl;
			XSRETURN_UNDEF;
		
		} 

		

		g = new SQLiteGraph(collection);
		g->set_file(db);
		g->set_mirror_changes_to_storage(true);

		index = new SQLiteIndexer(*g, lexicon);
		if( !parser.empty() )
			index->set_parsing_method(parser);
		
		if( !min.empty() )
			index->set_collection_value("min",min);
		
		if( !max.empty() )
			index->set_collection_value("max",max);
		
		RETVAL = index;

	OUTPUT:
		RETVAL

#else  

void
new()
	CODE:
		std::cerr << "Semantic::API was not compiled with SQLite support!" << std::endl;
		exit(EXIT_FAILURE);

#endif

#if SEMANTIC_HAVE_SQLITE3

void
SQLiteIndexer::set_default_encoding(SVencoding)
	SV* SVencoding;
	CODE:
		THIS->set_default_encoding(SvPV_nolen(SVencoding));

void
SQLiteIndexer::set_parsing_method(SVmethod)
	SV* SVmethod;
	CODE:
		THIS->set_parsing_method(SvPV_nolen(SVmethod));


void
SQLiteIndexer::set_pdf_layout(SVlayout)
	SV* SVlayout;
	CODE:
		THIS->set_pdf_layout(SvPV_nolen(SVlayout));


void
SQLiteIndexer::add_word_filters(...)
	PREINIT:
		AV* array_tmp;
		std::string key;
		int i, j, val;
		std::set<std::string> list;
	CODE:
		// parse the options
		val = 1;
		for(i = 1; i < items; i++) {
			key = SvPV_nolen(ST(i));
			if( key != "blacklist" && key != "whitelist"){
				val = SvIV(ST(i+1));
			} else if( SvROK(ST(i+1))) {
				array_tmp = (AV*)SvRV(ST(i+1));
				for( j = 0; j <= av_len(array_tmp); ++j){
					list.insert( std::string(SvPV_nolen(*av_fetch(array_tmp,j,0))) );
				}
			} else {
				XSRETURN_UNDEF;
			}

			i++;

			if (key == "too_many_numbers")
				THIS->add_word_filter(too_many_numbers_filter(val));
			else if (key == "minimum_length")
				THIS->add_word_filter(minimum_length_filter(val));
			else if (key == "maximum_word_length")
				THIS->add_word_filter(maximum_word_length_filter(val));
			else if (key == "maximum_phrase_length")
				THIS->add_word_filter(maximum_phrase_length_filter(val));
			else if (key == "minimum_phrase_length")
				THIS->add_word_filter(minimum_phrase_length_filter(val));
			else if (key == "blacklist")
				THIS->add_word_filter(blacklist_filter(list));
			else if (key == "whitelist")
				THIS->add_word_filter(whitelist_filter(list));
		}
		

		
void
SQLiteIndexer::_index(SVid, SVtext, Iweight)
		SV* SVtext;
		SV* SVid;
		SV* Iweight;
	PREINIT:
		std::string id, text, encoding;
		int weight;

	CODE:
		id = SvPV_nolen(SVid);
		text = SvPV_nolen(SVtext);
		weight = SvIV(Iweight);
		THIS->index(id, text, weight);


void
SQLiteIndexer::unindex(SVid)
		SV* SVid;
	PREINIT:
		std::string id;
	CODE:
		id = SvPV_nolen(SVid);
		THIS->unindex(id);


void
SQLiteIndexer::DESTROY()
	CODE:
		delete THIS;

SV*
SQLiteIndexer::_index_file(SVfilename, Iweight)
		SV* SVfilename;
		SV* Iweight;
	PREINIT:
		std::string filename, text;
		int weight;

	CODE:
		weight = SvIV(Iweight);
		filename = SvPV_nolen(SVfilename);
		text = THIS->index(filename, weight);

		RETVAL = newSVpv(text.c_str(), text.length());

	OUTPUT:
		RETVAL

bool
SQLiteIndexer::finish()
	CODE:
		RETVAL = THIS->finish();

	OUTPUT:
	    RETVAL

#endif

MODULE = Semantic::API		PACKAGE = Semantic::API::SQLiteSearch

#if SEMANTIC_HAVE_SQLITE3

SQLiteSearchEngine*
SQLiteSearchEngine::new(...)
	PREINIT:
		SQLiteSubgraph *g;
		SQLiteSearchEngine *s;
		std::string collection, file;
		unsigned int depth, trials;
		double top_edges;
	CODE:
		top_edges = 0.3;
		depth = 4;
		trials = 100;
		// parse the options
		for(int i = 1; i < items; i++) {
			std::string key = std::string(SvPV_nolen(ST(i)));
			if (key == "collection")
				collection = std::string(SvPV_nolen(ST(i+1)));
			else if (key == "database")
				file = std::string(SvPV_nolen(ST(i+1)));
			else if ( key == "trials")
				trials = SvIV(ST(i+1));
			else if ( key == "depth")
				depth = SvIV(ST(i+1));
			else if ( key == "keep_top_edges")
				top_edges = SvNV(ST(i+1));
		
			i++;
		}
		
		if( file.empty() ){
			std::cerr << "A `database` must be specified in the constructor" << std::endl;
			XSRETURN_UNDEF;
		} else if( collection.empty() ){
			std::cerr << "A `collection` name must be specified in the constructor" << std::endl;
			XSRETURN_UNDEF;
		
		} 
		
		
		g = new SQLiteSubgraph(collection);
		g->set_file(file);
		g->clear();
		g->keep_only_top_edges(top_edges);
		g->set_depth(depth);
		g->set_trials(trials);
		s = new SQLiteSearchEngine(*g);

		RETVAL = s;

	OUTPUT:
		RETVAL


#else

void
new()
	CODE:
		std::cerr << "Semantic::API was not compiled with SQLite support!" << std::endl;
		exit(EXIT_FAILURE);


#endif

#if SEMANTIC_HAVE_SQLITE3

void
SQLiteSearchEngine::DESTROY()
	CODE:
		delete THIS;


AV*
SQLiteSearchEngine::_summarize(AVids)
	AV*		AVids
	PREINIT:
		std::string summary, doc_id;
		AV *ret;
		int i;

	CODE:
		ret = newAV();
		for( i = 0; i <= av_len(AVids); ++i){
			doc_id = SvPV_nolen(*av_fetch(AVids,i,0));
			summary = THIS->summarize_document(doc_id);
			av_push(ret,newSVpv(summary.c_str(),summary.length()));
		}
		RETVAL = ret;

	OUTPUT:
		RETVAL

AV*
SQLiteSearchEngine::_find_similar(AVids)
	AV*		AVids
	PREINIT:
		HV *terms, *docs;
		AV *ret;
		I32 i;
		SQLitesearch_results results;
		SQLitesorted_results doc_results, term_results;
		SQLitesorted_results::iterator pos;
		std::string content;
		std::vector<std::string> ids;

	CODE:
		for( i = 0; i <= av_len(AVids); ++i){
			ids.push_back(SvPV_nolen(*av_fetch(AVids,i,0)));
		}

		results = THIS->similar(ids.begin(),ids.end());
		doc_results = results.first;
		term_results = results.second;
		terms = newHV(); docs = newHV(); ret = newAV();
		sv_2mortal((SV*)terms);
		sv_2mortal((SV*)docs);
		sv_2mortal((SV*)ret);

		// run through the graph and add the docs and terms to their respective hashes
		for(pos = doc_results.begin(); pos != doc_results.end(); ++pos) {
			content = pos->first;
			hv_store(docs, content.c_str(), content.length(), newSVnv(pos->second), 0 );
		}
		for(pos = term_results.begin(); pos != term_results.end(); ++pos) {
			content = pos->first;
			hv_store(terms, content.c_str(), content.length(), newSVnv(pos->second), 0 );
		}
		av_store(ret,0,newRV((SV*)docs));
		av_store(ret,1,newRV((SV*)terms));

		RETVAL = ret;

	OUTPUT:
		RETVAL






SV*
SQLiteSearchEngine::get_document_text(SVdoc)
	SV* SVdoc
	PREINIT:
		std::string text;
		SV* ret;

	CODE:
		text = THIS->get_document_text(SvPV_nolen(SVdoc));
		ret = newSVpv(text.c_str(), text.length());
		RETVAL = ret;

	OUTPUT:
		RETVAL



AV*
SQLiteSearchEngine::_keyword_search(SVquery)
	SV*		SVquery
	PREINIT:
		HV *terms, *docs;
		AV *ret;
		SQLitesearch_results results;
		SQLitesorted_results doc_results, term_results;
		SQLitesorted_results::iterator pos;
		std::string content;

	CODE:

		results = THIS->keyword(SvPV_nolen(SVquery));
		doc_results = results.first;
		term_results = results.second;
		terms = newHV(); docs = newHV(); ret = newAV();
		sv_2mortal((SV*)terms);
		sv_2mortal((SV*)docs);
		sv_2mortal((SV*)ret);

		// run through the graph and add the docs and terms to their respective hashes
		for(pos = doc_results.begin(); pos != doc_results.end(); ++pos) {
			content = pos->first;
			hv_store(docs, content.c_str(), content.length(), newSVnv(pos->second), 0 );
		}
		for(pos = term_results.begin(); pos != term_results.end(); ++pos) {
			content = pos->first;
			hv_store(terms, content.c_str(), content.length(), newSVnv(pos->second), 0 );
		}
		av_store(ret,0,newRV((SV*)docs));
		av_store(ret,1,newRV((SV*)terms));

		RETVAL = ret;

	OUTPUT:
		RETVAL

AV*
SQLiteSearchEngine::_semantic_search(SVquery)
	SV*		SVquery
	PREINIT:
		HV *terms, *docs;
		AV *ret;
		SQLitesearch_results results;
		SQLitesorted_results doc_results, term_results;
		SQLitesorted_results::iterator pos;
		std::string content;

	CODE:
		results = THIS->semantic(SvPV_nolen(SVquery));
		doc_results = results.first;
		term_results = results.second;

		terms = newHV(); docs = newHV(); ret = newAV();
		sv_2mortal((SV*)terms);
		sv_2mortal((SV*)docs);

		// run through the graph and add the docs and terms to their respective hashes
		for(pos = doc_results.begin(); pos != doc_results.end(); ++pos) {
			content = pos->first;
			hv_store(docs, content.c_str(), content.length(), newSVnv(pos->second), 0 );
		}
		for(pos = term_results.begin(); pos != term_results.end(); ++pos) {
			content = pos->first;
			hv_store(terms, content.c_str(), content.length(), newSVnv(pos->second), 0 );
		}
		av_push(ret,newRV((SV*)docs));
		av_push(ret,newRV((SV*)terms));

		RETVAL = ret;

	OUTPUT:
		RETVAL

#endif
