#ifndef _VISUAL_GL_DISPLAY_H_
#define _VISUAL_GL_DISPLAY_H_

// Qt
#include <QtGui>
#include <QtOpenGL>

// local
#include "se_graph.h"

// semantic
#include <semantic/analysis/linlog.hpp>

// some typedefs
typedef LinLog::point point;
typedef GraphTraits::vertex_descriptor graph_vertex;

enum graph_display_options {
	SHOW_DOC_NODES = 1,
	SHOW_TERM_NODES = 2,
	SHOW_EDGES = 4,
	SIZE_UNIFORM = 8,
	SIZE_RANK = 16,
	SIZE_DEGREE = 32
};

class VisualGLDisplay : public QGLWidget
{
	Q_OBJECT
	
public:
	VisualGLDisplay (std::string, QWidget *parent = 0);

	void reset();
	void reset(Graph *, search<Graph> *, WeightingTraits::edge_weight_map);
	
	void draw(PositionMap);
	
	void mouseMoveEvent(QMouseEvent *);
	void mousePressEvent(QMouseEvent *);
	void mouseDoubleClickEvent(QMouseEvent *);
	
	QSize minimumSizeHint() const;
	QSize sizeHint() const;
	
	void paintGL();
	void initializeGL();
	void resizeGL(int,int);
	
	void setOptions(int);

signals:
	void termDoubleClicked(QString &);
	void documentDoubleClicked(QString &);
	
private:
	void mkCircle(GLfloat, GLfloat, GLfloat, int = 5);
	void mkFilledCircle(GLfloat x, GLfloat y, GLfloat size, int seg = 5);
	void radialVertices(GLfloat x, GLfloat y, GLfloat size, int seg, bool includeLast);
	void drawVertex(graph_vertex u, GLfloat x, GLfloat y, GLfloat r, GLfloat g, GLfloat b, GLfloat size);
	void drawLabels();
	void moveLabels();
	void drawFilledRect(GLfloat x, GLfloat y, GLfloat width, GLfloat height);
	
	bool shouldDrawVertex(graph_vertex) const;
	float getVertexSize(graph_vertex);
	
	point screenToLocal(const point &, bool = false);

	Graph			*m_graph;
	PositionMap	m_position;
	std::map<graph_vertex,
		std::pair<point, point>
		>				m_label_info;
	std::map<graph_vertex,
		std::list<point> >
						m_label_moves;
	PositionMap         m_translated_position;
	point				m_mouse_pos, m_min, m_max, m_dimensions;
	bool				m_mouse_in;
	
	std::vector<std::pair<graph_vertex, point> >
						m_labeled_vertices;
	bool				m_have_highlighted_node;
	graph_vertex  			m_highlighted_node;
	
	int                 m_options;
	double              m_max_degree, m_max_weight;
	maps::unordered<
	    graph_vertex,
	    double>         m_vertex_degrees;
	
	WeightingTraits::edge_weight_map    m_normalized_weights;
	bool isSearching;
	search<Graph> *m_searchEngine;        
	
};

#endif // _VISUAL_GL_DISPLAY_HPP_
