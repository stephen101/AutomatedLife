#ifndef RESULTS_H
#define RESULTS_H

#include <QtGui>

#include "se_graph.h"
#include "tabbed_display.h"

typedef std::pair<std::string,double> single_result;
typedef std::vector<single_result> sorted_results;
typedef std::pair<sorted_results,sorted_results> docs_and_terms;


class SearchThread : public QThread {
    Q_OBJECT
    
    public:
        SearchThread(search<Graph> *, QStringList, searchType, QObject * = 0);
		QList<QPair<QString,double> > getSearchResults();
        QStringList getRelatedTerms();

    protected:
        void run();
        
    signals:
        void finishedSearching();
        void startedSearching();
                
    private:
        search<Graph> 					*searchEngine;
		searchType 						m_type;
		QStringList 					m_query;
        QList<QPair<QString,double> > 	m_resultList;
		QStringList 					m_topTerms;

}; // class SearchThread


class ResultsModel : public QAbstractListModel {
	Q_OBJECT
	
	public:
		enum Roles {
			TitleRole		= Qt::DisplayRole,
			DetailRole		= Qt::UserRole + 1, // summary
			IdRole,
			RankRole,
			OriginalTitleRole,                  // unaltered title
		};
	
		ResultsModel(search<Graph> *, QList<QPair<QString,double> > &, QObject *parent = 0);
		
		
		int rowCount(const QModelIndex &parent = QModelIndex()) const;
		QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

		QMap<int, QVariant> itemData(const QModelIndex &index) const;
		double min() const;
		double max() const;
		
	private:
		search<Graph>					*m_searchEngine;
		QList<QPair<QString,double> >   m_resultList;
		mutable QMap<int, QString>		    m_summaryList;
		
	    
};

class ResultsView : public QAbstractItemView {
	Q_OBJECT
	
	public:
		ResultsView(QWidget *parent = 0);
	


	
	protected:
	    QRect visualRect(const QModelIndex&) const;
		void scrollTo(const QModelIndex&, QAbstractItemView::ScrollHint);
		QModelIndex indexAt(const QPoint&) const;
		QModelIndex moveCursor(QAbstractItemView::CursorAction, Qt::KeyboardModifiers);
		int horizontalOffset() const;
		int verticalOffset() const;
		bool isIndexHidden(const QModelIndex&) const;
		void setSelection(const QRect&, QFlags<QItemSelectionModel::SelectionFlag>);
		QRegion visualRegionForSelection(const QItemSelection&) const;
		int rows(const QModelIndex &) const;
		int rowHeight() const;
		QRegion itemRegion(const QModelIndex &) const;
		QRect itemRect(const QModelIndex &) const;
		void paintEvent(QPaintEvent *event);
		void resizeEvent(QResizeEvent *);
		void scrollContentsBy(int, int);
		void updateGeometries();
		void mouseMoveEvent(QMouseEvent *);
		void dragEnterEvent(QDragEnterEvent *);
		
		QRect m_selectionRect;
		QPoint dragStartPosition;
};


class SearchResults : public QWidget
{
	Q_OBJECT

	
	public:
		SearchResults(search<Graph> *, QWidget *parent = 0);
		QStandardItemModel *createResultsModel(QObject *parent);
		QStringList getTopTerms();
		QPair<WeightingTraits::edge_weight_map,
				WeightingTraits::vertex_weight_map> get_weight_map();
		void startSearch(QStringList, searchType);
		QList<QPair<QString,double> > getSearchResults();
		//QList<QPair<QString,double> > doKeywordSearch(const QString &string);
		//QList<QPair<QString,double> > doSemanticSearch(const QString &string);



	signals:
		void displaySingleResultWindow(QString &);
		void searchFinished();
		
	public slots:
		void displayItem(const QModelIndex &);
		void searchCompleted();
		void searchStarted();

	private:
		void setupLayout();
		void setupConnections();
		void addRow(QStandardItemModel *model, const QString &title);
		
		QList<QPair<QString,double> > 	resultList;
		search<Graph> 					*searchEngine;
		QStringList 					topTerms;
		QLabel 							*related_terms;
		QLabel 							*related_label;
		ResultsView 					*search_result_list;
		SearchThread 					*m_searchThread;
		SearchStatus 					*searchStatus;
};





#endif


