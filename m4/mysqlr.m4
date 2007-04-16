AC_DEFUN([AC_CHECK_MYSQLR],[
AC_PATH_PROG(mysqlconfig,mysql_config)
if test [ -z "$mysqlconfig" ]
then
    AC_MSG_ERROR([mysql_config executable not found])
else
    AC_MSG_CHECKING(mysql libraries)
    MYSQL_LIBS=`${mysqlconfig} --libs | sed -e \
    's/-lmysqlclient /-lmysqlclient_r /' -e 's/-lmysqlclient$/-lmysqlclient_r/'`
    AC_MSG_RESULT($MYSQL_LIBS)
    AC_MSG_CHECKING(mysql includes)
    MYSQL_CFLAGS=`${mysqlconfig} --cflags`
    AC_MSG_RESULT($MYSQL_CFLAGS)
fi
])
