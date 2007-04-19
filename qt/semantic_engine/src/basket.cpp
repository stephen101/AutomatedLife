

#include "basket.h"

#include <iostream>

BasketWidget::BasketWidget(QWidget *parent)
	: QListWidget(parent)
{
	setDropIndicatorShown(true);
	setDragDropMode(QAbstractItemView::DropOnly);
	setSelectionMode(QAbstractItemView::ExtendedSelection);
	setAlternatingRowColors(true);
	setTextElideMode(Qt::ElideMiddle);
	setMaximumWidth(200);
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

QStringList BasketWidget::getSavedItems(){
	QList<QString> keys = saved_items.keys();
	return QStringList(keys);
}


void BasketWidget::dragEnterEvent(QDragEnterEvent *event)
{
	if( event->mimeData()->hasHtml() ){
		event->accept();
	} 
}

void BasketWidget::dragMoveEvent(QDragMoveEvent *event){
	if( event->mimeData()->hasHtml() ){
		event->acceptProposedAction();
	}
	
}

void BasketWidget::clearAll(){
	saved_items.clear();
	this->clear();
}

void BasketWidget::keyPressEvent(QKeyEvent *event){

	QList<QListWidgetItem *> items = this->selectedItems();
	
	switch( event->key() ){
		case Qt::Key_Delete:
		case Qt::Key_Backspace:
			for( int i = 0; i < items.count(); ++i ){
				QListWidgetItem *item = items.at(i);
				saved_items.remove(item->text());
				this->takeItem(this->row(item));
			}	
			
			break;
		
		default:
			QAbstractItemView::keyPressEvent(event);
			break;
	
	}
}

void BasketWidget::dropEvent(QDropEvent *event)
{
	if( event->mimeData()->hasHtml()){
		QString title = event->mimeData()->html();
		if( !saved_items.count(title) ){
			event->accept();
			this->addItem(title);
			saved_items.insert(title,1);
		}
	}
}



