#ifndef CLUSTERS_H
#define CLUSTERS_H

#include <QtGui>

#include "se_graph.h"
#include "tabbed_display.h"


class ClusteringThread : public QThread {
    Q_OBJECT
    
    public:
        ClusteringThread(Graph *, WeightMapType, QObject * = 0);
        semantic::dendrogram<Graph> &dendrogram();
        
    protected:
        void run();
        
    signals:
        void progress(int,int);
        void finishedClustering();
        void startedClustering();
        
    public slots:
        void safeTerminate();
        
    private:
        Graph                *graph_;
        WeightMapType           weights_;
        semantic::dendrogram<Graph>
                                dendrogram_;
        bool                    safeTerminate_;
}; // class ClusteringThread


class ClusterWidget : public QWidget {
	
	Q_OBJECT
	
	public:
		ClusterWidget(Graph *g, QWidget *parent=0);
		void setResultData(QStringList,QList<QPair<QString,double> >);
		void setWeightMap(QPair<WeightingTraits::edge_weight_map,WeightingTraits::vertex_weight_map>);
		void startSearch();
		void displayResults(searchType);
		
	public slots:
		void calculateClusters();
		void displayItem(QTreeWidgetItem *,int);
	
	signals:
		void displaySingleResultWindow(QString &);
        
    protected slots:
        // these are for the thread business
        void clusteringStarted();
		void updateProgress(int, int);      // steps completed out of total # of steps
        void clusteringCompleted();
        
        void killThread();
        void startThread();
        
        void numClustersChanged(int);


		
	private:
		void setupLayout();
		void setupConnections();
		void setStatus(QString);
		
		ClusteringThread    *m_clusteringThread;
		QBrush m_backgroundColor;
        maps::unordered<std::string,std::string> wordlist;
   		Graph *m_graph;
		unsigned maxClusters;
		WeightingTraits::edge_weight_map m_weightMap;
		WeightingTraits::vertex_weight_map m_rankMap;
		QTreeWidget *clusterTree;
		QPushButton *calculateClustersButton;
		QProgressBar *clusterProgress;
		QSpinBox *numberOfClusters;
		QLabel *clusterNumberLabel;
		QLabel *resultCountLabel;
		SearchStatus 				*searchStatus;
};





class ClusterTreeWidget : public QTreeWidget {
	
	Q_OBJECT
	
	public:
		ClusterTreeWidget(QTreeWidget *parent=0);
		
	protected:
		Qt::DropActions supportedDropActions() const;
		void mouseMoveEvent(QMouseEvent *);
};



#endif
