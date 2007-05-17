#include <iostream>

// semantic
#include <semantic/analysis/linlog.hpp>
#include <semantic/analysis/agglomerate_clustering/cluster_helper.hpp>


#include "clusters.h"
#include "single_result.h"



void ClusterWidget::calculateClusters(){
	numberOfClusters->setEnabled(false);
	calculateClustersButton->setEnabled(false);
	numberOfClusters->setHidden(false);
	clusterNumberLabel->setHidden(false);
	startThread();
//	doClustering();
}

void ClusterWidget::updateProgress(int s, int total) {
    if (clusterProgress->maximum() != total) clusterProgress->setMaximum(total);
    clusterProgress->setValue(s);
}


ClusterWidget::ClusterWidget(Graph *g, QWidget *parent) 
	: QWidget(parent), m_graph(g)
{
	setupLayout();
	setupConnections();
}

void ClusterWidget::setStatus(QString status){
	searchStatus->setStatus(status);
}

void ClusterWidget::startSearch(){
	calculateClustersButton->setVisible(false);
	clusterProgress->setVisible(false);
	clusterTree->setVisible(false);
	numberOfClusters->setVisible(false);
	clusterNumberLabel->setVisible(false);	
	setStatus(tr("Searching ..."));
	searchStatus->setVisible(true);
	
}

void ClusterWidget::displayResults(searchType type){
	if( clusterTree->topLevelItemCount() > 0){
		if( type != KeywordSearch ){
			searchStatus->setVisible(false);
			clusterNumberLabel->setVisible(true);	
			clusterProgress->setVisible(true);
			calculateClustersButton->setVisible(true);
			clusterTree->setVisible(true);
			numberOfClusters->setVisible(true);
		} else {
			setStatus(tr("Clustering disabled on keyword searches"));
		}
	} else {
		setStatus(tr("No Results"));
	}
}

void ClusterWidget::setResultData(QStringList topTerms, QList<QPair<QString,double> > resultData){
	numberOfClusters->setHidden(true);
	clusterNumberLabel->setHidden(true);
	maxClusters = resultData.count() - 1;
	
	QString countStr = QVariant(resultData.count()).toString();
	QStringList labels;
	QStringList topFive;
	for( int i=0; i < topTerms.count() && i < 5; ++i){
		topFive << topTerms.at(i);
	}
	labels << countStr << topFive.join(", ");
	clusterTree->clear();
	if( resultData.count() > 0){
		QTreeWidgetItem *tResults = new QTreeWidgetItem(labels);
		tResults->setBackground(0,m_backgroundColor);
		tResults->setBackground(1,m_backgroundColor);
		for( int i = 0; i < resultData.count(); ++i){
			QStringList tLabels;
			QString title = resultData.at(i).first;

				
			tLabels << QString() << title;
			QTreeWidgetItem *item = new QTreeWidgetItem(tLabels);
			item->setFlags(Qt::ItemIsDragEnabled | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
			tResults->addChild(item);
		}
		clusterTree->addTopLevelItem(tResults);
		clusterTree->expandAll();
		calculateClustersButton->setEnabled(true);
	}
}



void ClusterWidget::setupConnections(){
	connect(calculateClustersButton, SIGNAL(clicked()), this, SLOT(calculateClusters()));
	connect(clusterTree, SIGNAL(itemDoubleClicked(QTreeWidgetItem *,int)),this, SLOT(displayItem(QTreeWidgetItem *,int)));
	connect(numberOfClusters, SIGNAL(valueChanged(int)), this, SLOT(numClustersChanged(int)));
}



void ClusterWidget::displayItem(QTreeWidgetItem *item,int){
	int col = 1; // valid column is always 1
	QString title = item->data(col,Qt::DisplayRole).toString();
	
	// allow double clicking only on child items
	if( !(item->flags() &= Qt::ItemIsUserCheckable | Qt::ItemIsDropEnabled) ){
		if( QFile::exists( title ) ){
			QDesktopServices::openUrl(QUrl(title));
		} else {
			/* emit */ displaySingleResultWindow(title);
		}
	} else {
		if( item->isExpanded() ){
			item->setExpanded(false);
		} else {
			item->setExpanded(true);
		}
	}
}

void ClusterWidget::setupLayout(){
	// cluster tree
	clusterTree = new ClusterTreeWidget;
	
	m_backgroundColor =	QBrush(QColor(160,206,221));


	// buttons and progress bar
	QHBoxLayout *statusLayout = new QHBoxLayout;
	clusterProgress = new QProgressBar;
	calculateClustersButton = new QPushButton(tr("Create Clusters"));
	calculateClustersButton->setEnabled(false);
	numberOfClusters = new QSpinBox;
	numberOfClusters->setMinimum(1);
	numberOfClusters->setMaximum(20);
	numberOfClusters->setHidden(true);
	clusterNumberLabel = new QLabel(tr("Number of Clusters"));
	clusterNumberLabel->setHidden(true);
	
	statusLayout->addWidget(clusterProgress);
	statusLayout->addWidget(clusterNumberLabel);
	statusLayout->addWidget(numberOfClusters);
	statusLayout->addWidget(calculateClustersButton);
	statusLayout->setAlignment(Qt::AlignRight);

	searchStatus = new SearchStatus();
	searchStatus->setAlignment(Qt::AlignCenter);
	searchStatus->setVisible(false);

	
	QVBoxLayout *mainLayout = new QVBoxLayout;
	mainLayout->addSpacing(20);
	mainLayout->addWidget(clusterTree);
	mainLayout->addLayout(statusLayout);
	mainLayout->addWidget(searchStatus);
	setLayout(mainLayout);
}

ClusterTreeWidget::ClusterTreeWidget(QTreeWidget *parent) 
		: QTreeWidget(parent)
	{
		setColumnCount(2);
		setSelectionMode(QAbstractItemView::ExtendedSelection);
		//setDragDropMode(QAbstractItemView::DragOnly);
		//setDragEnabled(true);

		QStringList labels;
		labels << tr("Document Count") << tr("Cluster");
		setHeaderLabels(labels);
		setTextElideMode(Qt::ElideRight);	
	}

Qt::DropActions ClusterTreeWidget::supportedDropActions() const {
	return Qt::CopyAction;
}

void ClusterTreeWidget::mouseMoveEvent(QMouseEvent *){
	QItemSelectionModel *selections = this->selectionModel();
	QModelIndexList list = selections->selectedRows(1);
	for( int i = 0; i < list.size(); ++i){
		QDrag *drag = new QDrag(this);
		QModelIndex index = list.at(i);
		// don't allow drags for top-level items
		if( !(index.flags() &= Qt::ItemIsUserCheckable | Qt::ItemIsDropEnabled) ){
			QMimeData *mimeData = new QMimeData;
			mimeData->setHtml(index.data(Qt::DisplayRole).toString());
			drag->setMimeData(mimeData);
			drag->start();
		}
	}	
}

void ClusterWidget::setWeightMap(QPair<WeightingTraits::edge_weight_map,WeightingTraits::vertex_weight_map> weightMap){
	m_weightMap = weightMap.first;
	m_rankMap = weightMap.second;
}





ClusteringThread::ClusteringThread(Graph *g, WeightMapType w, QObject *parent) 
	: QThread(parent), graph_(g), weights_(w), dendrogram_(*graph_), safeTerminate_(false) {}

void ClusteringThread::safeTerminate() {
    safeTerminate_ = true;
}

void ClusteringThread::run() {
    startedClustering();
    
    unsigned int max_num_clusters = num_vertices(*graph_)-1;
    if (max_num_clusters>20) max_num_clusters = 20;
	
	if( max_num_clusters < 3 ){
		return;
	}
	
	LinLog::all_maps maps;
    LinLogHelper::populate_all_maps(*graph_, weights_, maps);
    
    // create a minimizer
    semantic::analysis::detail::MinimizerBarnesHutHelper<LinLog::all_maps>
        minimizer(maps, 1.0f, 0.0f, 0.01f);
    
    const int iterations = 100;
    minimizer.setIterations(iterations);
    int step;
    while((step = minimizer.step()) != -1) {
        if (safeTerminate_) return;
        /*emit*/ progress(step, iterations + max_num_clusters);
    }
    progress(iterations, iterations + max_num_clusters);

    if (safeTerminate_) return;

	// use the position map to generate a distance map
    typedef semantic::se_graph_traits<Graph>::vertex_pair_edge edge;
    typedef semantic::se_graph_traits<Graph>::vertex_descriptor vertex;
    maps::unordered<edge, double> distances;
    maps::unordered<vertex, maps::unordered<vertex, double> > distances_2d;
    BGL_FORALL_VERTICES(u, *graph_, Graph) {
        if (safeTerminate_) return;
        BGL_FORALL_VERTICES(v, *graph_, Graph) {
            edge e1(u,v), e2(v,u);
            double distance = maps.position[maps.vertex_index[u]].dist(maps.position[maps.vertex_index[v]]);
            distances[e1] = distances[e2] = distance;
            distances_2d[u][v] = distances_2d[v][u] = distance;
        }
    }
    
    if (safeTerminate_) return;
    // now generate an MST
    std::vector<edge> mst;
	semantic::minimum_weight_spanning_tree(*graph_, extract_keys(distances.begin()), extract_keys(distances.end()), boost::make_assoc_property_map(distances), back_inserter(mst));
    if (safeTerminate_) return;
    
    // create the dendrogram
	semantic::dendrogram_from_distance_mst(*graph_, mst, boost::make_assoc_property_map(distances), dendrogram_, SingleLinkDistanceCalculator());
    
    // now let's silhouette this baby
    // measuring clusters between having 2 and having n-1 (or 30, whichever is smaller)... test silhouettes
	unsigned int best_num = 0;
    double best_silhouette = 0;
	for(unsigned int i = 2; i < max_num_clusters; i++) {
        maps::unordered<vertex, unsigned long> cluster_map;
        dendrogram_.set_num_clusters(i);
        dendrogram_.get_clusters(inserter(cluster_map, cluster_map.begin()));
        // silhouette!!!
        maps::unordered<vertex, double> quality_map;
        double quality = semantic::analysis::silhouette(*graph_, boost::make_assoc_property_map(cluster_map), boost::make_assoc_property_map(distances_2d), boost::make_assoc_property_map(quality_map));
        if (quality > best_silhouette) {
            best_silhouette = quality;
            best_num = i;
        }
        progress(iterations + i, iterations + max_num_clusters);
    }
	
	
    if (best_num < 2) best_num = 5; // why not
    
    dendrogram_.set_num_clusters(best_num);
    progress(iterations + max_num_clusters, iterations + max_num_clusters);
	
	finishedClustering();
}

semantic::dendrogram<Graph> & ClusteringThread::dendrogram() {
    return dendrogram_;
}


void ClusterWidget::numClustersChanged(int num) {
    if (!m_clusteringThread || m_clusteringThread->isRunning()) return;
    
    m_clusteringThread->dendrogram().set_num_clusters(num);
    clusteringCompleted(); // rebuild the view
}


void ClusterWidget::clusteringStarted() {
    clusterProgress->setEnabled(true);
}


void ClusterWidget::clusteringCompleted() {
    numberOfClusters->setEnabled(true);
	calculateClustersButton->setEnabled(true);
	typedef GraphTraits::vertex_descriptor vertex;

	// rebuild the view
    cluster_helper<Graph> helper;

    helper.build(   *m_graph, 
                    m_clusteringThread->dendrogram(), 
                    boost::make_assoc_property_map(m_rankMap));

    
  unsigned int count = helper.count();
    numberOfClusters->setValue(count);
     clusterTree->clear(); // just in case
	
    for(unsigned int i = 0; i < count; i++) {
        std::vector<vertex> docs, terms;
        docs = helper.docs(i);
        terms = helper.terms(i);
        
        
        std::vector<std::string> terms_str;
        for(unsigned int k = 0; k < terms.size() && k < 5; k++) {
        	
			std::string term = (*m_graph)[terms[k]].content;
			term = m_graph->unstem_term(term);
			if( term.size() ){
				terms_str.push_back( term );
			}
		}
        
        std::string terms_list = join(terms_str.begin(), terms_str.end(), ", ");
        
        QTreeWidgetItem *item = new QTreeWidgetItem(QStringList() << QString::number(docs.size()) << QString::fromStdString(terms_list));
        item->setBackground(0, m_backgroundColor);
        item->setBackground(1, m_backgroundColor);
        
        // get the document names now
        for(unsigned int k = 0; k < docs.size(); k++) {
			std::string doc_title = (*m_graph)[docs[k]].content;
			QString title = QString::fromStdString(doc_title);

/*			title = title.replace(QRegExp("&nbsp;?"), " ");
			title = title.replace(QRegExp("&#333;?"), "o");
			title = title.replace(QRegExp("&#332;?"), "O");
			title = title.replace(QRegExp("&#363;?"), "u");
			title = title.replace(QRegExp("&#362;?"), "U"); */
		
			QTreeWidgetItem *docItem = new QTreeWidgetItem(QStringList() << QString() << title);
			docItem->setFlags(Qt::ItemIsDragEnabled | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
			item->addChild(docItem);
	    }
        if( docs.size() > 0 ){
	        clusterTree->addTopLevelItem(item);
		}
    }
    
    clusterProgress->setEnabled(false);
	
}

void ClusterWidget::startThread() {
    
    m_clusteringThread = new ClusteringThread(m_graph, boost::make_assoc_property_map(m_weightMap));
    connect(m_clusteringThread, SIGNAL(startedClustering()), this, SLOT(clusteringStarted()));
    connect(m_clusteringThread, SIGNAL(finishedClustering()), this, SLOT(clusteringCompleted()));
    connect(m_clusteringThread, SIGNAL(progress(int, int)), this, SLOT(updateProgress(int, int)));
    m_clusteringThread->start();
}

void ClusterWidget::killThread() {
    if (m_clusteringThread!=0) {
        std::cerr << "stopping clustering thread" << std::endl;
        m_clusteringThread->safeTerminate();
        m_clusteringThread->wait();
        m_clusteringThread->deleteLater();
        m_clusteringThread = 0;
    }
    clusterProgress->setEnabled(false);
}

