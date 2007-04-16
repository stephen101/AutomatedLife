AC_AUX_DIR = m4
AC_MACRO_DIR = m4
SUBDIRS = include share tools 
EXTRA_DIST = $(top_srcdir)/bootstrap \
			 $(top_srcdir)/pl/API.xs \
			 $(top_srcdir)/pl/Changes \
			 $(top_srcdir)/pl/Makefile.PL.in \
			 $(top_srcdir)/pl/MANIFEST \
			 $(top_srcdir)/pl/perlobject.map \
			 $(top_srcdir)/pl/README \
			 $(top_srcdir)/pl/lib/Semantic/API.pm \
			 $(top_srcdir)/pl/scripts/autocomplete.pl \
			 $(top_srcdir)/pl/scripts/export_wordlist.pl \
			 $(top_srcdir)/pl/scripts/federated.pl \
			 $(top_srcdir)/pl/scripts/index.pl \
			 $(top_srcdir)/pl/scripts/search.pl \
			 $(top_srcdir)/pl/t/api.t \
			 $(top_srcdir)/pl/t/mysql.t \
			 $(top_srcdir)/pl/t/sqlite.t \
			 $(top_srcdir)/pl/typemap \
			 $(top_srcdir)/m4/bnv_have_qt.m4 \
			 $(top_srcdir)/m4/boost.m4 \
			 $(top_srcdir)/m4/iconv.m4 \
			 $(top_srcdir)/m4/lib-ld.m4 \
			 $(top_srcdir)/m4/lib-link.m4 \
			 $(top_srcdir)/m4/lib-prefix.m4 \
			 $(top_srcdir)/m4/msword-reader.m4 \
			 $(top_srcdir)/m4/mysql.m4 \
			 $(top_srcdir)/m4/mysqlr.m4 \
			 $(top_srcdir)/m4/pdf-reader.m4 \
			 $(top_srcdir)/m4/sqlite3.m4 \
			 $(top_srcdir)/m4/zlib.m4 \
			 $(top_srcdir)/sql/mysql5/procedures.sql \
			 $(top_srcdir)/sql/mysql5/tables.sql \
			 $(top_srcdir)/sql/mysql5/termlist.sql \
			 $(top_srcdir)/sql/mysql5/triggers.sql \
			 $(top_srcdir)/sql/sqlite3/tables.sql \
			 $(top_srcdir)/sql/sqlite3/termlist.sql \
			 $(top_srcdir)/sql/sqlite3/triggers.sql \
			 $(top_srcdir)/doc/html/examples.html \
			 $(top_srcdir)/doc/html/filtering.html \
			 $(top_srcdir)/doc/html/indexing.html \
			 $(top_srcdir)/doc/html/weights.html \
			 $(top_srcdir)/doc/html/index.html \
			 $(top_srcdir)/doc/html/installation.html \
			 $(top_srcdir)/doc/html/usage.html
			
dist-hook:
	rm -rf $(distdir)/qt; \
	if [ -f $(top_srcdir)/pl/scripts/create_mysql_db.pl ]; then \
		cp $(top_srcdir)/pl/scripts/create_mysql_db.pl $(distdir)/pl/scripts/create_mysql_db.pl; \
	elif [ -f $(top_srcdir)/pl/scripts/stub_create_mysql.pl ]; then \
		cat $(top_srcdir)/pl/scripts/stub_create_mysql.pl \
			$(top_srcdir)/sql/mysql5/tables.sql \
	    	$(top_srcdir)/sql/mysql5/triggers.sql \
	    	$(top_srcdir)/sql/mysql5/procedures.sql >> $(distdir)/pl/scripts/create_mysql_db.pl; \
		chmod +x $(distdir)/pl/scripts/create_mysql_db.pl; \
	fi 
	
	     

