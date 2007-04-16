
#include "tabbed_display.h"

SearchStatus::SearchStatus()
{
	setText("Searching ...");
//	setPalette(QPalette(QColor(255, 56, 255)));
//    setAutoFillBackground(true);
}

void SearchStatus::setStatus(QString status){
	setText(status);
}
