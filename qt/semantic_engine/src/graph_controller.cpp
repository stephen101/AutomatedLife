#include <QtGui>

#include "graph_controller.h"
#include "locate_datafile.h"
#include <iostream>


// semantic
#include <semantic/search.hpp> // for unserialize_wordlist()

GraphController::GraphController(QWidget *parent) : QWidget(parent) {
	trials = 100;
	top_edges = (float)0.3;
	depth = 4;
#ifdef MYSQL_STORAGE
	m_host = QString("localhost");
	m_user = QString("semantic");
	m_database = QString("semantic_engine");
	m_port = 3306;
#endif
}

Graph *GraphController::instance() {

#ifdef SQLITE_STORAGE
	if( !m_dataFile.length() ){
		m_dataFile = DatafileLocator::getDatafileLocation();
	}
	qDebug() << "Opening semantic index" << m_dataFile;
	std::vector<std::string> collections;
	if( QFile::exists(m_dataFile) ){
		StorageInfo info_query;
		info_query.set_file(m_dataFile.toStdString());
		info_query.open();
		collections = info_query.list_collections();
	} 
	if( collections.size() == 0 ){
		collections.push_back("Untitled Collection");
	}
	Graph *g = new Graph(collections[0]);
	g->set_file(m_dataFile.toStdString());
	g->set_trials(trials);
	g->keep_only_top_edges(top_edges);
	g->set_depth(depth);
	return g;
#elif defined MYSQL_STORAGE
	
	StorageInfo info_query;
	info_query.set_host(m_host.toStdString());
	info_query.set_user(m_user.toStdString());
	info_query.set_database(m_database.toStdString());
	info_query.set_port(m_port);
	std::vector<std::string> collections = info_query.list_collections();

	if( collections.size() == 0 ){
		collections.push_back("Untitled Collection");
	}
	Graph *g = new Graph(collections[0]);
	g->set_host(m_host.toStdString());
	g->set_user(m_user.toStdString());
	g->set_database(m_database.toStdString());
	g->set_port(m_port);
	g->set_trials(trials);
	g->keep_only_top_edges(top_edges);
	g->set_depth(depth);
	return g;
#endif
}

#ifdef SQLITE_STORAGE
void GraphController::initializeDataFile(QString &dataFile){
	Graph *g = new Graph("Untitled Collection");
	g->set_file(dataFile.toStdString());
	g->open();
	g->set_mirror_changes_to_storage(true);
	g->close();
}
#else
void GraphController::initializeDataFile(QString &){}
#endif

QStringList GraphController::listCollections(){
	QStringList list;
	StorageInfo info_query;
	std::vector<std::string> collections;
#ifdef SQLITE_STORAGE
	if( QFile::exists(m_dataFile) ){
		
		info_query.set_file(m_dataFile.toStdString());
		info_query.open();
		collections = info_query.list_collections();
		for( unsigned i = 0; i < collections.size(); ++i ){
			list << QString::fromStdString(collections[i]);
		}
	}
#elif defined MYSQL_STORAGE
	info_query.set_host(m_host.toStdString());
	info_query.set_user(m_user.toStdString());
	info_query.set_database(m_database.toStdString());
	info_query.set_port(m_port);
	collections = info_query.list_collections();

	for( unsigned i = 0; i < collections.size(); ++i ){
		list << QString::fromStdString(collections[i]);
	}
	
#endif	
	if( list.size() == 0){
		list << "Untitled Collection";
	}
	return list;
}

#ifdef SQLITE_STORAGE
void GraphController::setDataFile(QString &fileName){
	if( QFile::exists(fileName) ){
		m_dataFile = fileName;
	} else {
		QMessageBox::warning(this, "Error", "File does not exist!" );
	}
}
#else
void GraphController::setDataFile(QString &){}
#endif

#ifdef SQLITE_STORAGE 
void GraphController::copyDataFile(QString &fileName){
	if( QFile::exists(m_dataFile) ){
		bool success = QFile::copy(m_dataFile, fileName);
		// TODO thread this operation... or add to a status bar, etc.
		if( !success ){
			QMessageBox::warning(this, "Export Index Error", "Could not save exported index!" );
		}
	}
}
#else
void GraphController::copyDataFile(QString &){}
#endif
