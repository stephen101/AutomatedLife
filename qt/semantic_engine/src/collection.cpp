
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




void IndexingThread::run(){
	/* emit */ startedIndexing();
	/* emit */ status(QString(tr("Clearing index")));
	m_graph->reset_all_collections();
	
	
	std::vector<std::string> filenames;
	for( int i = 0; i < m_directories.count(); ++i ){
		//std::cerr << m_directories.at(i).toStdString() << std::endl;
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
		/* emit */ status(QString(tr("Done")));
		/* emit */ finishedIndexing();
		return;
	}
	int i = 0;
	/* emit */ progress(i, filenames.size());
	QString indexingMessage = tr("Indexing ") + QVariant((int)filenames.size()).toString() + " files";
	/* emit */ status(indexingMessage);
	
	// blacklist
	QFile blacklistFile(":data/stoplist_en.txt");
	blacklistFile.open(QIODevice::ReadOnly);
	QTextStream blacklistStream(&blacklistFile);
	std::set<std::string> blacklist;
	while(true) {
		QString str = blacklistStream.readLine();
		if (str.isNull()) break;
		blacklist.insert(str.toStdString());
	}
	
	
	m_graph->set_mirror_changes_to_storage(true);
	m_graph->set_meta_value("locations", m_directories.join(", ").toStdString());
	m_graph->set_meta_value("max_phrase_length", "3");
	m_graph->set_meta_value("parser", "nouns");
	m_graph->set_meta_value("min", "3"); // collection_minimum
	m_graph->set_mirror_changes_to_storage(false);
	m_graph->set_mirror_changes_to_storage(true);
	
	semantic::text_indexer<Graph> indexer(*m_graph, ":data/lexicon.txt" );
	std::string max = m_graph->get_meta_value("max_phrase_length","3");
	std::string parser = m_graph->get_meta_value("parser", "nouns");
	indexer.store_text(false);
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
			/* emit */ //status( statusMessage );
			indexer.index( this_file );
			//msleep(300);
			
		} catch (std::exception &e){
			QString statusMessage = QString(tr("Error indexing ")) + QString::fromStdString(this_file);
			/* emit */ status( statusMessage );
			std::cerr << "Error indexing file: " << this_file << " (" << e.what() << ")" << std::endl;
			continue;
		}
		/* emit */ progress(++i, filenames.size());
	}
	/* emit */ status( QString(tr("Storing Semantic Index")));

	try {
        indexer.commit_changes_to_storage();
    } catch ( std::exception &e){
        std::cerr << "Error Indexing to Database: " << e.what() << std::endl;
    }

	/* emit */ status(QString(tr("Storing Terms")));
	std::map<std::string,
              std::pair<std::string,int>
        > my_wordlist = indexer.prune_wordlist(2);

	i = 0;
	int term_count = m_graph->get_vertex_count_of_type(node_type_major_term);
	BGL_FORALL_VERTICES(u, *m_graph, Graph) {
		if ((*m_graph)[u].type_major == node_type_major_term ){
			i++;
			if( my_wordlist.count((*m_graph)[u].content)){
				std::pair<std::string, int> wordpair = my_wordlist[(*m_graph)[u].content];
				std::ostringstream oss;
				oss << wordpair.second;
				std::string word = wordpair.first+":"+oss.str();
				m_graph->set_vertex_meta_value(u, "term", word);
				if( !(i % 20) ){
					/* emit */ progress(i, term_count);
				}
			}
		}
	}
    
		
	
	/* emit */ status(QString(tr("Storing Text")));
	file_reader reader;

    reader.set_pdfLayout( "layout" );
	i = 0;
	int doc_count = m_graph->get_vertex_count_of_type(node_type_major_doc);
	BGL_FORALL_VERTICES(u, *m_graph, Graph) {
		if ((*m_graph)[u].type_major == node_type_major_doc ){
	        std::string text = reader( (*m_graph)[u].content );
            m_graph->set_vertex_meta_value(u, "body", text);
			/* emit */ progress(++i, doc_count);
		}
	}
	
	
	m_graph->set_mirror_changes_to_storage(false);
	/* emit */ status( QString(tr("Done")));
	/* emit */ finishedIndexing();
	
}

void IndexingThread::safeTerminate(){
	m_safeTerminate = true;
}



void CollectionWidget::startIndexing(){
	//dataLayout->setEnabled(false);
	indexingStatus->clear();
	indexingBox->setVisible(true);
	directoriesBox->setVisible(false);
	
	isIndexing = true;
	closeButton->setEnabled(false);
	indexButton->setEnabled(false);
	QStringList directoryList;
	for( int i = 0; i < directoryListWidget->count(); ++i ){
		QListWidgetItem *item = directoryListWidget->item(i);
		directoryList << item->text();
	}
	m_indexingThread = new IndexingThread(m_graph, directoryList);
    connect(m_indexingThread, SIGNAL(finishedIndexing()), this, SLOT(finishIndexing()));
    connect(m_indexingThread, SIGNAL(progress(int, int)), this, SLOT(updateProgress(int, int)));
	connect(m_indexingThread, SIGNAL(status(QString)), this, SLOT(updateStatus(QString)));
	m_indexingThread->start();
	
}

void CollectionWidget::updateProgress(int s, int total){
	if (indexingProgress->maximum() != total) indexingProgress->setMaximum(total);
    indexingProgress->setValue(s);	
}

void CollectionWidget::updateStatus(QString message){
	indexingStatus->addItem(message);
	indexingStatus->scrollToBottom();
	std::cerr << message.toStdString() << std::endl;
}

void CollectionWidget::finishIndexing(){
	indexButton->setVisible(false);
	indexingBox->setVisible(false);
	directoriesBox->setVisible(true);
	closeButton->setEnabled(true);
	indexButton->setEnabled(true);
	collectionTitle->setFocus();
	
	isIndexing = false;
	
	// reload the collection data
	getCollectionData();
	documentCount->setText(collectionData[CollectionWidget::DocumentCount].toString());
	termCount->setText(collectionData[CollectionWidget::TermCount].toString());
	QString parserString = collectionData[CollectionWidget::Parser].toString();
	parserString.replace(0,1,parserString[0].toUpper());
	parserType->setText(parserString);
	
	/* emit */ indexingCompleted();
	
}


CollectionWidget::CollectionWidget(Graph *g, QWidget *parent)
	: QWidget(parent), m_graph(g)
{
	setWindowTitle(tr("Manage Document Collections"));
	setMinimumSize(200,200);
	getCollectionData();
	
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



void CollectionWidget::getCollectionData(){
	
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
	closeShortcut = new QShortcut(QKeySequence(tr("Esc", "Close")), this);
	
	// indexing
	indexingProgress = new QProgressBar;
	indexingStatus = new QListWidget;
	
	QVBoxLayout *indexingLayout = new QVBoxLayout;
	indexingLayout->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
	indexingLayout->addWidget(indexingStatus);
	indexingLayout->addWidget(indexingProgress);
	
	indexingBox = new QGroupBox(tr("Indexing Status"));
	indexingBox->setLayout(indexingLayout);
	indexingBox->setVisible(false);
	
	// collections
	QGroupBox *collectionBox = new QGroupBox(tr("Collection Settings"));
	collectionTitle = new QLineEdit(QString::fromStdString(m_graph->collection()));

	
	documentCount = new QLabel(collectionData[CollectionWidget::DocumentCount].toString());
	documentCount->setToolTip(tr("The number of documents in the collection"));
	
	termCount = new QLabel(collectionData[CollectionWidget::TermCount].toString());
	termCount->setToolTip(tr("The number of terms included in the collection"));
	QString parserString = collectionData[CollectionWidget::Parser].toString();
	parserString.replace(0,1,parserString[0].toUpper());
	parserType = new QLabel(parserString);
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
	directoriesBox = new QGroupBox(tr("Document Locations"));
	directoriesBox->setToolTip(tr("The document locations included in this collection"));
	
	QHBoxLayout *directoryToolsLayout = new QHBoxLayout;
	directoryListWidget = new QListWidget;
	directoryListWidget->setTextElideMode(Qt::ElideMiddle);
	directoryListWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	
	QString locations(collectionData[CollectionWidget::Locations].toString());
	if( locations.length()){
		QStringList directoryLocations = locations.split(", ");
		for( int i = 0; i < directoryLocations.size(); ++i ){
			directoryListWidget->addItem(directoryLocations.at(i));
		}
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
	indexButton->setVisible(false);
	//indexButton->setEnabled(false);
	
	QHBoxLayout *toolLayout = new QHBoxLayout;
	toolLayout->addWidget(closeButton);
	toolLayout->addWidget(indexButton);
	toolLayout->setAlignment(Qt::AlignRight);
	
	QHBoxLayout *dataLayout = new QHBoxLayout;
	dataLayout->addWidget(collectionBox);
	dataLayout->addLayout(allSettingsLayout);
	dataLayout->addWidget(indexingBox);
	
	QVBoxLayout *layout = new QVBoxLayout;
	layout->addLayout(dataLayout);
	layout->addLayout(toolLayout);
	setLayout(layout);	

}


void CollectionWidget::getFilePaths(){
	QString directory = QFileDialog::getExistingDirectory(
							this,
							QString("Select a directory to add to the collection"));
	
	if( !directoryListWidget->findItems(directory, Qt::MatchExactly).count() ){
		directoryListWidget->addItem(directory);
		indexButton->setVisible(true);
	}
}

void CollectionWidget::setupConnections(){
	connect(closeButton,SIGNAL(clicked()), this, SLOT(closeWindow()));
	connect(indexButton,SIGNAL(clicked()), this, SLOT(startIndexing()));
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
		indexButton->setVisible(true);
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







