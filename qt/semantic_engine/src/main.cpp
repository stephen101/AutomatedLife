
#include <iostream>

#include <QApplication>
#include <QSettings>

#include "mainwindow.h"
#include <iostream>


int main( int argc, char *argv[])
{
    QApplication app(argc,argv);
    QCoreApplication::setOrganizationName("Mellon");
    QCoreApplication::setOrganizationDomain("knowledgesearch.org");
    QCoreApplication::setApplicationName("Semantic Engine");
	
	MainWindow window;
	window.show();
    return app.exec();
}