#include <iostream>

#include "mainwindow.h"
#include "locate_datafile.h"
#include <semantic/version.hpp>

 MainWindow::MainWindow()
 {
	 QWidget::setWindowIcon(QIcon(QPixmap(":/icons/nse.png")));
     QWidget::setWindowTitle(tr("The Semantic Engine"));
	 QApplication::setStartDragTime(500);
	 QApplication::setStartDragDistance(8);
	
	 QString dataFile = DatafileLocator::getDatafileLocation();
	 searchEngine = new SearchWidget(dataFile, this);
	 
	 setCentralWidget(searchEngine);

     createActions();
     createMenus();
    
     readSettings();
 }

void MainWindow::openIndex(){
	
	QString dataFile = QFileDialog::getOpenFileName(this, tr("Open Semantic Index"));

	searchEngine->close();
	searchEngine = new SearchWidget(dataFile, this);
	setCentralWidget(searchEngine);
}

void MainWindow::copyIndex(){
	QString dataFile = QFileDialog::getSaveFileName(this, tr("Export Semantic Index"));
	searchEngine->copyDataFile(dataFile);

}


 void MainWindow::about()
 {
	QString text(tr("The Semantic Engine is an attempt to improve on keyword searches.\n\n"));
	text += QString(tr("Version ")) + QString(SEMANTIC_VERSION_STRING);

    QMessageBox::about(this,tr("Semantic Engine"),text);
 }


   
 void MainWindow::createActions()
 {

     //openAct = new QAction(tr("&Open..."), this);
     //openAct->setShortcut(tr("Ctrl+O"));
     //openAct->setStatusTip(tr("Open an existing index file"));
     //connect(openAct, SIGNAL(triggered()), this, SLOT(openIndex()) );

     //exportAct = new QAction(tr("&Export..."), this);
     //exportAct->setShortcut(tr("Ctrl+E"));
     //exportAct->setStatusTip(tr("Save this semantic index"));
     //connect(exportAct, SIGNAL(triggered()), this, SLOT(copyIndex()) );

	 closeAct = new QAction(tr("&Close"), this);
	 closeAct->setShortcut(tr("Ctrl+W"));
	 closeAct->setStatusTip(tr("Close the window"));
	 connect(closeAct, SIGNAL(triggered()), this, SLOT(close()));

     exitAct = new QAction(tr("E&xit"), this);
     exitAct->setShortcut(tr("Ctrl+Q"));
     exitAct->setStatusTip(tr("Exit the application"));
     connect(exitAct, SIGNAL(triggered()), this, SLOT(close()));

     cutAct = new QAction(tr("Cu&t"), this);
     cutAct->setShortcut(tr("Ctrl+X"));
     cutAct->setStatusTip(tr("Cut the current selection's contents to the "
                             "clipboard"));
	 
     copyAct = new QAction(tr("&Copy"), this);
     copyAct->setShortcut(tr("Ctrl+C"));
     copyAct->setStatusTip(tr("Copy the current selection's contents to the "
                              "clipboard"));
	 
     pasteAct = new QAction(tr("&Paste"), this);
     pasteAct->setShortcut(tr("Ctrl+V"));
     pasteAct->setStatusTip(tr("Paste the clipboard's contents into the current "
                               "selection"));
     connect(pasteAct, SIGNAL(triggered()), searchEngine, SLOT(paste()));

     aboutAct = new QAction(tr("&About"), this);
     aboutAct->setStatusTip(tr("Show the application's About box"));
     connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));
     
	minimizeAct = new QAction(tr("&Minimize"), this);
	minimizeAct->setShortcut(tr("Ctrl+M"));
	connect(minimizeAct, SIGNAL(triggered()), this, SLOT(minimize()));
	
	indexAct = new QAction(tr("&Collection Manager"), this);
	indexAct->setStatusTip(tr("Show the collection manager"));
	indexAct->setShortcut(tr("Ctrl+I"));
	connect(indexAct, SIGNAL(triggered()), searchEngine, SLOT(showCollectionWidget()));


 
}

void MainWindow::closeTopmostWindow(){
	QWidget *w = QApplication::activeWindow();
	if(w) w->close();
}

void MainWindow::minimize(){
	this->setWindowState(Qt::WindowMinimized);
}

 void MainWindow::createMenus()
 {
     fileMenu = menuBar()->addMenu(tr("&File"));
     //fileMenu->addAction(openAct);
	 //fileMenu->addAction(exportAct);
     fileMenu->addSeparator();
	 fileMenu->addAction(closeAct);
     fileMenu->addAction(exitAct);

     editMenu = menuBar()->addMenu(tr("&Edit"));
     editMenu->addAction(cutAct);
     editMenu->addAction(copyAct);
     editMenu->addAction(pasteAct);
	 
	windowMenu = menuBar()->addMenu(tr("&Window"));
	windowMenu->addAction(minimizeAct);
	windowMenu->addSeparator();
	windowMenu->addAction(indexAct);
	

     helpMenu = menuBar()->addMenu(tr("&Help"));
     helpMenu->addAction(aboutAct);
 }



 void MainWindow::createStatusBar()
 {
     statusBar()->showMessage(tr("Ready"));
 }

 void MainWindow::readSettings()
 {
	
	QSettings s;
	s.beginGroup("search_window");
	resize(s.value("size", QSize(600,355)).toSize());
	move(s.value("position", QPoint(100,100)).toPoint());
	s.endGroup();     
 }



void MainWindow::closeEvent(QCloseEvent *) {
	//searchEngine->closeResultWindow();
	QSettings s;
    s.beginGroup("search_window");
    s.setValue("size", size());
    s.setValue("position", pos());
    s.endGroup();
	qApp->quit();
}



 

 
