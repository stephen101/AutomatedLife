AC_DEFUN([REQUIRE_ZLIB],
#
# Handle user hints
#
[
AC_ARG_WITH(zlib,
[  --with-zlib-dir=DIR root directory path of zlib installation [defaults to
                    /usr/local or /usr if not found in /usr/local]],
[if test "$withval" != no ; then
  if test -d "$withval"
  then
    ZLIB_HOME="$withval"
  else
    AC_MSG_WARN([Sorry, $withval does not exist, checking usual places])
  fi
fi])

ZLIB_HOME=/usr/local
if test ! -f "${ZLIB_HOME}/include/zlib.h"
then
        ZLIB_HOME=/usr
fi

#
# Locate zlib, if wanted
#
if test -n "${ZLIB_HOME}"
then
        ZLIB_OLD_LDFLAGS=$LDFLAGS
        ZLIB_OLD_CPPFLAGS=$LDFLAGS
        LDFLAGS="$LDFLAGS -L${ZLIB_HOME}/lib"
        CPPFLAGS="$CPPFLAGS -I${ZLIB_HOME}/include"
        AC_LANG_SAVE
        AC_LANG_C
        AC_CHECK_LIB(z, inflateEnd, [zlib_cv_libz=yes], [zlib_cv_libz=no])
        AC_CHECK_HEADER(zlib.h, [zlib_cv_zlib_h=yes], [zlib_cv_zlib_h=no])
        AC_LANG_RESTORE
        if test "$zlib_cv_libz" = "yes" -a "$zlib_cv_zlib_h" = "yes"
        then
                #
                # If both library and header were found, use them
                #
                AC_MSG_CHECKING(zlib in ${ZLIB_HOME})
                AC_MSG_RESULT(ok)
				ZLIB_CFLAGS="-I${ZLIB_HOME}/include"
				ZLIB_LIBS="-L${ZLIB_HOME}/lib -lz"
				AC_SUBST(ZLIB_CFLAGS)
				AC_SUBST(ZLIB_LIBS)
        else
                #
                # If either header or library was not found, revert and bomb
                #
                AC_MSG_CHECKING(zlib in ${ZLIB_HOME})
                LDFLAGS="$ZLIB_OLD_LDFLAGS"
                CPPFLAGS="$ZLIB_OLD_CPPFLAGS"
                AC_MSG_RESULT(failed)
                AC_MSG_ERROR(please specify a valid zlib installation with --with-zlib-dir=DIR)
        fi
fi

])
