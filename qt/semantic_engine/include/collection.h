#ifndef COLLECTION_H
#define COLLECTION_H

#include <QWidget>
#include <QStringList>
#include <QListWidget>
#include <QLabel>
#include <QPushButton>
#include <QToolButton>

#include "se_graph.h"

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
	
	
	protected slots:
		void getCollectionData();
		void startEditItem(QListWidgetItem *);
		void endEditItem(QListWidgetItem *, QListWidgetItem *);
		void endEditItem(QListWidgetItem *);
		void addCollection();
		void closeWindow();
		void getFilePaths();
		void removeFiles();
		void removeCollection();
	
	signals:
		void collectionListChanged();
	
	private:
		Graph *m_graph;
		void setupCollectionData();
		void setupLayout();
		void setupConnections();
		void renameCollection(QString, QString);
		void saveCollectionData();
		
		QString m_dataFile;
		bool editOpen;
		QStringList directoryList;
		QString editPrevTitle;
		QSet<QString> collectionTitles;
		QListWidget *collectionsList;
				
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
