#include "locate_datafile.h"

// Qt includes
#include <QDir>

QString DatafileLocator::getDatafileLocation() {
	// get the ENV variable $HOME
	QDir d = QDir::home();
	
	d.mkdir(".semantic_engine");
	d.cd(".semantic_engine");
	
	return d.path() + QString("/index.db");
}
