/*
	SQLite 3
	
	At configure time:
	
	SQLITE3_LIBS 		= /usr/lib/libsqlite3.a
	SQLITE3_CFLAGS 		= -I/usr/include
*/

#define SEMANTIC_HAVE_SQLITE3 1
 

/*
	MySQL
	
	At configure time:
	
	MYSQL_LIBS			= -L/usr/local/mysql/lib -lmysqlclient -lz -lm
	MYSQL_CFLAGS		= -I/usr/local/mysql/include -Os -arch ppc -fno-common
*/

#define SEMANTIC_HAVE_MYSQL 1

/*
	BOOST
	
	At configure time:
	
	BOOST_CPPFLAGS		= -I/usr/local/include/boost-1_33_1
	BOOST_LIBS			= -lboost_signals -lboost_filesystem -lboost_program_options 
*/

#define SEMANTIC_STORAGE_ENGINES "sqlite3 mysql5 "


/*
	MSWord-Reader
	
	At configure time:
	
	MSWORD_READER_CPPFLAGS 	= -I/usr/local/include/msword-reader
	MSWORD_READER_LIBS		= -L/usr/local/lib -lmsword_reader
*/

#define SEMANTIC_HAVE_MSWORD_READER 0


/*
	PDF-Reader
	
	At configure time:
	
	PDF_READER_CPPFLAGS 	= -I/usr/local/include/pdf-reader
	PDF_READER_LIBS		= -L/usr/local/lib -lpdf_reader
*/

#define SEMANTIC_HAVE_PDF_READER 0

/*
	Iconv
	
	Determines iconv interface
	
*/

#define ICONV_CONST 1

/* 
	Unicode (std::wstring) support for the stemmer
*/

#define SEMANTIC_UNICODE 0


	
/*
    The configure (autoconf) install prefix
*/

#define AUTOCONF_INSTALL_PREFIX "C:/Semantic"

#define LEXICON_INSTALL_LOCATION "C:/Semantic/share/lexicon.txt"
#define STOPLIST_INSTALL_LOCATION "C:/Semantic/share/stoplist_en.txt"

