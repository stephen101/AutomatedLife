#ifndef BASKET_H
#define BASKET_H

#include <QWidget>
#include <QListView>
#include <QKeyEvent>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QListWidget>
#include <QStringList>

class BasketWidget : public QListWidget
{
	Q_OBJECT
	
	public:
		BasketWidget(QWidget *parent = 0);
		QStringList getSavedItems();
		
	public slots:
		void clearAll();
		
	protected:
		void keyPressEvent(QKeyEvent *);
	    void dropEvent(QDropEvent *);
		void dragEnterEvent(QDragEnterEvent *);
		void dragMoveEvent(QDragMoveEvent *);
	
	private:
		QMap<QString,int> saved_items;
};

#endif

