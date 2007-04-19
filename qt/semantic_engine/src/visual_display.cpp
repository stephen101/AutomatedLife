
// local
#include "visual_display.h"


/**
 * Computing Thread -- handles the computation of LinLog 2D layout in the background,
 * updates the controller (in the main thread) with new position maps every iteration.
 */

VisualDisplayComputeThread::VisualDisplayComputeThread(Graph *g, WeightMapType w, QObject *parent)
    : QThread(parent), m_graph(g), safeTerminate_(false), runLinLog_(true), weights_(w) {}

void VisualDisplayComputeThread::safeTerminate() {
    safeTerminate_ = true;
    runLinLog_ = false; // this will stop the computation in linlog_layout_2d
    //std::cerr << "visual display thread safe terminating..." << std::endl;
}

void VisualDisplayComputeThread::operator++() {
    /* emit */ newPositionMap(positions_);
}

void VisualDisplayComputeThread::run() {
    /* emit */ startedComputation();
    positions_.clear();
    
    // ok, set it up!
    semantic::analysis::linlog_layout_2d(*m_graph, weights_, VisualIterations, positions_, *this, runLinLog_, true);  // use *this as callback (operator++) for each step
                                                                        // 'true' means update the position map on each step
    
    /* emit */ finishedComputation();
    
    // now just continue to emit newPositionMap
    while(1) {
        if(safeTerminate_) return;
        newPositionMap(positions_);
        msleep(30);
    }
}

/**
 * Controller -- this communicates between the compute thread and the display widget
 */

VisualDisplayController::VisualDisplayController(Graph *g, search<Graph> *s, QWidget *parent) 
	: QWidget(parent), m_graph(g), m_searchEngine(s)
{
    display_ = new VisualGLDisplay("hello");
    compute_ = 0;
    
    go_button_ = new QPushButton(tr("Create Visualization"));
    go_button_->setEnabled(false);
    
    // create the display settings...
	QHBoxLayout *commandLayout = new QHBoxLayout();
	m_showDocNodes = new QCheckBox(tr("Documents"));
	m_showTermNodes = new QCheckBox(tr("Terms"));
	m_showEdges = new QCheckBox(tr("Graph Edges"));
	commandLayout->addWidget(m_showDocNodes);
	commandLayout->addWidget(m_showTermNodes);
	commandLayout->addWidget(m_showEdges);
	commandLayout->addWidget(go_button_);
    
	searchStatus = new SearchStatus();
	searchStatus->setAlignment(Qt::AlignCenter);
	searchStatus->setVisible(false);



    QVBoxLayout *layout = new QVBoxLayout();
	layout->setAlignment(Qt::AlignCenter);
	layout->addSpacing(20);
	layout->addWidget(display_);
    layout->addLayout(commandLayout);
	layout->addWidget(searchStatus);
	setLayout(layout);
    
    qRegisterMetaType<PositionMap>("PositionMap");
    
    connect(go_button_, SIGNAL(pressed()), this, SLOT(start()));
	connect(m_showDocNodes, SIGNAL(stateChanged(int)), this, SLOT(docNodesBoxClicked(int)));
	connect(m_showTermNodes, SIGNAL(stateChanged(int)), this, SLOT(termNodesBoxClicked(int)));
	connect(m_showEdges, SIGNAL(stateChanged(int)), this, SLOT(showEdgesBoxClicked(int)));
	connect(display_, SIGNAL(termDoubleClicked(QString &)), this, SLOT(processDoubleClickedTerm(QString &)));
    connect(display_, SIGNAL(documentDoubleClicked(QString &)), this, SIGNAL(displaySingleResultWindow(QString &)));
	setOptions(SHOW_EDGES | SHOW_TERM_NODES | SIZE_DEGREE);
	
}

void VisualDisplayController::setStatus(QString status){
	searchStatus->setStatus(status);
}

void VisualDisplayController::processDoubleClickedTerm(QString &term){
	
	
	/* emit */ doSearch(term);
}


void VisualDisplayController::docNodesBoxClicked(int state) {
    if ( state == Qt::Checked) options_flags_ |= SHOW_DOC_NODES;
    else options_flags_ &= ~SHOW_DOC_NODES;
    
    display_->setOptions(options_flags_);
}

void VisualDisplayController::startSearch(){
	stop();
	display_->setVisible(false);
	go_button_->setVisible(false);
	m_showDocNodes->setVisible(false);
	m_showTermNodes->setVisible(false);
	m_showEdges->setVisible(false);
	setStatus(tr("Searching ..."));
	searchStatus->setVisible(true);
	
	
}



void VisualDisplayController::displayResults(searchType type){
	if( m_weightMap.size() > 0 ){
		if( type != KeywordSearch ){
			searchStatus->setVisible(false);
			display_->setVisible(true);
			go_button_->setVisible(true);
			m_showDocNodes->setVisible(true);
			m_showTermNodes->setVisible(true);
			m_showEdges->setVisible(true);
		} else {
			setStatus(tr("Visualization disabled on keyword searches"));
		}
	} else {
		setStatus(tr("No Results"));
	}
	
}


void VisualDisplayController::termNodesBoxClicked(int state) {
    if ( state == Qt::Checked ) options_flags_ |= SHOW_TERM_NODES;
    else options_flags_ &= ~SHOW_TERM_NODES;
    
    display_->setOptions(options_flags_);
}

void VisualDisplayController::showEdgesBoxClicked(int state) {
    if ( state == Qt::Checked ) options_flags_ |= SHOW_EDGES;
    else options_flags_ &= ~SHOW_EDGES;
    
    display_->setOptions(options_flags_);
}


void VisualDisplayController::setOptions(int o) {
    options_flags_=o;
    display_->setOptions(o);
	m_showDocNodes->setChecked(o & SHOW_DOC_NODES?true:false);
	m_showTermNodes->setChecked(o & SHOW_TERM_NODES?true:false);
	m_showEdges->setChecked(o & SHOW_EDGES?true:false);
}


void VisualDisplayController::setWeightMap(WeightingTraits::edge_weight_map weightMap){
	m_weightMap = weightMap;
	resetEverything();
}

void VisualDisplayController::resetEverything() {
    stop();
    display_->reset(m_graph, m_searchEngine, m_weightMap);
    go_button_->setEnabled(true);
}

void VisualDisplayController::start() {
    
	stop();
	if( m_weightMap.size() > 0 ){
	    display_->reset(m_graph, m_searchEngine, m_weightMap);
    
	    compute_ = new VisualDisplayComputeThread(m_graph, boost::make_assoc_property_map(m_weightMap));
	    connect(compute_, SIGNAL(newPositionMap(PositionMap)), this, SLOT(updateDisplay(PositionMap)));
	    connect(compute_, SIGNAL(finishedComputation()), this, SLOT(threadFinished()));
	    connect(compute_, SIGNAL(startedComputation()), this, SLOT(threadStarted()));
    
	    compute_->start();
	}
}

void VisualDisplayController::threadStarted() {
    go_button_->setEnabled(false);
}

void VisualDisplayController::threadFinished() {
    go_button_->setEnabled(true);
}

void VisualDisplayController::stop() {
    if (compute_ == 0) return; // nothing to do!
    compute_->safeTerminate();
    compute_->wait();               // wait for thread to terminate cleanly
    compute_->deleteLater();
    compute_ = 0;
    
	updateDisplay(PositionMap());   // reset the display with an empty position map
}

void VisualDisplayController::updateDisplay(PositionMap m) {
    display_->draw(m);
    display_->updateGL();
}
