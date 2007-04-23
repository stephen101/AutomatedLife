
#include <iostream>
#include <iterator>
#include <string>
#include <vector>


#include "collection.h"


IndexingThread::IndexingThread(Graph *g, QStringList dirs, QObject *parent)
	: QThread(parent), m_graph(g) {
		m_directories = dirs;
		m_safeTerminate = false;
}

std::set<std::string> IndexingThread::load_stoplist(const std::string &filename){
    std::set<std::string> stoplist;
    if (try_loading_stoplist(STOPLIST_INSTALL_LOCATION, stoplist)) return stoplist;
    if (try_loading_stoplist(filename, stoplist)) return stoplist;
    
    std::cerr << "Could not load stop list from either '" << filename << "' or '" << STOPLIST_INSTALL_LOCATION << "'!" << std::endl;
	return stoplist;
}

bool IndexingThread::try_loading_stoplist(const std::string &filename, std::set<string> &stoplist) {
    std::ifstream file;
    file.open(filename.c_str(), std::ios_base::in | std::ios_base::binary);
    if (file) {
        std::string line;
        while( std::getline( file, line ) ){
    		stoplist.insert( line );
    	}
    	file.clear();
    	file.close();
    	return true;
    }
    return false;
}


void IndexingThread::run(){
	/* emit */ startedIndexing();
	/* emit */ status(QString(tr("Collecting files")));
	std::vector<std::string> filenames;
	for( int i = 0; i < m_directories.count(); ++i ){
		file_finder f(m_directories.at(i).toStdString());
		f.add_file_ext("html");
		f.add_file_ext("htm");
		f.add_file_ext("rtf");
		f.add_file_ext("pdf");
		f.add_file_ext("doc");
		f.add_file_ext("txt");
		std::vector<std::string> files = f.get_filenames();
		for( unsigned j = 0; j < files.size(); ++j ){
			filenames.push_back(files[j]);
		}
	}
	if( filenames.size() == 0 ){
		/* emit */ finishedIndexing();
		return;
	}
	int i = 0;
	/* emit */ progress(i, filenames.size());
	/* emit */ status(QString(tr("Loading indexer")));
	text_indexer<Graph> indexer(*m_graph, ":data/lexicon.txt" );
 	std::set<std::string> blacklist = load_stoplist(":data/stoplist_en.txt");
	
	std::string max = m_graph->get_meta_value("max_phrase_length","3");
	std::string parser = m_graph->get_meta_value("parser", "nouns");
	indexer.add_word_filter(blacklist_filter(blacklist));
 	indexer.add_word_filter(too_many_numbers_filter(1));
 	indexer.add_word_filter(minimum_length_filter(3));
 	indexer.add_word_filter(maximum_word_length_filter(10));
 	indexer.add_word_filter(maximum_phrase_length_filter(atoi(max.c_str())));
//	indexer.set_default_encoding( encoding );
	indexer.set_pdf_layout("raw");
	indexer.set_parsing_method(parser);
	
	std::vector<std::string>::iterator pos;
	for( pos = filenames.begin(); pos != filenames.end(); ++pos ){
		std::string this_file = *pos;
		if( m_safeTerminate ){
			/* emit */ finishedIndexing();
			return;
		}
		try {
			QString statusMessage = QString(tr("Indexing ")) + QString::fromStdString(this_file);
			/* emit */ status( statusMessage );
			indexer.index( this_file );
			
		} catch (std::exception &e){
			QString statusMessage = QString(tr("Error indexing ")) + QString::fromStdString(this_file);
			/* emit */ status( statusMessage );
			std::cerr << "Error indexing file: " << this_file << " (" << e.what() << ")" << std::endl;
			continue;
		}
		/* emit */ progress(++i, filenames.size());
	}
	/* emit */ status( QString(tr("Storing Graph")));
	indexer.finish();
	
	/* emit */ finishedIndexing();
	
}

void IndexingThread::safeTerminate(){
	m_safeTerminate = true;
}



void CollectionWidget::startIndexing(){
	// hide certain parts of the widget!
	// clear and then show indexing output 
	// extract dirList from QListWidget
	
	QStringList dirList;
	indexingStatus->clear();
	m_indexingThread = new IndexingThread(m_graph, dirList);
    connect(m_indexingThread, SIGNAL(finishedIndexing()), this, SLOT(finishIndexing()));
    connect(m_indexingThread, SIGNAL(progress(int, int)), this, SLOT(updateProgress(int, int)));
	connect(m_indexingThread, SIGNAL(status(QString)), this, SLOT(updateStatus(QString)));
	m_indexingThread->start();
	isIndexing = true;
	
}

void CollectionWidget::updateProgress(int s, int total){
	if (indexingProgress->maximum() != total) indexingProgress->setMaximum(total);
    indexingProgress->setValue(s);	
}

void CollectionWidget::updateStatus(QString message){
	indexingStatus->addItem(message);
	std::cerr << message.toStdString() << std::endl;
}

void CollectionWidget::finishIndexing(){
	// hide indexing output
	// show standard widget parts
	isIndexing = false;
}


CollectionWidget::CollectionWidget(Graph *g, QWidget *parent)
	: QWidget(parent), m_graph(g)
{
	setWindowTitle(tr("Manage Document Collections"));
	setMinimumSize(200,200);
	setupCollectionData();
	
	setupLayout();
	setupConnections();
}





void CollectionWidget::saveCollectionData(){
	renameCollection(collectionTitle->text());
}

void CollectionWidget::renameCollection(QString newName){
	if( newName.length() > 0 && m_graph->collection() != newName.toStdString() ){
		m_graph->rename_collection(m_graph->collection(), newName.toStdString());
		m_graph->set_collection(newName.toStdString());
		/* emit */ collectionTitleChanged();
	}
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

		/* emit */ collectionTitleChanged();
	}							
}


void CollectionWidget::setupCollectionData(){
	
	collectionData.clear();
	collectionData.insert(CollectionWidget::DocumentCount, (double)m_graph->get_vertex_count_of_type(node_type_major_doc));
	collectionData.insert(CollectionWidget::TermCount, (double)m_graph->get_vertex_count_of_type(node_type_major_term));
	collectionData.insert(CollectionWidget::Title, QString::fromStdString(m_graph->collection()));
	collectionData.insert(CollectionWidget::Parser, QString::fromStdString(m_graph->get_meta_value("parser","Nouns")));
	collectionData.insert(CollectionWidget::Locations, QString::fromStdString(m_graph->get_meta_value("locations","")));
}







void CollectionWidget::setWindowFlags(Qt::WindowFlags flags){
	QWidget::setWindowFlags(flags);	
}



void CollectionWidget::setupLayout(){
	
	isIndexing = false;
	editOpen = false;
	closeShortcut = new QShortcut(QKeySequence(tr("Esc", "Close")), this);
	
	// indexing
	indexingProgress = new QProgressBar;
	indexingStatus = new QListWidget;
	QHBoxLayout *indexingLayout = new QHBoxLayout;
	indexingLayout->setAlignment(Qt::AlignTop);
	indexingLayout->addWidget(indexingStatus);
	indexingLayout->addWidget(indexingProgress);
	
	
	
	// collections
	QGroupBox *collectionBox = new QGroupBox(tr("Collection Settings"));
	collectionTitle = new QLineEdit(QString::fromStdString(m_graph->collection()));

	
	documentCount = new QLabel(collectionData[CollectionWidget::DocumentCount].toString());
	documentCount->setToolTip(tr("The number of documents in the collection"));
	
	termCount = new QLabel(collectionData[CollectionWidget::TermCount].toString());
	termCount->setToolTip(tr("The number of terms included in the collection"));
	parserType = new QLabel(collectionData[CollectionWidget::Parser].toString());
	parserType->setToolTip(tr("The grammatical class of words used to create the term index"));
	
	QGridLayout *settingsGridLayout = new QGridLayout;
	settingsGridLayout->addWidget(new QLabel(tr("Title: ")),0,0);
	settingsGridLayout->addWidget(new QLabel(tr("Documents: ")),1,0);
	settingsGridLayout->addWidget(new QLabel(tr("Terms: ")),2,0);
	settingsGridLayout->addWidget(new QLabel(tr("Parser: ")),3,0);
	settingsGridLayout->addWidget(collectionTitle,0,1);
	settingsGridLayout->addWidget(documentCount,1,1);
	settingsGridLayout->addWidget(termCount,2,1);
	settingsGridLayout->addWidget(parserType,3,1);
	settingsGridLayout->setSpacing(3);
	settingsGridLayout->setAlignment(Qt::AlignTop);
	
	
	
	QVBoxLayout *collectionsLayout = new QVBoxLayout;
	collectionsLayout->addLayout(settingsGridLayout);
	collectionsLayout->setAlignment(Qt::AlignTop);
	collectionBox->setLayout(collectionsLayout);
	

	// directories
	QGroupBox *directoriesBox = new QGroupBox(tr("Directory Locations"));
	directoriesBox->setToolTip(tr("The directories included in this collection"));
	
	QHBoxLayout *directoryToolsLayout = new QHBoxLayout;
	directoryListWidget = new QListWidget;
	QString locations(collectionData[CollectionWidget::Locations].toString());
	if( locations.length()){
		QStringList directoryLocations = locations.split(", ");
		for( int i = 0; i < directoryLocations.size(); ++i ){
			directoryListWidget->addItem(directoryLocations.at(i));
		}
	} else {
		directoryListWidget->addItem(QString(tr("(Unknown)")));
		
	}
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
	
	if( directoryListWidget->count() == 1 && directoryListWidget->item(0)->text() == QString(tr("(Unknown)"))){
		directoryListWidget->clear();
	}
	if( !directoryListWidget->findItems(directory, Qt::MatchExactly).count() ){
		directoryListWidget->addItem(directory);	
	}
}

void CollectionWidget::setupConnections(){
	connect(closeButton,SIGNAL(clicked()), this, SLOT(closeWindow()));
	connect(addDocumentsButton, SIGNAL(clicked()), this, SLOT(getFilePaths()));
	connect(removeDocumentsButton, SIGNAL(clicked()), this, SLOT(removeFiles()));
	connect(closeShortcut, SIGNAL(activated()),this, SLOT(escCloseWindow()));

}

void CollectionWidget::escCloseWindow(){
	if( !isIndexing ){
		closeWindow();
	}
}

void CollectionWidget::removeFiles(){
	QList<QListWidgetItem *> selected = directoryListWidget->selectedItems();
	for( int i = 0; i < selected.size(); ++i ){
		directoryListWidget->takeItem(directoryListWidget->row(selected.at(i)));
	}
	
	QStringList dirList;
	for( int i = 0; i < directoryListWidget->count(); ++i ){
		dirList << directoryListWidget->item(i)->text();
	}
	collectionData[CollectionWidget::Locations] = dirList.join(", ");
}

void CollectionWidget::closeWindow(){
	if( collectionTitle->text().length() == 0 ){
		collectionTitle->setText(collectionData[CollectionWidget::Title].toString());
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
			renameCollection(newTitle);
		}
	}
	editOpen = false;
}




