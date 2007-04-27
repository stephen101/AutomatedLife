#ifndef DISPLAY_H
#define DISPLAY_H

#include <QtGui>

#include "results.h"
#include "collection.h"
#include "basket.h"
#include "clusters.h"
#include "single_result.h"
#include "se_graph.h"
#include "graph_controller.h"
#include "visual_display.h"



class SearchWidget : public QWidget
{
	Q_OBJECT

	enum tabbedDisplayPositions {
		SearchTab = 0,
		ClusterTab = 1,
		VisualTab = 2
	};

	public:
		SearchWidget(QString dataFile=QString(), QWidget *parent = 0);
		void copyDataFile(QString);
	
	public slots:	
		void showCollectionWidget();
		void updatePruning(int);
		void findSimilar();
		void paste();
		void populateCollectionMenu();
		void showResultWindow(QString &);
		void openDataFile(QString dataFile=QString());
		void startSearch();
		void populateSearchResults();
		void newSearch(QString &);
		void changeCollection(const QString &);
		
	private:
		
		search<Graph> *m_searchEngine;
		Graph *m_graph;
		void setupLayout();
		void setupConnections();
		
		SingleResultWindow *resultWindow;
		SearchResults *search_area;
		ClusterWidget *cluster_area;
		VisualDisplayController *visualization_area;
		CollectionWidget *indexer;
		searchType m_searchMethod;
		
		QSpinBox *pruningSpinBox;
		QTabWidget *resultsTab;
		QShortcut *keywordShortcut;
		GraphController *controller;
		QComboBox *collectionComboBox;
		QLineEdit *queryField;
		QCheckBox *keyword;
		QCheckBox *randomness;
		QToolButton *clear;
		BasketWidget *basket;
		QToolButton *find_similar;
		QPushButton *search;
		QToolButton *manage;
		QToolButton *save;
};

#endif

