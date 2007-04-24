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
		void closeWindow();
		void getFilePaths();
		void removeFiles();
		void startIndexing();
		void finishIndexing();
		void escCloseWindow();
	
	signals:
		void collectionTitleChanged();
		void collectionWindowClosed();
		void indexingCompleted();
		
	private:
		QShortcut *closeShortcut;
		Graph *m_graph;
		void getCollectionData();
		void setupLayout();
		void setupConnections();
		void renameCollection(QString);
		void saveCollectionData();
		
		bool isIndexing;
		QMap<CollectionWidget::Roles,QVariant> collectionData;
		QProgressBar *indexingProgress;
		QListWidget *indexingStatus;
		QLabel *indexingLabel;
		
		QGroupBox *collectionBox;
		QGroupBox *directoriesBox;
		
		IndexingThread *m_indexingThread;
		QLineEdit *collectionTitle;		
		QLabel *documentCount;
		QLabel *termCount;
		QLabel *parserType;
		QListWidget *directoryListWidget;
		QPushButton *closeButton;
		QToolButton *addDocumentsButton;
		QToolButton *removeDocumentsButton;
		QPushButton *indexButton;
		QMap<int,QString> doc_counts;
		QMap<int,QString> term_counts;
		QMap<int,QString> parsers;
		QMap<int,QString> locations;
	
     
};


#endif
