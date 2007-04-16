#ifndef GRAPH_CONTROLLER_H
#define GRAPH_CONTROLLER_H

#include <QWidget>

#include "se_graph.h"

class GraphController : public QWidget 
{
	
	Q_OBJECT
	
	public:
		GraphController(QWidget *parent=0);
		void copyDataFile(QString &);
		void setDataFile(QString &);
		Graph* instance();
		QStringList listCollections();
		void initializeDataFile(QString &);
		
	private:
		QString m_dataFile;
		Graph *m_graph;

};


#endif
