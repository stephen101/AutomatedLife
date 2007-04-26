#ifndef GRAPH_CONTROLLER_H
#define GRAPH_CONTROLLER_H

#include <QtGui>

#include "se_graph.h"

class GraphController : public QWidget 
{
	
	Q_OBJECT
	
	public:
		GraphController(QWidget *parent=0);
		Graph* instance();
		QStringList listCollections();
		
		// sqlite
		void copyDataFile(QString &);
		void setDataFile(QString &);
		void initializeDataFile(QString &);
		
	private:
		Graph *m_graph;
		int trials;
		int depth;
		float top_edges;
		// sqlite
		QString m_dataFile;
		// mysql
		QString m_database;
		QString m_host;
		QString m_user;
		QString m_password;

};


#endif
