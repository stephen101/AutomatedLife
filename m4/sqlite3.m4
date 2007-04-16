dnl -*- autoconf -*-
dnl $id$
dnl Check for libsqlite, based on version found at libdbi-drivers.sf.net (GPLv2-licensed)

AC_DEFUN([AC_FIND_FILE], [
  $3=no
  for i in $2; do
      for j in $1; do
          if test -r "$i/$j"; then
              $3=$i
              break 2
          fi
      done
  done ])

AC_DEFUN([AC_CHECK_SQLITE], [
  have_sqlite="no"
  ac_sqlite="no"
  ac_sqlite_incdir="no"
  ac_sqlite_libdir="no"

  # exported variables
  SQLITE3_LIBS=""
  SQLITE3_CFLAGS=""
  HAVE_SQLITE3=0

  AC_ARG_WITH(sqlite3,
      AC_HELP_STRING([--with-sqlite3],[configure with libsqlite3 at given dir. default = no] ),
      [ ac_sqlite="auto" 
        if test "x$withval" != "xyes"; then
            ac_sqlite="yes"
            ac_sqlite_incdir="$withval"/include
            ac_sqlite_libdir="$withval"/lib
		fi ],
      [ ac_sqlite="no" ] )
  AC_ARG_WITH(sqlite3-incdir,
      AC_HELP_STRING( [--with-sqlite3-incdir],
                      [specifies where the SQLite include files are.] ),
      [  ac_sqlite_incdir="$withval" ] )
  AC_ARG_WITH(sqlite3-libdir,
      AC_HELP_STRING( [--with-sqlite3-libdir],
                      [specifies where the SQLite libraries are.] ),
      [  ac_sqlite_libdir="$withval" ] )

  # Try to automagically find SQLite, either with pkg-config, or without.
  if test "x$ac_sqlite" = "xauto"; then
      if test "x$PKGCONFIG" != "xno" -a "x$PKGCONFIG" != "x"; then
          SQLITE3_LIBS=$($PKGCONFIG --libs sqlite)
          SQLITE3_CFLAGS=$($PKGCONFIG --cflags sqlite)
          if test "x$SQLITE3_LIBS" = "x" -a "x$SQLITE3_CFLAGS" = "x"; then
              AC_CHECK_LIB([sqlite3], [sqlite3_open], [ac_sqlite="yes"], [ac_sqlite="no"])
          else
              ac_sqlite="yes"
          fi
          AC_MSG_RESULT([$ac_sqlite])
      else
          AC_CHECK_LIB([sqlite3], [sqlite3_open], [ac_sqlite="yes"], [ac_sqlite="no"])
      fi
  fi

  if test "x$ac_sqlite" = "xyes"; then
	  AC_MSG_CHECKING([for SQLite])
	  sqlite_incdirs="$ac_sqlite_incdir"
	  sqlite_libdirs="$ac_sqlite_libdir"
	  sqlite_libs_static="libsqlite3.a"
      sqlite_libs="libsqlite3.so libsqlite3.dylib"

      if test "$ac_sqlite_incdir" = "no"; then
          sqlite_incdirs="/usr/include /usr/local/include /usr/include/sqlite /usr/local/include/sqlite /usr/local/sqlite/include /opt/sqlite/include"
      fi

	  if test "$ac_sqlite_libdir" = "no"; then
          sqlite_libdirs="/usr/lib /usr/local/lib /usr/lib/sqlite /usr/local/lib/sqlite /usr/local/sqlite/lib /opt/sqlite/lib"		  
	  fi
	
      AC_FIND_FILE($sqlite_libs_static, $sqlite_libdirs, ac_sqlite_libdir)
      if test "$ac_sqlite_libdir" = "no"; then
		  AC_FIND_FILE($sqlite_libs, $sqlite_libdirs, ac_sqlite_libdir)
	      if test "$ac_sqlite_libdir" = "no"; then
	          AC_MSG_ERROR([Invalid SQLite directory - libraries not found.])
		  else
	      	  test "x$SQLITE3_LIBS" = "x" && SQLITE3_LIBS="-L$ac_sqlite_libdir -lsqlite3"
		  fi
	  else
		  test "x$SQLITE3_LIBS" = "x" && SQLITE3_LIBS="$ac_sqlite_libdir/libsqlite3.a"
      fi

      AC_FIND_FILE(sqlite3.h, $sqlite_incdirs, ac_sqlite_incdir)
      if test "$ac_sqlite_incdir" = "no"; then
          AC_MSG_ERROR([Invalid SQLite directory - include files not found.])
      fi

      have_sqlite="yes"

      test "x$SQLITE3_CFLAGS" = "x" && SQLITE3_CFLAGS=-I$ac_sqlite_incdir

      HAVE_SQLITE3=1
      AC_SUBST(SQLITE3_LIBS)
      AC_SUBST(SQLITE3_CFLAGS)
	  AC_MSG_RESULT([yes])
  fi
  AC_SUBST(HAVE_SQLITE3)
])