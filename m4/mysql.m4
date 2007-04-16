AC_DEFUN([AC_CHECK_MYSQL],[
	# allow user to set mysql options
	mysqlconfig="auto"
	HAVE_MYSQL=0
	AC_ARG_WITH(mysql,
		AC_HELP_STRING([--with-mysql[=/path/to/mysql_config]], [configure with MySQL support. default = no]),
		[ if test "x$withval" != "xyes"; then
			mysqlconfig="$withval"
		  fi ], [ mysqlconfig="no" ])

	if test "x$mysqlconfig" = "xauto" || test -z $mysqlconfig; then
		AC_PATH_PROG(mysqlconfig,mysql_config)
	fi

	if test "x$mysqlconfig" != "xno"; then
		if test [ -z "$mysqlconfig" ]
		then
		    AC_MSG_ERROR([mysql_config executable not found])
		else
			if test ! -x "$mysqlconfig"; then
				AC_MSG_ERROR([file $mysqlconfig does not exist or is not executable])
			fi
		    AC_SUBST(MYSQL_LIBS)
		    AC_SUBST(MYSQL_CFLAGS)
		    AC_MSG_CHECKING(mysql libraries)
		    MYSQL_LIBS=`${mysqlconfig} --libs`
		    AC_MSG_RESULT($MYSQL_LIBS)
		    AC_MSG_CHECKING(mysql includes)
		    MYSQL_CFLAGS=`${mysqlconfig} --cflags`
		    AC_MSG_RESULT($MYSQL_CFLAGS)
			HAVE_MYSQL=1
			AC_SUBST(MYSQL_CFLAGS)
			AC_SUBST(MYSQL_LIBS)
		fi
	fi
	
	AC_SUBST(HAVE_MYSQL)
])
