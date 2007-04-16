#ifndef TABBED_DISPLAY_H
#define TABBED_DISPLAY_H

#include <QLabel>

class SearchStatus : public QLabel
{
	Q_OBJECT
	
	public:
		SearchStatus();
		void setStatus(QString);

};

#endif
