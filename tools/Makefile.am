
bin_PROGRAMS = semantic_indexer semantic_search

INCLUDES = -I$(top_builddir)/include
AM_CPPFLAGS=@BOOST_CPPFLAGS@ 
LIBS=@BOOST_LIBS@ 

semantic_indexer_SOURCES = indexer.cpp
semantic_indexer_LDADD = @MYSQL_LIBS@ @SQLITE3_LIBS@ @LIBICONV@ @MSWORD_READER_LIBS@ @PDF_READER_LIBS@
semantic_indexer_CXXFLAGS = @MYSQL_CFLAGS@ @SQLITE3_CFLAGS@ @MSWORD_READER_CPPFLAGS@ @PDF_READER_CPPFLAGS@

semantic_search_SOURCES = search.cpp
semantic_search_LDADD = @MYSQL_LIBS@ @SQLITE3_LIBS@
semantic_search_CXXFLAGS = @MYSQL_CFLAGS@ @SQLITE3_CFLAGS@
