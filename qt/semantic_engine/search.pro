######################################################################
# Program file for the SemanticEngine application
######################################################################

TEMPLATE = app
TARGET = SemanticEngine
DEPENDPATH += .
INCLUDEPATH += . ./include ../../include
RESOURCES = resources.qrc
CONFIG += thread debug_and_release
QT += opengl

debug {
    message("Building Debug")
    TARGET = SemanticEngine-debug
} else {
    message("Building Release")
    TARGET = SemanticEngine
    CONFIG += no_warn
}


# define storage engine
contains( STORAGE, mysql ) {
    message( "Building with MySQL storage" )
	DEFINES += MYSQL_STORAGE
    win32{
        INCLUDEPATH += C:/MySQL/include
        LIBS += C:/MySQL/lib/opt/libmysql.lib 
    }
} else {
	message ("Building with SQLite storage")
	DEFINES += SQLITE_STORAGE
    win32{
        INCLUDEPATH += C:/SQLite/include
        LIBS += C:/SQLite/lib/sqlite3.lib
    }
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

    CONFIG += embed_manifest_exe
    debug {
	    CONFIG += console
    }

    SOURCES += src/locate_datafile_win32.cpp
    

    INCLUDEPATH += C:/Boost/include/boost-1_33_1 \
                   C:/Progra~1/GnuWin32/include \
                   C:/Semantic/msword-reader \
                   C:/Semantic/pdf-reader 

    LIBS += C:/Boost/lib/libboost_filesystem-vc80-mt-1_33_1.lib \
            C:/Progra~1/GnuWin32/lib/libiconv.lib \
            C:/Semantic/msword-reader/MSWordReader.lib \
            C:/Semantic/pdf-reader/PDFReader.lib 
            
}

unix {
    !macx{
        SOURCES += src/locate_datafile_unix.cpp
    }

    INCLUDEPATH += 	/usr/local/include/boost-1_33_1 \
						/usr/include \
						/usr/local/include/msword-reader \
						/usr/local/include/pdf-reader \
						/usr/local/mysql/include

    LIBS += /usr/local/lib/libsqlite3.a \
			/usr/local/mysql/lib/libmysqlclient.a \
 			-L/usr/local/lib -lboost_filesystem -lmsword_reader -lpdf_reader -liconv \

}
