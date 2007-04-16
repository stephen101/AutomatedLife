AC_DEFUN([AC_CHECK_MSWORD_READER],
#
# Handle user hints
#
[
AC_ARG_WITH(msword-reader,
[AS_HELP_STRING([--with-msword-reader],[path to msword-reader installation (defaults to
                    /usr/local or /usr)])],
[if test "$withval" != yes ; then
  
    MSWORD_READER_HOME="$withval"
  
fi])

MSWORD_READER_LIBS="${MSWORD_READER_HOME}/lib"
MSWORD_READER_INCLUDE="${MSWORD_READER_HOME}/include/msword-reader"
if test ! -f "${MSWORD_READER_INCLUDE}/msword_reader.hpp"
then
	MSWORD_READER_HOME=/usr/local
	if test -f "${MSWORD_READER_HOME}/include/msword-reader/msword-reader.hpp"
	then
		MSWORD_READER_INCLUDE="${MSWORD_READER_HOME}/include/msword-reader"
		MSWORD_READER_LIBS="${MSWORD_READER_HOME}/lib"
	else 
		# try /usr
		MSWORD_READER_HOME=/usr
		MSWORD_READER_INCLUDE="${MSWORD_READER_HOME}/include/msword-reader"
		MSWORD_READER_LIBS="${MSWORD_READER_HOME}/lib"
	fi
fi

#
# Locate MSWord-Reader, if wanted
#
if test -f "${MSWORD_READER_INCLUDE}/msword-reader.hpp"
then
        AC_MSG_CHECKING(for msword-reader)
		AC_MSG_RESULT(yes)
		MSWORD_READER_CPPFLAGS="-I${MSWORD_READER_INCLUDE}"
		MSWORD_READER_LIBS="-L${MSWORD_READER_LIBS} -lmsword_reader"
		HAVE_MSWORD_READER=1
else 
		AC_MSG_CHECKING(for msword-reader)
		AC_MSG_RESULT(no)
		MSWORD_READER_CPPFLAGS=""
		MSWORD_READER_LIBS=""
		HAVE_MSWORD_READER=0
fi
AC_SUBST(MSWORD_READER_CPPFLAGS)
AC_SUBST(MSWORD_READER_LIBS)
AC_SUBST(HAVE_MSWORD_READER)

])
