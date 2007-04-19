#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui>

#include "display.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();
 	void closeEvent(QCloseEvent *);
  
public slots:
 	void about();
	void closeTopmostWindow();
	void minimize();
	void openIndex();
	void copyIndex();
	
private:
    void createActions();
    void createMenus();
    void createStatusBar();
    void readSettings();
    void writeSettings();

     
    SearchWidget *searchEngine;
     
    QMenu *fileMenu;
    QMenu *editMenu;
    QMenu *helpMenu;
	QMenu *windowMenu;
	QAction *openAct;
    QAction *exportAct;
    QAction *closeAct;
    QAction *exitAct;
    QAction *cutAct;
    QAction *copyAct;
    QAction *pasteAct;
    QAction *aboutAct;
	QAction *helpAct;
	QAction *indexAct;
	QAction *minimizeAct;
	
     
};

#endif
