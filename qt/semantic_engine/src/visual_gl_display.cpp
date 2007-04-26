#include "visual_gl_display.h"
#include "single_result.h"

#include <semantic/analysis/utility.hpp>
#include <math.h>

VisualGLDisplay::VisualGLDisplay(std::string, QWidget *parent) : QGLWidget(QGLFormat(QGL::SampleBuffers), parent), m_mouse_in(false), m_options(SHOW_TERM_NODES | SHOW_EDGES | SIZE_DEGREE) {
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	setMouseTracking(true);
	isSearching = false;
}

void VisualGLDisplay::reset() {
	m_position.clear();
	isSearching = false;
}

void VisualGLDisplay::reset(Graph *g, WeightingTraits::edge_weight_map weights) {
	reset();
	m_graph = g;
	
	double max = 0;
	WeightingTraits::edge_weight_map::iterator it;
	for(it = weights.begin(); it != weights.end(); ++it) {
	    if (it->second > max) max = it->second;
	}

    if (max != 0) {
    	for(it = weights.begin(); it != weights.end(); ++it) {
    	    it->second/=max;
    	}
	}
	
	m_normalized_weights = weights;
	m_max_weight = max;
	
	// compute max degree
	m_max_degree = 0;
	BGL_FORALL_VERTICES(u, *g, Graph) {
	    double cum = 0;
	    BGL_FORALL_OUTEDGES(u, e, *g, Graph) {
	        cum += weights[e];
	    }
	    
	    if (cum > m_max_degree) m_max_degree = cum;
	    m_vertex_degrees[u] = cum;
	}
}

void VisualGLDisplay::draw(PositionMap m) {
	m_position = m;
	repaint();
}

QSize VisualGLDisplay::minimumSizeHint() const {
	return QSize(100,100);
}

QSize VisualGLDisplay::sizeHint() const {
	return QSize(500,500);
}

void VisualGLDisplay::initializeGL() {
	//std::cout << "initialized OpenGL" << std::endl;
	// init our openGL methods
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glEnable(GL_LINE_SMOOTH);
	glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
	glEnable(GL_POINT_SMOOTH);
	
	// alpha colors
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
//	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	glEnable(GL_BLEND);
//	glShadeModel(GL_SMOOTH);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-5, 5, 5, -5, -1000, 1000);
	glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glClearColor(1.0, 1.0, 1.0, 1.0);
}

void VisualGLDisplay::resizeGL(int w, int h) {
	// std::cout << "resized OpenGL" << std::endl;
	glViewport(0, 0, w, h);
}

void VisualGLDisplay::paintGL() {
//	std::cerr << "painting OpenGL: " ;
	
	// compute our bounds and set up the GL view
	semantic::get_bounding_rect(extract_values(m_position.begin()), extract_values(m_position.end()), m_min, m_max);
	m_max *= 1.1; m_min *= 1.1; // expand a little so we have some visual space on the sides
	m_dimensions = m_max - m_min;
	float lens_radius = (m_max.x() - m_min.x())/12;
	float lens_radius2 = lens_radius * lens_radius;
	
	glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
	glOrtho(m_min.x(), m_max.x(), m_min.y(), m_max.y(), -1000, 1000);
	glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

	// clear our screen
	glClear(GL_COLOR_BUFFER_BIT);
	
	// if we don't have any points to draw, don't draw them.
	if (m_position.empty() or isSearching ) return;
	// or if we have no graph
	if (!m_graph) return;
	
	const int options = m_options;
		
	if (options & SHOW_EDGES) {
		glBegin(GL_LINES);
		GraphTraits::edge_iterator ei, ei_end;
		for(boost::tie(ei, ei_end) = edges(*m_graph); ei != ei_end; ++ei) {
			glColor4f(0.0,0.0,0.0,m_normalized_weights[*ei]*0.7);
			// start drawing lines
			graph_vertex u, v;
			u = source(*ei, *m_graph);
			v = target(*ei, *m_graph);
			point Psrc = m_position[u];
			point Ptgt = m_position[v];
			glVertex2f(Psrc.x(), Psrc.y());
			glVertex2f(Ptgt.x(), Ptgt.y());
		}
		glEnd();
	}
	
	
	glColor4f(0.0,0.3,0.1,0.3);

	// translate the mouse coordinate into local coordinates
	point mouse_pos = screenToLocal(m_mouse_pos, true);
	
//	mkFilledCircle(mouse_pos.x(), mouse_pos.y(), 40, 3); // <-- debug
	
	std::set<graph_vertex> old_labels(extract_keys(m_labeled_vertices.begin()), extract_keys(m_labeled_vertices.end()));
	m_labeled_vertices.clear();	
	
	m_have_highlighted_node = false;
	double smallest_highlighted_node = (std::numeric_limits<double>::max)();
	
	for(PositionMap::iterator i = m_position.begin(); i != m_position.end(); ++i) {
		if (shouldDrawVertex(i->first) && m_mouse_in && (square(i->second.x() - mouse_pos.x()) 
				+ square(i->second.y() - mouse_pos.y())) < lens_radius2 && m_labeled_vertices.size() < 20) {
			m_labeled_vertices.push_back(*i);
			
			// check if the mouse is actually inside a node
			double node_size;
			if (square(i->second.x() - mouse_pos.x()) + square(i->second.y() - mouse_pos.y()) < square(node_size = getVertexSize(i->first)/2)) {
				// ok so far so good
				if (node_size < smallest_highlighted_node) {
					m_have_highlighted_node = true;
					m_highlighted_node = i->first;
					smallest_highlighted_node = node_size;
				}
			}
		}
	}
	
	
	for(PositionMap::iterator i = m_position.begin(); i != m_position.end(); ++i) {
		if (m_have_highlighted_node && m_highlighted_node == i->first) continue; // draw the highlighted node last	

		if((*m_graph)[i->first].type_major == node_type_major_doc)
			drawVertex(i->first, i->second.x(), i->second.y(), 0.0, 0.1, 0.6, getVertexSize(i->first));
		else
			drawVertex(i->first, i->second.x(), i->second.y(), 0.0, 0.6, 0.1, getVertexSize(i->first));
	}
	
	if (m_have_highlighted_node) {
		graph_vertex u = m_highlighted_node;
		point p = m_position[u];
		if((*m_graph)[u].type_major == node_type_major_doc)
			drawVertex(u, p.x(), p.y(), 0.0, 0.1, 0.6, getVertexSize(u));
		else
			drawVertex(u, p.x(), p.y(), 0.0, 0.6, 0.1, getVertexSize(u));		
	}
	
	std::set<graph_vertex> labels_diff;
	std::set_difference(old_labels.begin(), old_labels.end(), extract_keys(m_labeled_vertices.begin()), extract_keys(m_labeled_vertices.end()), inserter(labels_diff, labels_diff.end()));
	for(std::set<graph_vertex>::iterator i = labels_diff.begin(); i != labels_diff.end(); ++i) {
		m_label_info.erase(*i);
	}

	// draw the labels
	drawLabels();
	
	// move them
	moveLabels();
	
	glFlush();
}

float VisualGLDisplay::getVertexSize(graph_vertex u) {
	if (m_options & SIZE_DEGREE) {
		return m_vertex_degrees[u]/m_max_degree * 70 + 10;
	}
	if (m_options & SIZE_RANK) {
		return 5;
	}
	// assume uniform size
	return 20;
}

bool VisualGLDisplay::shouldDrawVertex(graph_vertex u) const {
	if ((*m_graph)[u].type_major == node_type_major_doc && m_options & SHOW_DOC_NODES) return true;
	if ((*m_graph)[u].type_major == node_type_major_term && m_options & SHOW_TERM_NODES) return true;
	return false;
}

void VisualGLDisplay::setOptions(int o) {
    m_options = o;
}

point VisualGLDisplay::screenToLocal(const point &p, bool flip) {
	point norm(p.x()/width(), p.y()/height(), 0);
	point np(norm.x() * m_dimensions.x(), (flip?(1-norm.y()):norm.y()) * m_dimensions.y(), 0);
	return np + m_min;
}

void VisualGLDisplay::drawVertex(graph_vertex u, GLfloat x, GLfloat y, GLfloat r, GLfloat g, GLfloat b, GLfloat size) {
	// alpha
	GLfloat a = shouldDrawVertex(u)?0.8:0.1;
	// see if it's highlighted. if so draw it yellow
	if (m_have_highlighted_node && m_highlighted_node == u) {
		r = 0.9; g = 0.8; b = 0.15;
	}
	
	int screen_size = static_cast<int>(size / m_dimensions.x() * width());
	int seg = screen_size * 20 / 50;
	keep_within_range(seg, 5, 15);
	
	const GLfloat f = 0.6;

	if (screen_size > 2) {
		// first draw the outer circle
		glColor4f(r,g,b,a);
		mkCircle(x,y,size,seg);
		// and the inner circle
		glColor4f(r*f, g*f, b*f,a);
		mkFilledCircle(x,y,size,seg);
	} else {
		glPointSize(screen_size);
		glColor4f(r*f, g*f, b*f,a);
		glBegin(GL_POINTS);
			glVertex2f(x,y);
		glEnd();
	}
}

void VisualGLDisplay::mkFilledCircle(GLfloat x, GLfloat y, GLfloat size, int seg) {
	static bool first = true;
	static GLuint object[30];
	static bool have_object[30];
	if (first) {
		for(int i = 0; i < 30; i++) have_object[i] = false;
		first = false;
	}
	if (!have_object[seg]) {
		object[seg] = glGenLists(1);
		glNewList(object[seg], GL_COMPILE);
		
		glBegin(GL_TRIANGLE_FAN);
		glVertex2f(0,0);
		radialVertices(0,0,1.0,seg,true);
		glEnd();
		
		glEndList();
		
		have_object[seg] = true;
	}
	
	glLoadIdentity();
	glTranslatef(x,y,0);
	glScalef(size, size, 1);
	glCallList(object[seg]);
	glLoadIdentity();
}

void VisualGLDisplay::radialVertices(GLfloat x, GLfloat y, GLfloat size, int seg, bool includeLast) {
	GLfloat size2 = size/2;

	int cmp = includeLast?seg+1:seg;

	for(int i = 0; i < cmp; i++) {
		double a = i*M_PI*2/seg;
		GLfloat nx, ny;
		nx = cos(a) * size2 + x;
		ny = sin(a) * size2 + y;
		glVertex2f(nx, ny);
	}	
}

void VisualGLDisplay::mkCircle(GLfloat x, GLfloat y, GLfloat size, int seg) {
	static bool first = true;
	static GLuint object[30];
	static bool have_object[30];
	if (first) {
		for(int i = 0; i < 30; i++) have_object[i] = false;
		first = false;
	}
	if (!have_object[seg]) {
		object[seg] = glGenLists(1);
		glNewList(object[seg], GL_COMPILE);
		
		glBegin(GL_LINE_LOOP);
		radialVertices(0,0,1.0,seg,false);
		glEnd();
		
		glEndList();
		
		have_object[seg] = true;
	}
	
	glLoadIdentity();
	glTranslatef(x,y,0);
	glScalef(size, size, 1);
	glCallList(object[seg]);
	glLoadIdentity();
}

void VisualGLDisplay::drawFilledRect(GLfloat x, GLfloat y, GLfloat width, GLfloat height) {
	static bool first = true;
	static GLuint object;
	if (first) {
		object = glGenLists(1);
		glNewList(object, GL_COMPILE);
		
		glBegin(GL_QUADS);
		{
			glVertex2f(0,0);
			glVertex2f(1, 0);
			glVertex2f(1, 1);
			glVertex2f(0, 1);		
		}
		glEnd();
		
		glEndList();
		
		first = false;
	}
	
	glLoadIdentity();
	glTranslatef(x,y,0);
	glScalef(width, height, 1);
	glCallList(object);
	glLoadIdentity();
}

void VisualGLDisplay::moveLabels() {
	// this function will move the labels around so that they don't intersect each other
	// or the side of the screen
	for(unsigned int i = 0; i < m_labeled_vertices.size(); i++) {
		// go through each label, see if it needs to be moved
		point motion(0,0,0);
		graph_vertex u,v;
		u = m_labeled_vertices[i].first;
		point u_pos, u_dim, v_pos, v_dim;
		boost::tie(u_pos, u_dim) = m_label_info[u];
		
		// look at all the other labels, see if we need to move them away from each other
		for(unsigned int k = 0; k < m_labeled_vertices.size(); k++) {
			v = m_labeled_vertices[k].first;
			boost::tie(v_pos, v_dim) = m_label_info[v];
			
			// first check the node -- even our own -- we don't want to cover up any nodes
			point rel = m_labeled_vertices[k].second - u_pos;
			point max = u_dim;
			max.x() *= 0.5;
			max.x() += 20.0/width()*m_dimensions.x();
			point dist = rel.absolute();
			point diff = dist - max;
			if (diff.x() <= 0 && diff.y() <= 0) {
				// we are too close to a point (node/vertex). move away from it

				// add repelling force to motion based on inverse square-ness
				float offender, max_allowed;
				if (diff.x() < diff.y()) { offender = dist.x(); max_allowed = max.x(); }
				else { offender = dist.y(); max_allowed = max.y(); }
				
				float norm = offender/max_allowed;
				float inv_sqr = 1;
				if (norm != 0 && max_allowed != 0)
					inv_sqr = 1/(norm*norm);
				else rel.set(0,1,0);
				
				// now the repelling force * inv_sqr
				motion += rel * -1 * inv_sqr;
			}
			
			// now check if this is a label & it's too close. we won't check our own label
			if (i == k) continue;

			// are we too close?
			max = (v_dim + u_dim) * 0.5;
			max.x() += 20.0/width()*m_dimensions.x();
			dist = (v_pos - u_pos).absolute();
			diff = dist - max;
			// if we are too close on the x or y, move away from them
			if (diff.x() <= 0 && diff.y() <= 0) {
				// add repelling force to motion based on inverse square-ness
				float offender, max_allowed;
				if (diff.x() < diff.y()) { offender = dist.x(); max_allowed = max.x(); }
				else { offender = dist.y(); max_allowed = max.y(); }
				
				if (max_allowed == 0) max_allowed = 1;
				float norm = offender/max_allowed;
				if (norm == 0) norm = 1;
				float inv_sqr = 1/norm - 1;
				
				// now the repelling force * inv_sqr
				motion += (u_pos - v_pos).normalize() * inv_sqr * (max_allowed - offender);
			}
		}
		
		// add a bit of a "go home" force
//		motion += (m_labeled_vertices[i].second - u_pos) / 20;
		
		point move = motion/50;
		// if the movement is too extreme, cap it
		if (move.length() > m_dimensions.x()/50) {
			move = move.normalize() * m_dimensions.x()/50;
		}
		
		std::list<point> &moves = m_label_moves[u];
		
		moves.push_back(move);
		if (moves.size() > 1) moves.pop_front();
		
		// get the average of the moves in the queue
		point movement(0,0,0);
		movement = std::accumulate(moves.begin(), moves.end(), movement);
		movement /= moves.size();
		
		u_pos = m_label_info[u].first + movement;
		
		// now move ourselves so that we're not touching walls
		if (u_pos.x() + u_dim.x() > m_max.x()) {
			// we're off the right side
			u_pos.x() = m_max.x() - u_dim.x()*0.6;
		}
		if (u_pos.x() - u_dim.x() < m_min.x()) {
			// off the left side
			u_pos.x() = m_min.x() + u_dim.x()*0.6;
		}
		if (u_pos.y() + u_dim.y() > m_max.y()) {
			// off the "top" (y-axis)
			u_pos.y() = m_max.y() - u_dim.y();
		}
		if (u_pos.y() - u_dim.y() < m_min.y()) {
			// off the "bottom"
			u_pos.y() = m_min.y() + u_dim.y();
		}
		
		m_label_info[u].first = u_pos;
	}
}

std::string VisualGLDisplay::unstemTerm(const std::string &stem){
	GraphTraits::vertex_descriptor u;
	u = m_graph->vertex_by_id(
             m_graph->fetch_vertex_id_by_content_and_type(
                 stem, node_type_major_term));

	std::string term = m_graph->get_vertex_meta_value(u, "term");
    std::string::size_type pos = term.find_last_of(":");
	if( pos != std::string::npos && pos > 0 ){
		return term.substr(0,pos);
	}
	return stem;
}


void VisualGLDisplay::drawLabels() {
	if( isSearching ) 
		return;
	
	for(unsigned int i = 0; i < m_labeled_vertices.size(); i++) {
		graph_vertex u = m_labeled_vertices[i].first;
		point p = m_labeled_vertices[i].second;
		GLfloat x = p.x(), y = p.y();
		GLfloat r,g,b;
		// get the base color
		if (m_have_highlighted_node && m_highlighted_node == u) {
			r = 0.9; g = 0.8; b = 0.15;
		} else if ((*m_graph)[u].type_major == node_type_major_doc) {
			r = 0.3; g = 0.5; b = 0.8;
		} else {
			r = 0.3; g = 0.8; b = 0.5;
		}

		
		std::string the_label = "label";
		if( m_graph ){
			
			if ((*m_graph)[u].type_major == node_type_major_doc) {
				the_label = (*m_graph)[u].content;
				// the_label = scrub_vertex_title(m_graph, (*m_graph)[u].content);
			} else {
				the_label = unstemTerm((*m_graph)[u].content);
		    }
		}
		
		GLfloat orig_x = x, orig_y = y;
		GLfloat d_x, d_y, b_x, b_y;

		// get the metrics for the text
		QFont font("Arial", 12);
		QFontMetricsF metrics(font, this);

		QRectF box = metrics.boundingRect(QString::fromStdString(the_label));
		point box_local((box.width()+2)/width() * m_dimensions.x() *1.1+2, (box.height()+2)/height() * m_dimensions.y(), 0);

		if (m_label_info.count(u)) {
			point p = m_label_info[u].first;
			x = p.x();
			y = p.y();
		} else {
			m_label_info[u] = std::make_pair(point(x,y,0), box_local);
		}

		// either way, update the box
		m_label_info[u].second = box_local;

		// where will we actually draw the box/label?
		d_x = x - box_local.x()/2;
		d_y = y - box_local.y()/2;
		b_x = d_x - 1.0/width() * m_dimensions.x();
		b_y = d_y - 2.0/height() * m_dimensions.y();

		// first draw the line connecting the label and the node in red
		glColor4f(r,g,b, 0.7);
		glBegin(GL_LINES);
		{
			glVertex2f(orig_x, orig_y);
			glVertex2f(x, y);
		}
		glEnd();

	//	glColor4f(0.9,0.9,0.9,1.0);
		glColor4f(r,g,b,0.8);
		drawFilledRect(b_x,b_y,box_local.x(),box_local.y());
		glColor4f(0,0,0,1);
		renderText(d_x,d_y,0,QString(the_label.c_str()),font);

	}
}

void VisualGLDisplay::mouseMoveEvent(QMouseEvent *event) {
	m_mouse_pos = point(event->pos().x(), event->pos().y(), 0);
	m_mouse_in = true;
}

void VisualGLDisplay::mouseDoubleClickEvent(QMouseEvent *) {
	if (m_have_highlighted_node) {
		graph_vertex u = m_highlighted_node;
		std::string node = (*m_graph)[u].content;
		if ((*m_graph)[u].type_major == node_type_major_doc) {
			QString nodeTitle = QString::fromStdString( node );
			/* emit */ documentDoubleClicked(nodeTitle);
		} else {
			reset();
			QString nodeTitle = QString::fromStdString( unstemTerm( node ));
			isSearching = true;
			/* emit */ termDoubleClicked(nodeTitle);
			//std::cerr << "emit: term double clicked" << std::endl;
		}
	}
}

void VisualGLDisplay::mousePressEvent(QMouseEvent *) {
	if (m_have_highlighted_node) {
		// let's show the doc!!
//		m_controller->showDocument(m_highlighted_node);
	}
}
