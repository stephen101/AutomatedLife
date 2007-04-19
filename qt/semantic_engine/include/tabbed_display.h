#ifndef TABBED_DISPLAY_H
#define TABBED_DISPLAY_H

#include <QtGui>

class SearchStatus : public QLabel
{
	Q_OBJECT
	
	public:
		SearchStatus();
		void setStatus(QString);

};

#endif
