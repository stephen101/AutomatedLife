#ifndef COLLECTION_H
#define COLLECTION_H

#include <QtGui>
#include <semantic/indexing.hpp>
#include <semantic/file_finder.hpp>

#include "se_graph.h"

class IndexingThread : public QThread {
    Q_OBJECT
    
    public:
        IndexingThread(Graph *, QStringList, QObject * = 0);
        
    protected:
        void run();
        
    signals:
		void status(QString);
        void progress(int,int);
        void finishedIndexing();
        void startedIndexing();
        
    public slots:
        void safeTerminate();
        
    private:
		std::set<std::string> load_stoplist(const std::string &);
		bool				 try_loading_stoplist(const std::string &, std::set<string> &);
        Graph                *m_graph;
		QStringList			 m_directories;
        bool                 m_safeTerminate;
}; // class ClusteringThread




class CollectionWidget : public QWidget
{
	
	Q_OBJECT
	
	public:
		CollectionWidget(Graph *, QWidget *parent=0);
		enum Roles {
			Title			= Qt::DisplayRole,
			Id				= Qt::UserRole + 1, // summary
			DocumentCount	= Qt::UserRole + 2,
			TermCount		= Qt::UserRole + 3,
			Parser			= Qt::UserRole + 4,
			Locations 		= Qt::UserRole + 5 // unaltered title
		};
		
		QStringList getCollectionList();
		void setWindowFlags(Qt::WindowFlags);
		void indexFiles();
	
	
	protected slots:
		void updateStatus(QString);
		void updateProgress(int,int);
		void startEditItem(QListWidgetItem *);
		void endEditItem(QListWidgetItem *, QListWidgetItem *);
		void endEditItem(QListWidgetItem *);
		void closeWindow();
		void getFilePaths();
		void removeFiles();
		void removeCollection();
		void startIndexing();
		void finishIndexing();
		void escCloseWindow();
	
	signals:
		void collectionTitleChanged();
		void collectionWindowClosed();
		
	private:
		QShortcut *closeShortcut;
		Graph *m_graph;
		void setupCollectionData();
		void setupLayout();
		void setupConnections();
		void renameCollection(QString);
		void saveCollectionData();
		
		QString m_dataFile;
		bool editOpen;
		bool isIndexing;
		QStringList directoryList;
		QString editPrevTitle;
		QSet<QString> collectionTitles;
		QMap<CollectionWidget::Roles,QVariant> collectionData;
		QProgressBar *indexingProgress;
		QListWidget *indexingStatus;
		
		IndexingThread *m_indexingThread;
		QListWidget *collectionsList;
		QLineEdit *collectionTitle;		
		QLabel *documentCount;
		QLabel *termCount;
		QLabel *parserType;
		QListWidget *directoryListWidget;
		QPushButton *closeButton;
		QToolButton *addCollectionButton;
		QToolButton *removeCollectionButton;
		QToolButton *addDocumentsButton;
		QToolButton *removeDocumentsButton;
		QPushButton *indexButton;
		QMap<int,QString> doc_counts;
		QMap<int,QString> term_counts;
		QMap<int,QString> parsers;
		QMap<int,QString> locations;
	
     
};


#endif
