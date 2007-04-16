#include "single_result.h"

// Qt
#include <QTextEdit>
#include <QVBoxLayout>
#include <QSize>
#include <QMenu>
#include <QAction>
#include <QMenuBar>
#include <QApplication>
#include <QShortcut>


SingleResultWindow::SingleResultWindow(QString &content, QString &title, QWidget *parent) : QWidget(parent) {
	
	
	this->setAttribute(Qt::WA_DeleteOnClose);
	this->setWindowTitle(title);

	setupLayout();

			

	if (!content.contains(QRegExp("<[a-zA-Z]+>"))) {
	    content = content.replace("\n\n", "<p>").replace("\n","<br/>");
	}

	/*	
	// highlighting
	std::multimap<double, 
	              std::string,
	              std::greater<double> > terms = result.all_results->top_terms_highlighting;
    std::multimap<double, 
	              std::string,
	              std::greater<double> >::iterator it = terms.begin();
	for(int i = 10; i > 0 && it != terms.end(); i--, ++it) {
	    QString qt_term = QString::fromStdString(it->second);
	    if (qt_term.endsWith('i'), Qt::CaseInsensitive) qt_term.remove(qt_term.length()-1,1);
	    content = content.replace(
	        QRegExp(QString("\\b(%1\\w*)\\b").arg(qt_term), Qt::CaseInsensitive), 
	        QString("<span style='color: rgb(%1, 0, 0);'><b>\\1</b></span>").arg(static_cast<int>(static_cast<double>(i)/10*255)));
	}	
	*/
	
	textViewer->insertHtml(content);
	textViewer->moveCursor(QTextCursor::Start);
}

void SingleResultWindow::minimize(){
	this->setWindowState(Qt::WindowMinimized);
}

void SingleResultWindow::setupLayout(){
	QVBoxLayout *layout = new QVBoxLayout();
	textViewer = new QTextEdit();
	textViewer->setReadOnly(true);
	
	layout->addWidget(textViewer);
	setLayout(layout);
	
	resize(QSize(500, 600));

	QShortcut *closeMe = new QShortcut(QKeySequence(tr("Ctrl+W", "Close")), this);
	QShortcut *minimizeMe = new QShortcut(QKeySequence(tr("Ctrl+M", "Minimize")), this);

	connect(closeMe, SIGNAL(activated()), this, SLOT(close()));
	connect(minimizeMe, SIGNAL(activated()), this, SLOT(minimize()));

}




