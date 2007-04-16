

#include <QWidget>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QHBoxLayout>
#include <QFont>
#include <QPixmap>
#include <QPushButton>
#include <QLineEdit>
#include <QTabWidget>
#include <QGridLayout>
#include <QToolButton>
#include <QFileDialog>
#include <QShortcut>

#include <iostream>
#include <semantic/search.hpp>

#include "display.h"
#include "results.h"
#include "basket.h"
#include "collection.h"
#include "clusters.h"
#include "single_result.h"
#include "graph_controller.h"
#include "locate_datafile.h"
#include "se_graph.h"



SearchWidget::SearchWidget(QString dataFile, QWidget *parent) 
    : QWidget(parent)
{
	controller = new GraphController(this);
	if( !QFile::exists(dataFile) ){
		controller->initializeDataFile(dataFile);
	}
	
	openDataFile(dataFile);

	setupLayout();
	keywordShortcut = new QShortcut(QKeySequence(tr("Ctrl+K", "Keyword")), parent);
	setupConnections();

}

void SearchWidget::copyDataFile(QString dataFile){
	controller->copyDataFile(dataFile);
}

void SearchWidget::openDataFile(QString dataFile){
	
    if( QFile::exists(dataFile ) ){
    	
    	
    	controller->setDataFile(dataFile);
    	m_graph = controller->instance();
    	m_searchEngine = new semantic::search<Graph>(*m_graph);
    	
    	indexer = new CollectionWidget(m_graph, this);
		indexer->hide();
		
		// tabbed result area
		search_area = new SearchResults(m_searchEngine);
		cluster_area = new ClusterWidget(m_graph, m_searchEngine);
		visualization_area = new VisualDisplayController(m_graph, m_searchEngine);
	
		populateCollectionMenu();
		
    }
}

void SearchWidget::populateCollectionMenu(){
	QStringList collections = controller->listCollections();
    collectionComboBox = new QComboBox();
	collectionComboBox->clear();
    for( int i = 0; i < collections.size(); ++i ){
        collectionComboBox->addItem(collections.at(i));
    }
}

void SearchWidget::setupLayout(){

	// saved results 
	
	QGroupBox *saved = new QGroupBox(tr("Saved Results"));
	basket = new BasketWidget;
	
	QHBoxLayout *tools = new QHBoxLayout;
	//QToolBar *tools = new QToolBar;
	find_similar = new QToolButton;
//	find_similar->setIcon(QPixmap(":/icons/find.png"));
//	find_similar->setIconSize(QSize(20,20));
	find_similar->setMinimumSize(QSize(20,26));
	find_similar->setText(tr("Find Similar"));
	find_similar->setToolTip(tr("Find similar results"));
	
	clear = new QToolButton;
	clear->setIcon(QPixmap(":/icons/clear.png"));
	clear->setMinimumSize(QSize(20,26));
//	clear->setIconSize(QSize(20,20));
	clear->setToolTip(tr("Clear saved results"));
	
	save = new QToolButton;
	save->setIcon(QPixmap(":/icons/save.png"));
	save->setIconSize(QSize(20,20));
	save->setToolTip(tr("Save these results"));
	
	// results tools
	tools->setSpacing(2);
	tools->setAlignment(Qt::AlignRight);
	tools->addWidget(clear);
	//tools->addWidget(save);
	tools->addWidget(find_similar);
	tools->setMargin(0);
	//find_similar = tools->addAction(tr("Find Similar"));
	//clear = tools->addAction(QIcon(QPixmap(":/icons/clear.png")), tr("Clear"));
	
	QVBoxLayout *contents = new QVBoxLayout;
	contents->addWidget(basket);
	contents->addLayout(tools);
	//contents->addWidget(tools);
	saved->setLayout(contents);
	
	// settings 
	QGroupBox *settingsBox = new QGroupBox(tr("Settings"));
	QVBoxLayout *settingsLayout = new QVBoxLayout;
	
		
	keyword = new QCheckBox(tr("Keyword Only"));
	
	settingsLayout->setAlignment(Qt::AlignTop);
	settingsLayout->addWidget(collectionComboBox);
	settingsLayout->addWidget(keyword);
	settingsBox->setLayout(settingsLayout);

	// query 
	QLabel *icon = new QLabel();
	icon->setPixmap(QPixmap(":/icons/nse.png"));
	QLabel *queryText = new QLabel(tr("Search "));
	queryText->setFont(QFont("Lucida Grande",15,QFont::Bold));
	queryField = new QLineEdit();
	search = new QPushButton(tr("&Search"));
	




/*  Main Layout  -- put it all together */

	// Query line
	QHBoxLayout *top = new QHBoxLayout();
	top->addWidget(icon);
	top->addWidget(queryText);
	top->addWidget(queryField);
	top->addWidget(search);
	
	// Right Margin
	QVBoxLayout *margin = new QVBoxLayout();
	margin->addWidget(saved);
	margin->addWidget(settingsBox);
	margin->setStretchFactor(settingsBox,0);
	margin->setStretchFactor(saved,1);

  	// Tab display 
	resultsTab = new QTabWidget;
	resultsTab->insertTab(SearchTab, search_area, tr("Results"));
	resultsTab->insertTab(ClusterTab, cluster_area, tr("Clusters"));
	resultsTab->insertTab(VisualTab, visualization_area, tr("Visualization"));

	QHBoxLayout *resultsLayout = new QHBoxLayout();
	resultsLayout->addWidget(resultsTab);
	resultsLayout->addLayout(margin);
	resultsLayout->setStretchFactor(resultsTab,1);
	resultsLayout->setStretchFactor(margin,0);
	
	QVBoxLayout *allLayout = new QVBoxLayout();
	allLayout->addLayout(top);
	allLayout->addLayout(resultsLayout);
	allLayout->setStretchFactor(resultsLayout,1);
	allLayout->setStretchFactor(top,0);

	
	setLayout(allLayout);

}

void SearchWidget::setupConnections(){
	connect(keywordShortcut, SIGNAL(activated()),keyword, SLOT(toggle()));
	connect(clear,SIGNAL(clicked()), basket, SLOT(clearAll()));
	connect(queryField, SIGNAL(returnPressed()), search, SLOT(animateClick()));
	connect(search, SIGNAL(clicked()), this, SLOT(startSearch()));
	connect(indexer, SIGNAL(collectionListChanged()), this, SLOT(populateCollectionMenu()));
	connect(search_area, SIGNAL(displaySingleResultWindow(QString &)),this, SLOT(showResultWindow(QString &)));
	connect(cluster_area, SIGNAL(displaySingleResultWindow(QString &)),this, SLOT(showResultWindow(QString &)));
	connect(visualization_area, SIGNAL(displaySingleResultWindow(QString &)), this, SLOT(showResultWindow(QString &)));
	connect(visualization_area, SIGNAL(doSearch(QString &)), this, SLOT(newSearch(QString &)));
	connect(search_area, SIGNAL(searchFinished()), this, SLOT(populateSearchResults()) );
	connect(find_similar, SIGNAL(clicked()), this, SLOT(findSimilar()));
}


void SearchWidget::newSearch(QString &query){
	std::cerr << "new search" << std::endl;
	if( query.contains(" ") ){
		query = "\"" + query + "\"";
	}
	queryField->setText(query);
	std::cerr << query.toStdString() << std::endl;
	startSearch();
}

void SearchWidget::showResultWindow(QString &title){
	QString content = QString::fromStdString(
								m_graph->get_vertex_meta_value(
									m_graph->vertex_by_id(
										m_graph->fetch_vertex_id_by_content_and_type(
											title.toStdString(), node_type_major_doc)), "body"));

	resultWindow = new SingleResultWindow(content, title);
	resultWindow->setWindowFlags(Qt::Window | Qt::WindowMinimizeButtonHint | Qt::WindowSystemMenuHint);
	
	resultWindow->show();
}

void SearchWidget::findSimilar(){
	find_similar->setEnabled(false);
	search->setEnabled(false);
	m_searchMethod = SimilarSearch;
	QStringList documents = basket->getSavedItems();
	if( documents.size() > 0){
		queryField->setText("");
		cluster_area->startSearch();
		visualization_area->startSearch();
		search_area->startSearch(documents, m_searchMethod);
	}
}

void SearchWidget::startSearch() { 
	//std::cerr << "started Search" << std::endl;
	find_similar->setEnabled(false);
	search->setEnabled(false);
	cluster_area->startSearch();
	visualization_area->startSearch();
	if( keyword->checkState() == Qt::Checked ){
		m_searchMethod = KeywordSearch;
	} else {
		m_searchMethod = SemanticSearch;
	}
	search_area->startSearch(QStringList(queryField->text()), m_searchMethod );
}

void SearchWidget::populateSearchResults(){
	//std::cerr << "search completed" << std::endl;
	
	QList<QPair<QString,double> > results = search_area->getSearchResults();
	
	
 	QPair<WeightingTraits::edge_weight_map,
			WeightingTraits::vertex_weight_map> weightMap = search_area->get_weight_map();
	QStringList topTerms = search_area->getTopTerms();
	
	cluster_area->setResultData(topTerms,results);
	cluster_area->setWeightMap(weightMap);
	cluster_area->displayResults(m_searchMethod);

	visualization_area->setWeightMap(weightMap.first);
	visualization_area->displayResults(m_searchMethod);

	find_similar->setEnabled(true);
	search->setEnabled(true);
	if( resultsTab->currentIndex() == VisualTab and m_searchMethod != KeywordSearch  ){
		visualization_area->start();
	}
}



void SearchWidget::paste(){
	queryField->paste();
}



void SearchWidget::showCollectionWidget(){
	indexer->setWindowFlags(Qt::Sheet);
	indexer->show();
}


