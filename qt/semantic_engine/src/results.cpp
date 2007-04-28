
#include "results.h"

#include <iostream>
#include <string>
#include <vector>

// defines for display
#define D_RANK_BAR_WIDTH 50		// pixels
#define D_RANK_BAR_HEIGHT 10	// pixels
#define D_LEFT_SPACING 10		// pixels
#define D_TOP_SPACING 5		// pixels
#define D_RIGHT_SPACING 10		// pixels





/* ***************************************************************** *
 * 		SearchResults implementation
 * ***************************************************************** */
SearchResults::SearchResults(search<Graph> *g, QWidget *parent) 
	: QWidget(parent), searchEngine(g)
{
	setupLayout();
	setupConnections();
}

// for use with the clustering code
QPair<WeightingTraits::edge_weight_map,
		WeightingTraits::vertex_weight_map> SearchResults::get_weight_map(){
		std::pair<WeightingTraits::edge_weight_map,
					WeightingTraits::vertex_weight_map> maps = searchEngine->get_weight_map();
		return QPair<WeightingTraits::edge_weight_map,
						WeightingTraits::vertex_weight_map>(maps.first, maps.second);
}



QStringList SearchResults::getTopTerms(){
	return topTerms;
}

void SearchResults::displayItem(const QModelIndex &index){
	QString title = index.data(ResultsModel::TitleRole).toString();
	
	if( QFile::exists( title ) ){
		QDesktopServices::openUrl(QUrl::fromLocalFile(title));
	} else {
		/* emit */ displaySingleResultWindow(title);
	}
}


void SearchResults::setupLayout(){
	// display the related terms
	related_label = new QLabel();
	related_terms = new QLabel();
	related_terms->setAlignment(Qt::AlignLeft);
	related_terms->setWordWrap(true);
	QGridLayout *related = new QGridLayout();
	related->addWidget(related_label,0,0);
	related->addWidget(related_terms,0,1);
	related->setColumnStretch(0,0);
	related->setColumnStretch(1,1);
	
	// display the result list
	search_result_list = new ResultsView();
	search_result_list->setDragEnabled(true);
	search_result_list->setAlternatingRowColors(true);
	search_result_list->setSelectionMode(QAbstractItemView::ExtendedSelection);
	search_result_list->setModel(new ResultsModel(searchEngine, resultList));
	
	searchStatus = new SearchStatus();
	searchStatus->setAlignment(Qt::AlignCenter);
	searchStatus->setVisible(false);
	
	
	QVBoxLayout *layout = new QVBoxLayout();
	layout->setAlignment(Qt::AlignCenter);
	layout->addLayout(related);
	layout->addWidget(search_result_list);
	layout->addWidget(searchStatus);
	setLayout(layout);
}

void SearchResults::setupConnections(){
	connect(search_result_list,SIGNAL(doubleClicked(const QModelIndex &)),this,SLOT(displayItem(const QModelIndex &)));

//	connect(related_terms,SIGNAL(linkActivated(related_terms->text())),this,SLOT(doSemanticSearch(related_terms->text())));
}

void SearchResults::searchStarted(){
	resultList.clear();
	topTerms.clear();
	search_result_list->setVisible(false);
	searchStatus->setStatus("Searching ...");
	searchStatus->setVisible(true);
	
	related_label->clear();
	related_terms->clear();
	

}

void SearchResults::searchCompleted(){
	
	topTerms = m_searchThread->getRelatedTerms();
	if( topTerms.count() ){
		related_label->setFont(QFont("Lucida Grande",13,QFont::Bold));
		related_label->setText(tr("Related Terms: "));
		related_terms->setText(topTerms.join(", "));
	}

	resultList = resultList = m_searchThread->getSearchResults();
	if( resultList.count() ){
		ResultsModel *model = new ResultsModel(searchEngine, resultList);
		search_result_list->setModel(model);
		searchStatus->setVisible(false);
		search_result_list->setVisible(true);
	} else {
		searchStatus->setStatus("No Results");
//		search_result_list->setModel(new ResultsModel(searchEngine, resultList));
	}
	searchFinished();
		
}

QList<QPair<QString,double> > SearchResults::getSearchResults(){
	return resultList;
}


void SearchResults::startSearch(QStringList query, searchType type=SemanticSearch) {
    
    m_searchThread = new SearchThread(searchEngine, query, type);
    connect(m_searchThread, SIGNAL(startedSearching()), this, SLOT(searchStarted()));
    connect(m_searchThread, SIGNAL(finishedSearching()), this, SLOT(searchCompleted()));



    m_searchThread->start();
    
}





/* ******************************************************************** *
 *		Search Thread Implementation
 * ******************************************************************** */
SearchThread::SearchThread(search<Graph> *s, QStringList query, searchType type, QObject *parent) 
	: QThread(parent), searchEngine(s) 
{
	m_query = query;
	m_type = type;
}


QList<QPair<QString,double> > SearchThread::getSearchResults(){
	return m_resultList;
}


QStringList SearchThread::getRelatedTerms(){
	return m_topTerms;
}


void SearchThread::run() {
    startedSearching();

	sorted_results docs, terms;
	m_resultList.clear();
	m_topTerms.clear();
	
	if( m_query.size() > 0 ){
		// add threading here!
		docs_and_terms results;
		switch( m_type ){
			case SemanticSearch:
				results = searchEngine->semantic(m_query.at(0).toStdString());
//				results = searchEngine->do_better_search(m_query.at(0).toStdString());
				break;
			
			case KeywordSearch:
				results = searchEngine->keyword(m_query.at(0).toStdString());
				break;
			
			case SimilarSearch:
				std::vector<std::string> documents;
				for( int i=0;i<m_query.size(); ++i){
					documents.push_back( m_query.at(i).toStdString());
				}
				results = searchEngine->similar(documents.begin(), documents.end());
				break;
		}
		
		docs = results.first;
		terms = results.second;
		double min = 0;
		double range = 1;
		int resultCount = docs.size();
		if( resultCount > 1 ){
			range = docs[0].second - docs[resultCount-1].second;
			min = docs[resultCount-1].second;
		}
		for( unsigned i = 0; i < docs.size(); ++i){
			QString title = QString::fromStdString(docs[i].first);
			
			double normalizedWeight = (docs[i].second - min ) / range;
			m_resultList.push_back(QPair<QString,double>(title,normalizedWeight));
		}
		
		for( unsigned i = 0; i < 10 && i < terms.size(); i++){
			m_topTerms << QString::fromStdString(terms[i].first);
		}
	}


    
	finishedSearching();
}




/* ***************************************************************** *
 * 		ResultsModel implementation
 * ***************************************************************** */
ResultsModel::ResultsModel(search<Graph> *s, QList<QPair<QString,double> > &resultList, QObject *parent)
			: QAbstractListModel(parent), m_searchEngine(s)
{
	m_resultList = resultList; 
	m_summaryList.clear();
}


int ResultsModel::rowCount(const QModelIndex &) const 
{
	
	return m_resultList.size();
}

double ResultsModel::max() const
{
	int size = m_resultList.size();
	if( size > 0 ){
		return m_resultList[0].second;	
	} else {
		return 0;
	}
}

double ResultsModel::min() const
{
	int size = m_resultList.size();
	if( size > 0 ){
		return m_resultList[size-1].second;	
	} else {
		return 0;
	}
}

QVariant ResultsModel::data(const QModelIndex &index, int role) const
{
	
	if( !index.isValid() || index.row() >= m_resultList.size())
		return QVariant();		
		
	QPair<QString,double> singleResult = m_resultList[index.row()];
	QString title = singleResult.first;
		
	switch(role) {
		case TitleRole:
			return QString(title);
			break;
		case DetailRole:
			if(!m_summaryList.count(index.row())) {
				std::string summary = m_searchEngine->summarize_document(title.toStdString(),2);
				m_summaryList.insert(index.row(),QString::fromStdString(summary));
			}
			return QString(m_summaryList[index.row()]);
			break;
		case IdRole:
			return 101;
			break;
		case RankRole:
			return singleResult.second;
			break;
		case OriginalTitleRole:
		    return QString(singleResult.first);
		    break;
		default:
			return QVariant();
	}
	return QVariant();
}



QMap<int, QVariant>
ResultsModel::itemData(const QModelIndex &index) const 
{
	int row = index.row();
	int col = index.column();
	
	if (! index.isValid() || col != 0 || row < 0 || row > rowCount()) 
		return QMap<int, QVariant>();
				
	QMap<int, QVariant> data_map = QAbstractListModel::itemData(index);
	
	data_map.insert(TitleRole, QString("Vertex Content"));
	data_map.insert(DetailRole, QString("Summary"));
	data_map.insert(IdRole, 101);
	data_map.insert(RankRole, row);
	data_map.insert(OriginalTitleRole, QString("Original Title"));
	
	return data_map;
}



/* ********************************************************************** *
 * 		ResultsView implementation
 * ********************************************************************** */
ResultsView::ResultsView(QWidget *parent) 
	: QAbstractItemView(parent) 
{
	setDragDropMode(QAbstractItemView::DragOnly);
	setDragEnabled(true);
	setDropIndicatorShown(true);
};


QRect ResultsView::visualRect(const QModelIndex &index) const
	{
		QRect rect = QRect(0, rowHeight() * index.row(), width(), rowHeight());
	    if (rect.isValid())
	        return QRect(rect.left() - horizontalScrollBar()->value(),
	                     rect.top() - verticalScrollBar()->value(),
	                     rect.width(), rect.height());
	    else
	        return rect;
	}

int ResultsView::rowHeight() const {
	return 50;
}

void ResultsView::scrollTo(const QModelIndex &index, ScrollHint)
{
    QRect area = viewport()->rect();
    QRect rect = visualRect(index);

    if (rect.left() < area.left())
        horizontalScrollBar()->setValue(
            horizontalScrollBar()->value() + rect.left() - area.left());
    else if (rect.right() > area.right())
        horizontalScrollBar()->setValue(
            horizontalScrollBar()->value() + qMin(
                rect.right() - area.right(), rect.left() - area.left()));

    if (rect.top() < area.top())
        verticalScrollBar()->setValue(
            verticalScrollBar()->value() + rect.top() - area.top());
    else if (rect.bottom() > area.bottom())
        verticalScrollBar()->setValue(
            verticalScrollBar()->value() + qMin(
                rect.bottom() - area.bottom(), rect.top() - area.top()));

    update();
}	
	
QModelIndex ResultsView::moveCursor(QAbstractItemView::CursorAction action, Qt::KeyboardModifiers /*modifiers*/)
{
	QModelIndex index = currentIndex();
	
	switch(action) {
		case MoveLeft:
		case MoveUp:
			if (index.row() > 0)
				index = model()->index(index.row() - 1, index.column(), rootIndex());
			else
				index = model()->index(0, index.column(), rootIndex());
			break;
		case MoveRight:
		case MoveDown:
			if (index.row() < rows(index) - 1)
				index = model()->index(index.row() + 1, index.column(), rootIndex());
			else
				index = model()->index(rows(index) - 1, index.column(), rootIndex());
			break;
		default:
			break;
	}
	
	viewport()->update();
	
	return index;
}

int ResultsView::rows(const QModelIndex &index) const
{
    return model()->rowCount(model()->parent(index));
}

int ResultsView::horizontalOffset() const {
	return horizontalScrollBar()->value();
}

int ResultsView::verticalOffset() const {
	return verticalScrollBar()->value();
}

bool ResultsView::isIndexHidden(const QModelIndex &index) const {
	int row = index.row();
	
	// if we're off the top
	if ((row + 1) * rowHeight() - verticalOffset() < 0) return true;
	if (row * rowHeight() - verticalOffset() > height()) return true;
	
	return false;
}

void ResultsView::setSelection(const QRect& rect, QItemSelectionModel::SelectionFlags command) {

	QRect contentsRect = rect.translated(horizontalScrollBar()->value(),
                                         verticalScrollBar()->value()).normalized();

	int rows = model()->rowCount(rootIndex());
    int columns = model()->columnCount(rootIndex());
    QModelIndexList indexes;

    for (int row = 0; row < rows; ++row) {
        for (int column = 0; column < columns; ++column) {
            QModelIndex index = model()->index(row, column, rootIndex());
            QRegion region = itemRegion(index);
            if (!region.intersect(contentsRect).isEmpty())
                indexes.append(index);
        }
    }

    if (indexes.size() > 0) {
        int firstRow = indexes[0].row();
        int lastRow = indexes[0].row();
        int firstColumn = indexes[0].column();
        int lastColumn = indexes[0].column();

        for (int i = 1; i < indexes.size(); ++i) {
            firstRow = qMin(firstRow, indexes[i].row());
            lastRow = qMax(lastRow, indexes[i].row());
            firstColumn = qMin(firstColumn, indexes[i].column());
            lastColumn = qMax(lastColumn, indexes[i].column());
        }

        QItemSelection selection(
            model()->index(firstRow, firstColumn, rootIndex()),
            model()->index(lastRow, lastColumn, rootIndex()));
        selectionModel()->select(selection, command);
    } else {
        QModelIndex noIndex;
        QItemSelection selection(noIndex, noIndex);
        selectionModel()->select(selection, command);
    }
    
    m_selectionRect = rect;
    update();
}



QRegion ResultsView::itemRegion(const QModelIndex &index) const {
	if (!index.isValid()) return QRegion();
	
	return QRegion(0, rowHeight()*index.row(), width(), rowHeight());
}

QRegion ResultsView::visualRegionForSelection(const QItemSelection &selection) const
{
    int ranges = selection.count();

    if (ranges == 0)
        return QRect();

    // Note that we use the top and bottom functions of the selection range
    // since the data is stored in rows.

    int firstRow = selection.at(0).top();
    int lastRow = selection.at(0).bottom();

    for (int i = 0; i < ranges; ++i) {
        firstRow = qMin(firstRow, selection.at(i).top());
        lastRow = qMax(lastRow, selection.at(i).bottom());
    }

    QModelIndex firstItem = model()->index(qMin(firstRow, lastRow), 0, rootIndex());
    QModelIndex lastItem = model()->index(qMax(firstRow, lastRow), 0, rootIndex());

    QRect firstRect = visualRect(firstItem);
    QRect lastRect = visualRect(lastItem);

    return firstRect.unite(lastRect);
}

QModelIndex ResultsView::indexAt(const QPoint &point) const {
	if (model()->rowCount() == 0) return QModelIndex();
	
//	int wx = point.x() + horizontalScrollBar()->value();
    int wy = point.y() + verticalOffset();

	int w = wy/rowHeight(); // we want this floored, which conversion to "int" will do
	
	return model()->index(w, 0, rootIndex());
}

QRect ResultsView::itemRect(const QModelIndex &index) const {
	if (!index.isValid()) return QRect();
	
	return QRect(0, rowHeight()*index.row(), width(), rowHeight());
}





void ResultsView::paintEvent(QPaintEvent *event) {

	QItemSelectionModel *selections = selectionModel();
    
	QStyleOptionViewItem option = viewOptions();
	QStyle::State state = option.state;
	
	QBrush background = option.palette.base();
	QPen foreground(option.palette.color(QPalette::Foreground));
	QPen textPen(option.palette.color(QPalette::Text));
	QColor highlightedColor = option.palette.color(QPalette::HighlightedText);
	QPen highlightedPen(highlightedColor);
	QPen collectionPen(QColor(100,100,100));
	QColor highlightBackground = option.palette.color(QPalette::Highlight);
	QColor rankBarColor(200, 70, 70);
	QFont font("Verdana", 12);
	QFont small_font("Verdana", 9);
	QFontMetrics metrics(font);
	QFontMetrics small_metrics(small_font);
	
	QPainter painter(viewport());
	painter.setRenderHint(QPainter::Antialiasing);
	painter.setFont(font);
	
	painter.fillRect(event->rect(), background);
	painter.setPen(foreground);
	
	
	if (model()->rowCount() > 0) {
		painter.save();
		painter.translate(-1 * horizontalScrollBar()->value(), -1 * verticalScrollBar()->value());
		
		// go through each result item and draw it, if it's visible
		for(int i = 0; i < model()->rowCount(); i++) {
			QModelIndex index = model()->index(i, 0, rootIndex());
			if (!isIndexHidden(index)) {
				bool isH = (/*currentIndex()==index||*/selections->isSelected(index))?true:false;
				// the background color
				if (isH) {painter.setPen(highlightedPen); painter.fillRect(itemRect(index), highlightBackground); }
				else painter.setPen(foreground);
				
				// the title
				QString titleString = QVariant(i+1).toString() + ". " +
										model()->data(index, ResultsModel::TitleRole).toString();

				QString elidedTitle = metrics.elidedText(titleString, Qt::ElideMiddle, viewport()->width() - D_LEFT_SPACING - D_RIGHT_SPACING - D_RANK_BAR_WIDTH - 10 );
				
				
				QRect titleRect = metrics.boundingRect(elidedTitle);
				painter.drawText(D_LEFT_SPACING, i*rowHeight() + titleRect.height() + D_TOP_SPACING - 1, elidedTitle);
				
				
				// the rank bar
				double rank = model()->data(index, ResultsModel::RankRole).toDouble();
				int width = int(rank * D_RANK_BAR_WIDTH);
				if (width < 1) width = 1;
				if( width > D_RANK_BAR_WIDTH * 1)
					width = D_RANK_BAR_WIDTH;
					
					
				
				QRect barRect(viewport()->width() - D_RANK_BAR_WIDTH - D_RIGHT_SPACING, D_TOP_SPACING + i*rowHeight(), width, D_RANK_BAR_HEIGHT);
				painter.fillRect(barRect, isH?highlightedColor:rankBarColor);
				
				// the summary -- just below the title
				painter.save();
				painter.setFont(small_font);
				isH?painter.setPen(Qt::lightGray):painter.setPen(Qt::darkGray);
				QString summaryString = model()->data(index, ResultsModel::DetailRole).toString();
				summaryString.replace(QRegExp("\\s+"), " ");
				QString elidedSummary = small_metrics.elidedText(summaryString, Qt::ElideRight, (viewport()->width() - D_LEFT_SPACING - D_RIGHT_SPACING - D_RANK_BAR_WIDTH - 20)*2 );
				painter.drawText(	D_LEFT_SPACING + 10, 
									i*rowHeight() + titleRect.height() + D_TOP_SPACING + 4,
									viewport()->width() - D_LEFT_SPACING - D_RIGHT_SPACING - D_RANK_BAR_WIDTH - 10,
									small_metrics.boundingRect(summaryString).height()*2 + 2,
									Qt::TextWordWrap, elidedSummary);				    
				
				painter.restore();
			}
		}
		
		painter.restore();
	} 
}


void ResultsView::resizeEvent(QResizeEvent */*event*/) {
	updateGeometries();
}

void ResultsView::scrollContentsBy(int dx, int dy)
{
    viewport()->scroll(dx, dy);
}

void ResultsView::updateGeometries()
{
    horizontalScrollBar()->setPageStep(viewport()->width());
    horizontalScrollBar()->setRange(0, 0);
    verticalScrollBar()->setPageStep(viewport()->height());
    verticalScrollBar()->setRange(0, qMax(0, model()->rowCount()*rowHeight() - viewport()->height()));
}

void ResultsView::dragEnterEvent(QDragEnterEvent *event) {
	
	event->setDropAction(Qt::CopyAction);
	event->accept();
}


void ResultsView::mouseMoveEvent(QMouseEvent *){
	QItemSelectionModel *selections = this->selectionModel();
	QModelIndexList list = selections->selectedIndexes();
	for( int i = 0; i < list.size(); ++i){
		QDrag *drag = new QDrag(this);
		QModelIndex index = list.at(i);
		QMimeData *mimeData = new QMimeData;
		mimeData->setHtml(index.data(Qt::DisplayRole).toString());
		drag->setMimeData(mimeData);
		drag->start();
	}	
}




