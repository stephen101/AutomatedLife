######################################################################
# Program file for the SemanticEngine application
######################################################################

TEMPLATE = app
TARGET = SemanticEngine
DEPENDPATH += .
INCLUDEPATH += . ./include ../../include
RESOURCES = resources.qrc
QT += opengl
debug {
    message("Building Debug")
    TARGET = SemanticEngine-debug
}
!debug {
    message("Building Release")
    TARGET = SemanticEngine
    CONFIG += no_warn
}

# define storage engine
contains( STORAGE, mysql ) {
    message( "Building with MySQL storage" )
	DEFINES += MYSQL_STORAGE
} else {
	message ("Building with SQLite storage")
	DEFINES += SQLITE_STORAGE
}


# Input
HEADERS += include/display.h \
           include/mainwindow.h \
           include/basket.h \
           include/collection.h \
           include/clusters.h \
           include/locate_datafile.h \
           include/single_result.h \
           include/se_graph.h \
           include/results.h \
           include/graph_controller.h \
           include/tabbed_display.h \
           include/visual_display.h \
           include/visual_gl_display.h
        
        
SOURCES += src/display.cpp \
           src/results.cpp \
           src/basket.cpp \
           src/collection.cpp \
           src/mainwindow.cpp \
           src/clusters.cpp \
           src/single_result.cpp \
           src/graph_controller.cpp \
           src/tabbed_display.cpp \
           src/visual_display.cpp \
           src/visual_gl_display.cpp \
           src/main.cpp 

macx {
    RC_FILE = icons/SemanticEngine.icns
    SOURCES += src/locate_datafile_mac.cpp
}

win32 {
    RC_FILE = search.rc
    SOURCES += src/locate_datafile_win32.cpp

    QMAKE_CXXFLAGS += -IC:/Boost/include/boost-1_33_1 \
                      -IC:/SQLite/include \
                      -IC:/MySQL/include \
                      -IC:/Progra~1/GnuWin32/include

    LIBS += C:/Boost/lib/libboost_filesystem-mgw.lib \
            C:/SQLite/lib/sqlite3.dll \
            C:/MySQL/lib/opt/libmysql.a \
            C:/Progra~1/GnuWin32/lib/libiconv.lib

}

unix {
    !macx{
        SOURCES += src/locate_datafile_unix.cpp
    }

    QMAKE_CXXFLAGS += 	-I/usr/local/include/boost-1_33_1 \
						-I/usr/include \
						-I/usr/local/include/msword-reader \
						-I/usr/local/include/pdf-reader \
						-I/usr/local/mysql/include

    LIBS += /usr/local/lib/libsqlite3.a \
			/usr/local/mysql/lib/libmysqlclient.a \
 			-L/usr/local/lib -lboost_filesystem -lmsword_reader -lpdf_reader -liconv \

}
