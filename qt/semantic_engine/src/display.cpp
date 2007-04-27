
#include <iostream>
#include <semantic/search.hpp>

#include "display.h"
#include "locate_datafile.h"



SearchWidget::SearchWidget(QString dataFile, QWidget *parent) 
    : QWidget(parent)
{
	controller = new GraphController(this);
	if( !QFile::exists(dataFile) ){
		controller->initializeDataFile(dataFile);
	}
	
	collectionComboBox = new QComboBox();
	
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
		cluster_area = new ClusterWidget(m_graph);
		visualization_area = new VisualDisplayController(m_graph);
	
		populateCollectionMenu();
		
    }
}

void SearchWidget::populateCollectionMenu(){
	
	QStringList collections = controller->listCollections();
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
	find_similar = new QToolButton;
	find_similar->setMinimumSize(QSize(20,26));
	find_similar->setText(tr("Find Similar"));
	find_similar->setToolTip(tr("Find similar results"));
	
	clear = new QToolButton;
	clear->setIcon(QPixmap(":/icons/clear.png"));
	clear->setMinimumSize(QSize(20,26));
	clear->setToolTip(tr("Clear saved results"));
	
	
	
	// results tools
	tools->setSpacing(2);
	tools->setAlignment(Qt::AlignRight);
	tools->addWidget(clear);
	tools->addWidget(find_similar);
	tools->setMargin(0);
	
	QVBoxLayout *contents = new QVBoxLayout;
	contents->addWidget(basket);
	contents->addLayout(tools);
	saved->setLayout(contents);
	
	// settings 
	QGroupBox *settingsBox = new QGroupBox(tr("Search Settings"));
	QVBoxLayout *settingsLayout = new QVBoxLayout;
	
	QFont settingsFont("Lucida Grande",12);
	keyword = new QCheckBox(tr("Keyword"));
	keyword->setFont(settingsFont);
	
	pruningSpinBox = new QSpinBox;
	pruningSpinBox->setRange(10,100);
	pruningSpinBox->setSuffix("%");
	pruningSpinBox->setSingleStep(5);
	pruningSpinBox->setValue(30);
	pruningSpinBox->setMaximumWidth(60);
	pruningSpinBox->setToolTip(tr("Set the 'graph pruning factor' -- a higher value returns more results but is slower"));
	pruningSpinBox->setFont(settingsFont);
	
	collectionComboBox->setFont(settingsFont);
	
	QLabel *pruningLabel = new QLabel("Pruning");
	pruningLabel->setFont(settingsFont);
	
	QHBoxLayout *graphSettingsLayout = new QHBoxLayout();
	graphSettingsLayout->addWidget(keyword);
	graphSettingsLayout->addWidget(pruningSpinBox);
	graphSettingsLayout->addWidget(pruningLabel);
	
	settingsLayout->setAlignment(Qt::AlignTop);
	settingsLayout->addWidget(collectionComboBox);
	settingsLayout->addLayout(graphSettingsLayout);
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

void SearchWidget::updatePruning(int val){
	float pruning = (float)val/100;
	m_graph->keep_only_top_edges(pruning);
	
}

void SearchWidget::setupConnections(){
	connect(pruningSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updatePruning(int)));
	connect(keywordShortcut, SIGNAL(activated()),keyword, SLOT(toggle()));
	connect(clear,SIGNAL(clicked()), basket, SLOT(clearAll()));
	connect(queryField, SIGNAL(returnPressed()), search, SLOT(animateClick()));
	connect(search, SIGNAL(clicked()), this, SLOT(startSearch()));
	connect(indexer, SIGNAL(collectionTitleChanged()), this, SLOT(populateCollectionMenu()));
	connect(indexer, SIGNAL(collectionWindowClosed()), this, SLOT(populateCollectionMenu()));
	connect(search_area, SIGNAL(displaySingleResultWindow(QString &)),this, SLOT(showResultWindow(QString &)));
	connect(cluster_area, SIGNAL(displaySingleResultWindow(QString &)),this, SLOT(showResultWindow(QString &)));
	connect(visualization_area, SIGNAL(displaySingleResultWindow(QString &)), this, SLOT(showResultWindow(QString &)));
	connect(visualization_area, SIGNAL(doSearch(QString &)), this, SLOT(newSearch(QString &)));
	connect(search_area, SIGNAL(searchFinished()), this, SLOT(populateSearchResults()) );
	connect(find_similar, SIGNAL(clicked()), this, SLOT(findSimilar()));
	connect(collectionComboBox, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(changeCollection(const QString &)));
}

void SearchWidget::changeCollection(const QString &collection){
	m_graph->set_collection(collection.toStdString());
}



void SearchWidget::newSearch(QString &query){
	if( query.contains(" ") ){
		query = "\"" + query + "\"";
	}
	queryField->setText(query);
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
	find_similar->setEnabled(false);
	pruningSpinBox->setEnabled(false);
	keyword->setEnabled(false);
	search->setEnabled(false);
	collectionComboBox->setEnabled(false);
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
	pruningSpinBox->setEnabled(true);
	keyword->setEnabled(true);
	collectionComboBox->setEnabled(true);
	
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


