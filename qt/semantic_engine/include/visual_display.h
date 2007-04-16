#ifndef _VISUAL_DISPLAY_H_
#define _VISUAL_DISPLAY_H_

// local
#include "se_graph.h"
#include "visual_gl_display.h"
#include "tabbed_display.h"

// semantic
#include <semantic/analysis/linlog.hpp>

// Qt
#include <QThread>
#include <QWidget>
#include <QPushButton>
#include <QButtonGroup>


class VisualDisplayComputeThread : public QThread {
    Q_OBJECT

    enum ThreadOptions {
		VisualIterations = 100
	};

    public:
        VisualDisplayComputeThread(Graph *, WeightMapType, QObject * = 0);
        void operator++();
        
    public slots:
        void safeTerminate();
        
    protected:
        virtual void run();
        
    signals:
        void startedComputation();
        void finishedComputation();
        
        void newPositionMap(PositionMap);
        
    private:
        Graph *m_graph;
        bool safeTerminate_, runLinLog_;
        WeightMapType weights_;
        PositionMap positions_;
}; // class VisualDisplayComputeThread

class VisualDisplayController : public QWidget {
    Q_OBJECT
    
    public:
        VisualDisplayController(Graph *, search<Graph> *, QWidget * = 0);
        virtual ~VisualDisplayController() {};
        void setWeightMap(WeightingTraits::edge_weight_map);
		void startSearch();
		void displayResults(searchType);
		
    public slots:
        void start();
        void stop();
        void updateDisplay(PositionMap);
		void processDoubleClickedTerm(QString &);
    
	signals:
		void displaySingleResultWindow(QString &);
		void doSearch(QString &);
		
    protected slots:
        void threadFinished();
        void threadStarted();
        void setOptions(int);
        
       	void termNodesBoxClicked(int);
		void docNodesBoxClicked(int);
		void showEdgesBoxClicked(int);
 
    private:
        void resetEverything();
    	void setStatus(QString);
		
    	Graph 						*m_graph;
        WeightingTraits::edge_weight_map m_weightMap;
		VisualGLDisplay             *display_;
        VisualDisplayComputeThread  *compute_;
        
        QPushButton                 *go_button_;
		QCheckBox					*m_showEdges;
		QCheckBox					*m_showDocNodes;
		QCheckBox					*m_showTermNodes;
        int                         options_flags_;
		SearchStatus 				*searchStatus;
		search<Graph> 				*m_searchEngine;  
		
}; // class VisualDisplayController

#endif /* _VISUAL_DISPLAY_HPP_ */
