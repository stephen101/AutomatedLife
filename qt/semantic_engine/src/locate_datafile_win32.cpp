#include "locate_datafile.h"

// Qt includes
#include <QDir>

QString DatafileLocator::getDatafileLocation() {
	// get the ENV variable $HOME
	QDir d = QDir::home();
	
	d.mkdir("Semantic Engine");
	d.cd("Semantic Engine");
	
	return d.path() + QString("/index.db");
}
