#include <QFile>
#include <QDebug>
#include <QMessageBox>

#include "graph_controller.h"
#include "se_graph.h"
#include "locate_datafile.h"
#include <iostream>


// semantic
#include <semantic/search.hpp> // for unserialize_wordlist()

GraphController::GraphController(QWidget *parent) : QWidget(parent) {}

Graph *GraphController::instance() {

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
	g->set_trials(100);
	g->keep_only_top_edges(0.3);
	g->set_depth(4);
	return g;
}

void GraphController::initializeDataFile(QString &dataFile){
	Graph *g = new Graph("Untitled Collection");
	g->set_file(dataFile.toStdString());
	g->open();
	g->set_mirror_changes_to_storage(true);
	g->close();
}

QStringList GraphController::listCollections(){
	QStringList list;
	if( QFile::exists(m_dataFile) ){
		
		StorageInfo info_query;
		info_query.set_file(m_dataFile.toStdString());
		info_query.open();
		std::vector<std::string> collections = info_query.list_collections();
		for( unsigned i = 0; i < collections.size(); ++i ){
			list << QString::fromStdString(collections[i]);
		}
	} else {
		list << "Untitled Collection";
	}
	return list;
}

void GraphController::setDataFile(QString &fileName){
	if( QFile::exists(fileName) ){
		m_dataFile = fileName;
	} else {
		QMessageBox::warning(this, "Error", "File does not exist!" );
	}
}

void GraphController::copyDataFile(QString &fileName){
	if( QFile::exists(m_dataFile) ){
		bool success = QFile::copy(m_dataFile, fileName);
		// TODO thread this operation... or add to a status bar, etc.
		if( !success ){
			QMessageBox::warning(this, "Export Index Error", "Could not save exported index!" );
		}
	}
}
