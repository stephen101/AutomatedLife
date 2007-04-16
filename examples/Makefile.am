EXTRA_PROGRAMS = test linlog search tagger attach_titles mst summarize file_reader file_finder

INCLUDES = -I$(top_builddir)/include
AM_CPPFLAGS=@BOOST_CPPFLAGS@ 
LIBS=@BOOST_LIBS@ 

tagger_SOURCES = tagger.cpp

test_SOURCES = test.cpp
test_LDADD = @MYSQL_LIBS@ @SQLITE3_LIBS@ @LIBICONV@ @MSWORD_READER_LIBS@ @PDF_READER_LIBS@
test_CXXFLAGS = @MYSQL_CFLAGS@ @SQLITE3_CFLAGS@ @MSWORD_READER_CPPFLAGS@ @PDF_READER_CPPFLAGS@

linlog_SOURCES = linlog.cpp
linlog_LDADD = @LIBICONV@ @MSWORD_READER_LIBS@ @PDF_READER_LIBS@
linlog_CXXFLAGS = @MSWORD_READER_CPPFLAGS@ @PDF_READER_CPPFLAGS@

search_SOURCES = search.cpp
search_LDADD = @MYSQL_LIBS@
search_CXXFLAGS = @MYSQL_CFLAGS@

attach_titles_SOURCES = attach_titles.cpp
attach_titles_LDADD = @MYSQL_LIBS@ @SQLITE3_LIBS@
attach_titles_CXXFLAGS = @MYSQL_CFLAGS@ @SQLITE3_CFLAGS@

mst_SOURCES = mst.cpp
mst_LDADD = @SQLITE3_LIBS@
mst_CXXFLAGS = @SQLITE3_CFLAGS@

summarize_SOURCES = summarize.cpp

file_reader_SOURCES = file_reader.cpp
file_reader_LDADD = @LIBICONV@ @MSWORD_READER_LIBS@ @PDF_READER_LIBS@
file_reader_CXXFLAGS = @MSWORD_READER_CPPFLAGS@ @PDF_READER_CPPFLAGS@

file_finder_SOURCES = file_finder.cpp
