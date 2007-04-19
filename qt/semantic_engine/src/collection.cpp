#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QListWidget>
#include <QLabel>
#include <QPushButton>
#include <QToolButton>
#include <QFileDialog>
#include <QMessageBox>

#include <iostream>
#include <iterator>
#include <string>
#include <vector>

#include "mainwindow.h"
#include "collection.h"


void CollectionWidget::saveCollectionData(){
	/* 
		stub for semantic code -- iterate through collectionList, save
		Locations, Parser
	*/
}

void CollectionWidget::renameCollection(QString previous, QString current){


	m_graph->rename_collection(previous.toStdString(), current.toStdString());
	
	/* emit */ collectionListChanged();

}

void CollectionWidget::removeCollection(){
	int ret = QMessageBox::question(this, tr("Remove Collection"), 
								tr("Are you sure that you want to remove this collection?"),
								QMessageBox::Yes, QMessageBox::Cancel);
	if( ret == QMessageBox::Yes ){
		QListWidgetItem *item = collectionsList->takeItem(
										collectionsList->row(
											collectionsList->selectedItems()[0]));
		QString collectionTitle = item->data(CollectionWidget::Title).toString();
		m_graph->remove_collection( collectionTitle.toStdString() );

		/* emit */ collectionListChanged();
	
	}							
}


void CollectionWidget::setupCollectionData(){
	
	
	std::vector<std::string> collections;
	std::back_insert_iterator<std::vector<std::string> > iter(collections);
	m_graph->get_collections_list(iter);


	
		
		for(unsigned int i = 0; i < collections.size(); i++) {
			Graph g(collections[i]);
			g.set_file(m_graph->get_file());
			
			QListWidgetItem *item = new QListWidgetItem;
			item->setData(CollectionWidget::DocumentCount, (double)g.get_vertex_count_of_type(node_type_major_doc));
			item->setData(CollectionWidget::TermCount, (double)g.get_vertex_count_of_type(node_type_major_term));
			
			item->setData(CollectionWidget::Title, QString::fromStdString(collections[i]));
			item->setData(CollectionWidget::Parser, "Noun Phrases" );
			item->setData(CollectionWidget::Locations, "MySQL Database" );
			collectionsList->addItem(item);
		}
		
	
	
		
	for( int i = 0; i < collectionsList->count(); ++i ){
		collectionTitles.insert(collectionsList->item(i)->data(CollectionWidget::Title).toString());
	}	
}


CollectionWidget::CollectionWidget(Graph *g, QWidget *parent)
	: QWidget(parent), m_graph(g)
{
	setWindowTitle(tr("Manage Document Collections"));
	setMinimumSize(200,200);
	
	setupLayout();
	setupConnections();
	
	setupCollectionData();
	
	getCollectionData();

}

QStringList CollectionWidget::getCollectionList(){
	QStringList list;
	for( int i = 0; i < collectionsList->count(); ++i ){
		//QListWidgetItem *item = ;
		list << collectionsList->item(i)->data(CollectionWidget::Title).toString();
	}
	if(list.count() > 0){
		return list;
	} else {
		return QStringList(tr("Untitled Collection"));
	}
	
}

void CollectionWidget::getCollectionData(){


	
	if( collectionsList->currentRow() < 0 )
		collectionsList->setCurrentRow(0);
	
	
	QListWidgetItem *item = collectionsList->selectedItems()[0];
	
	documentCount->setText(item->data(CollectionWidget::DocumentCount).toString());
	termCount->setText(item->data(CollectionWidget::TermCount).toString());
	parserType->setText(item->data(CollectionWidget::Parser).toString());
	QStringList list = item->data(CollectionWidget::Locations).toString().split(", ");
	directoryListWidget->clear();
	directoryListWidget->addItems(list);
	if( item->data(CollectionWidget::DocumentCount).toInt() > 0 ){
		indexButton->setText(tr("Re-Index"));
	} else {
		indexButton->setText(tr("Index"));
	}

}

void CollectionWidget::setWindowFlags(Qt::WindowFlags flags){
	QWidget::setWindowFlags(flags);	
}



void CollectionWidget::setupLayout(){
	
	editOpen = false;
	
	// collections
	QGroupBox *collectionBox = new QGroupBox(tr("Document Collections"));
	collectionsList = new QListWidget(this);
	collectionsList->setSelectionMode(QAbstractItemView::SingleSelection);
	
	QHBoxLayout *collectionToolsLayout = new QHBoxLayout;
	addCollectionButton = new QToolButton;
	addCollectionButton->setText("+");
	addCollectionButton->setToolTip(tr("Add a new collection of documents"));
	removeCollectionButton = new QToolButton;
	removeCollectionButton->setText("-");
	removeCollectionButton->setToolTip(tr("Remove the selected collection"));
	collectionToolsLayout->setAlignment(Qt::AlignRight);
	collectionToolsLayout->addWidget(removeCollectionButton);
	collectionToolsLayout->addWidget(addCollectionButton);
	
	QVBoxLayout *collectionsLayout = new QVBoxLayout;
	collectionsLayout->addWidget(collectionsList);
	collectionsLayout->addLayout(collectionToolsLayout);
	collectionBox->setLayout(collectionsLayout);
	

	// settings	
	QGroupBox *settingsBox = new QGroupBox(tr("Settings"));
	
	documentCount = new QLabel;
	documentCount->setToolTip(tr("The number of documents in the collection"));
	
	termCount = new QLabel;
	termCount->setToolTip(tr("The number of terms included in the collection"));
	parserType = new QLabel;
	parserType->setToolTip(tr("The grammatical class of words used to create the term index"));
	
	QGridLayout *settingsGridLayout = new QGridLayout;
	settingsGridLayout->addWidget(new QLabel(tr("Documents")),0,0);
	settingsGridLayout->addWidget(new QLabel(tr("Terms")),1,0);
	settingsGridLayout->addWidget(new QLabel(tr("Parser")),2,0);
	settingsGridLayout->addWidget(documentCount,0,1);
	settingsGridLayout->addWidget(termCount,1,1);
	settingsGridLayout->addWidget(parserType,2,1);
	settingsGridLayout->setSpacing(2);
	
	settingsGridLayout->setAlignment(Qt::AlignTop);
	settingsBox->setLayout(settingsGridLayout);
	settingsBox->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Minimum);
	
	// directories
	QGroupBox *directoriesBox = new QGroupBox(tr("Directory Locations"));
	directoriesBox->setToolTip(tr("The directories included in this collection"));
	
	QHBoxLayout *directoryToolsLayout = new QHBoxLayout;
	directoryListWidget = new QListWidget;
	addDocumentsButton = new QToolButton;
	addDocumentsButton->setText("+");
	addDocumentsButton->setToolTip(tr("Add documents to this collection"));
	removeDocumentsButton = new QToolButton;
	removeDocumentsButton->setText("-");
	removeDocumentsButton->setToolTip(tr("Remove documents from the collection"));
	directoryToolsLayout->addWidget(removeDocumentsButton);
	directoryToolsLayout->addWidget(addDocumentsButton);
	directoryToolsLayout->setAlignment(Qt::AlignRight);

	QVBoxLayout *directoryLayout = new QVBoxLayout;
	directoryLayout->setAlignment(Qt::AlignTop);
	directoryLayout->addWidget(directoryListWidget);
	directoryLayout->addLayout(directoryToolsLayout);
	directoriesBox->setLayout(directoryLayout);
		
	QVBoxLayout *allSettingsLayout = new QVBoxLayout;
	allSettingsLayout->addWidget(settingsBox);
	allSettingsLayout->addWidget(directoriesBox);
	
	// tools
	closeButton = new QPushButton(tr("Close"));
	indexButton = new QPushButton(tr("Index"));
	indexButton->setToolTip(tr("Index or re-index the documents in the selected collection"));
	
	QHBoxLayout *toolLayout = new QHBoxLayout;
	toolLayout->addWidget(closeButton);
	toolLayout->addWidget(indexButton);
	toolLayout->setAlignment(Qt::AlignRight);
	
	QHBoxLayout *dataLayout = new QHBoxLayout;
	dataLayout->addWidget(collectionBox);
	dataLayout->addLayout(allSettingsLayout);
	
	QVBoxLayout *layout = new QVBoxLayout;
	layout->addLayout(dataLayout);
	layout->addLayout(toolLayout);
	setLayout(layout);	

}


void CollectionWidget::getFilePaths(){
	QString directory = QFileDialog::getExistingDirectory(
							this,
							QString("Select a directory to add to the collection"));
	
	QListWidgetItem *item = collectionsList->selectedItems()[0];
	item->setData(CollectionWidget::Locations, item->data(CollectionWidget::Locations).toString() + ", " + directory );
	
	directoryListWidget->addItem(directory);	
}

void CollectionWidget::setupConnections(){
	connect(collectionsList, SIGNAL(itemSelectionChanged()), this, SLOT(getCollectionData()));
	connect(closeButton,SIGNAL(clicked()), this, SLOT(closeWindow()));
	connect(addCollectionButton, SIGNAL(clicked()), this, SLOT(addCollection()));
	connect(collectionsList, SIGNAL(itemDoubleClicked(QListWidgetItem *)), this, SLOT(startEditItem(QListWidgetItem *)));
	connect(collectionsList, SIGNAL(currentItemChanged(QListWidgetItem *,QListWidgetItem *)), this, SLOT(endEditItem(QListWidgetItem *, QListWidgetItem *)));
	connect(collectionsList, SIGNAL(itemChanged(QListWidgetItem *)), this, SLOT(endEditItem(QListWidgetItem *)));
	connect(addDocumentsButton, SIGNAL(clicked()), this, SLOT(getFilePaths()));
	connect(removeDocumentsButton, SIGNAL(clicked()), this, SLOT(removeFiles()));
	connect(removeCollectionButton, SIGNAL(clicked()), this, SLOT(removeCollection()));
}

void CollectionWidget::removeFiles(){
	QListWidgetItem *item = directoryListWidget->selectedItems()[0];
	QListWidgetItem *collectionItem = collectionsList->selectedItems()[0];
	
	QStringList locations = collectionItem->data(CollectionWidget::Locations).toString().split(", ");
	QStringList newLocations;
	for( int i = 0; i < locations.size(); ++i ){
		if( item->data(Qt::DisplayRole).toString() != locations.at(i)){
			newLocations << locations.at(i);
		}
	}
	collectionItem->setData(CollectionWidget::Locations, newLocations.join(", "));
	directoryListWidget->takeItem(directoryListWidget->row(item));
}

void CollectionWidget::closeWindow(){
	for( int i = 0; i < collectionsList->count(); ++i ){
		if( collectionsList->item(i)->isSelected() ){
			collectionsList->closePersistentEditor(collectionsList->item(i));
		}
	}
	saveCollectionData();
	this->hide();
}



void CollectionWidget::startEditItem(QListWidgetItem *item){
	
	editOpen = true;
	editPrevTitle = item->data(CollectionWidget::Title).toString();

	collectionsList->openPersistentEditor(item);
	collectionsList->editItem(item);

}

void CollectionWidget::endEditItem( QListWidgetItem *, QListWidgetItem *previousItem){
	if( editOpen ){
		collectionsList->closePersistentEditor(previousItem);
		editOpen = false;
	}
}

void CollectionWidget::endEditItem( QListWidgetItem *item){
	if( editOpen ){
		
		QString newTitle = item->data(CollectionWidget::Title).toString();
		collectionTitles.remove(editPrevTitle);
		collectionsList->closePersistentEditor(item);
		
		if( collectionTitles.contains(newTitle)){
			item->setData(CollectionWidget::Title, editPrevTitle);
			std::cerr << "\tduplicate!" << std::endl;
		} else {
			renameCollection(editPrevTitle,newTitle);
		}
	}
	editOpen = false;
}



void CollectionWidget::addCollection(){
	QListWidgetItem *item = new QListWidgetItem;
	QString base = "Untitled Collection";
	QString title = base;
	int i = 1;
	while( collectionTitles.contains(title)){
		title = base + " (" + QVariant(++i).toString() + ")";
	}
	collectionTitles.insert(title);
	item->setData(CollectionWidget::Title, title);
	item->setData(CollectionWidget::Id, "0" );
	item->setData(CollectionWidget::DocumentCount, "0");
	item->setData(CollectionWidget::TermCount, "0" );
	item->setData(CollectionWidget::Parser, "Nouns" );
	collectionsList->addItem(item);
	collectionsList->setCurrentItem(item);

	/* emit */ collectionListChanged();
}


