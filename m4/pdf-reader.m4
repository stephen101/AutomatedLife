AC_DEFUN([AC_CHECK_PDF_READER],
#
# Handle user hints
#
[
AC_ARG_WITH(pdf-reader,
[AS_HELP_STRING([--with-pdf-reader],[path to pdf-reader installation (defaults to
                    /usr/local or /usr)])],
[if test "$withval" != yes ; then
  
    PDF_READER_HOME="$withval"
  
fi])

PDF_READER_LIBS=$PDF_READER_HOME
PDF_READER_INCLUDE=$PDF_READER_HOME
if test ! -f "${PDF_READER_INCLUDE}/pdftotext.h"
then
	PDF_READER_HOME=/usr/local
	if test -f "${PDF_READER_HOME}/include/pdf-reader/pdftotext.h"
	then
		PDF_READER_INCLUDE="${PDF_READER_HOME}/include/pdf-reader"
		PDF_READER_LIBS="${PDF_READER_HOME}/lib"
	else 
		# try /usr
		PDF_READER_HOME=/usr
		PDF_READER_INCLUDE="${PDF_READER_HOME}/include/pdf-reader"
		PDF_READER_LIBS="${PDF_READER_HOME}/lib"
	fi
fi

#
# Locate pdf-reader, if wanted
#
if test -f "${PDF_READER_INCLUDE}/pdftotext.h"
then
        AC_MSG_CHECKING(for pdf-reader)
		AC_MSG_RESULT(yes)
		PDF_READER_CPPFLAGS="-I${PDF_READER_INCLUDE}"
		PDF_READER_LIBS="-L${PDF_READER_LIBS} -lpdf_reader"
		HAVE_PDF_READER=1
else 
		AC_MSG_CHECKING(for pdf-reader)
		AC_MSG_RESULT(no)
		PDF_READER_CPPFLAGS=""
		PDF_READER_LIBS=""
		HAVE_PDF_READER=0
fi
AC_SUBST(PDF_READER_CPPFLAGS)
AC_SUBST(PDF_READER_LIBS)
AC_SUBST(HAVE_PDF_READER)

])
