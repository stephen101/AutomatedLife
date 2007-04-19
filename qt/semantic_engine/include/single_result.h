#ifndef SINGLE_RESULT_H
#define SINGLE_RESULT_H

#include <QtGui>

class SingleResultWindow : public QWidget {
	
	Q_OBJECT
	
public:
	SingleResultWindow(QString &, QString &title="Document Viewer", QWidget *parent=0);

public slots:
	void minimize();

private:
	void setupLayout();

	QTextEdit *textViewer;

};

#endif

