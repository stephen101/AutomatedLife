dnl RS_BOOST([MINIMUM-VERSION], [ACTION-IF-FOUND], [ACTION-IF-NOT-FOUND])
dnl Test for the Boost C++ libraries of a particular version (or newer)
dnl Defines:
dnl   BOOST_CPPFLAGS to the set of flags required to compiled Boost
AC_DEFUN([RS_BOOST], 
[
  AC_SUBST(BOOST_CPPFLAGS)
  AC_SUBST(BOOST_LIBS)
  BOOST_CPPFLAGS=""
  path_given="no"

dnl Extract the path name from a --with-boost=PATH argument
  AC_ARG_WITH(boost,
    AC_HELP_STRING([--with-boost=PATH],[absolute path name where the Boost C++ libraries reside, or `int', for internal library. Alternatively, the BOOST_ROOT environment variable will be used]),
    [
    if test "$withval" != yes ; then
        BOOST_ROOT="$withval"
    fi
    ]
  )
  AC_ARG_WITH(boost-include,
    AC_HELP_STRING([--with-boost-include=PATH],[absolute path name where the Boost C++ include files reside.]),
    [
    if test "$withval" != yes ; then
        BOOST_INCLUDE="$withval"
    fi
    ]
  )

  if test "x$BOOST_ROOT" = x ; then
    BOOST_ROOT="/usr"
  fi

  boost_min_version=ifelse([$1], ,1.20.0,[$1])
  WANT_BOOST_MAJOR=`expr $boost_min_version : '\([[0-9]]*\)'`
  WANT_BOOST_MINOR=`expr $boost_min_version : '[[0-9]]*\.\([[0-9]]*\)'`
  WANT_BOOST_SUB_MINOR=`expr $boost_min_version : '[[0-9]]*\.[[0-9]]*\.\([[0-9]]*\)'`
  
  BOOST_CPPFLAGS="-I$BOOST_INCLUDE"
  
  if test "x$BOOST_INCLUDE" = x ; then
    BOOST_CPPFLAGS="-I$BOOST_ROOT/include/boost-${WANT_BOOST_MAJOR}_${WANT_BOOST_MINOR}_$WANT_BOOST_SUB_MINOR"
  fi
  
  AC_LANG_SAVE
  AC_LANG_CPLUSPLUS
  OLD_CPPFLAGS="$CPPFLAGS"
  CPPFLAGS="$CPPFLAGS $BOOST_CPPFLAGS"
  AC_MSG_CHECKING([for the Boost C++ libraries, version $boost_min_version or newer])
  AC_TRY_COMPILE(
    [
#include <boost/version.hpp>
],
    [],
    [
      have_boost="yes"
    ],
    [
      AC_MSG_RESULT(no)
      have_boost="no"
      ifelse([$3], , :, [$3])
    ])

  if test "$have_boost" = "yes"; then
    WANT_BOOST_VERSION=`expr $WANT_BOOST_MAJOR \* 100000 \+ $WANT_BOOST_MINOR \* 100 \+ $WANT_BOOST_SUB_MINOR`
    AC_TRY_COMPILE(
      [
#include <boost/version.hpp>
],
      [
#if BOOST_VERSION >= $WANT_BOOST_VERSION
// Everything is okay
#else
#  error Boost version is too old
#endif

],
      [
    AC_MSG_RESULT(yes)
    if test "$target_os" = "mingw32"; then
       boost_libsuff=mgw
    else
       boost_libsuff=gcc
    fi
    boost_libsuff_r=$boost_libsuff-mt;
    if test "x$enable_debug" = xyes ; then
        boost_libsuff=$boost_libsuff-d;
        boost_libsuff_r=$boost_libsuff_r-d;
    fi
    boost_libsuff=$boost_libsuff-${WANT_BOOST_MAJOR}_${WANT_BOOST_MINOR}_$WANT_BOOST_SUB_MINOR
    boost_libsuff_r=$boost_libsuff_r-${WANT_BOOST_MAJOR}_${WANT_BOOST_MINOR}_$WANT_BOOST_SUB_MINOR
        ifelse([$2], , :, [$2])
      ],
      [
        AC_MSG_RESULT([no, version of installed Boost libraries is too old])
        ifelse([$3], , :, [$3])
      ])
  fi
  CPPFLAGS="$OLD_CPPFLAGS"
  AC_LANG_RESTORE
])

dnl RS_BOOST_THREAD([ACTION-IF-FOUND], [ACTION-IF-NOT-FOUND])
dnl Test for the Boost thread library
dnl Defines
dnl   BOOST_LDFLAGS to the set of flags required to compile boost_thread
AC_DEFUN([RS_BOOST_THREAD], 
[
    AC_REQUIRE([RS_BOOST])
  AC_MSG_CHECKING([whether we can use boost_thread library])
  AC_LANG_SAVE
  AC_LANG_CPLUSPLUS
  OLD_CPPFLAGS="$CPPFLAGS"
  CPPFLAGS="$BOOST_CPPFLAGS -D_REENTRANT"
  OLD_LIBS="$LIBS"
  LIBS="-lboost_thread-$boost_libsuff_r"
    AC_TRY_LINK(
        [ 
        #include <boost/thread.hpp> 
        bool bRet = 0;
        void thdfunc() { bRet = 1; }
        ],
        [
        boost::thread thrd(&thdfunc);
        thrd.join();
        return bRet == 1;
        ], 
        [
        AC_MSG_RESULT([yes])
        ifelse([$1], , :, [$1])
        ],
        [ 
		dnl Try without $boost_libsuff_r
	  LIBS="-lboost_thread"
	    AC_TRY_LINK(
	        [ 
	        #include <boost/thread.hpp> 
	        bool bRet = 0;
	        void thdfunc() { bRet = 1; }
	        ],
	        [
	        boost::thread thrd(&thdfunc);
	        thrd.join();
	        return bRet == 1;
	        ], 
	        [
	        AC_MSG_RESULT([yes])
	        ifelse([$1], , :, [$1])
	        ],
	        [ 
	        AC_MSG_RESULT([no])
	        ifelse([$2], , :, [$2])
	        ])
        ])

    AC_SUBST(BOOST_CPPFLAGS)
    AC_SUBST(BOOST_LIBS_R)
    BOOST_CPPFLAGS="$CPPFLAGS"
    BOOST_LIBS_R="$LIBS $BOOST_LIBS_R"
    CPPFLAGS="$OLD_CPPFLAGS"
    LIBS="$OLD_LIBS"
    AC_LANG_RESTORE
])
        
dnl RS_BOOST_DATETIME([ACTION-IF-FOUND], [ACTION-IF-NOT-FOUND])
dnl Test for the Boost datetime library
dnl Defines
dnl   BOOST_LDFLAGS to the set of flags required to compile boost_datetime
AC_DEFUN([RS_BOOST_DATETIME], 
[
    AC_REQUIRE([RS_BOOST])
  AC_MSG_CHECKING([whether we can use boost_datetime library])
  AC_LANG_SAVE
  AC_LANG_CPLUSPLUS
  OLD_CPPFLAGS="$CPPFLAGS"
  CPPFLAGS="$BOOST_CPPFLAGS"
  OLD_LIBS="$LIBS"
  LIBS="-lboost_date_time-$boost_libsuff"
    AC_TRY_LINK(
        [ 
        #include <boost/date_time/gregorian/gregorian.hpp> 
        ],
        [
        using namespace boost::gregorian;
        date d = from_string("1978-01-27");
        return d == boost::gregorian::date(1978, Jan, 27);
        ], 
        [
        AC_MSG_RESULT([yes])
        ifelse([$1], , :, [$1])
        ],
        [ 
		dnl Try without $boost_libsuff
		LIBS="-lboost_date_time"
	    AC_TRY_LINK(
	        [ 
	        #include <boost/date_time/gregorian/gregorian.hpp> 
	        ],
	        [
	        using namespace boost::gregorian;
	        date d = from_string("1978-01-27");
	        return d == boost::gregorian::date(1978, Jan, 27);
	        ], 
	        [
	        AC_MSG_RESULT([yes])
	        ifelse([$1], , :, [$1])
	        ],
	        [ 
	        AC_MSG_RESULT([no])
	        ifelse([$2], , :, [$2])
	        ])
        ])

    AC_SUBST(BOOST_CPPFLAGS)
    AC_SUBST(BOOST_LIBS)
    AC_SUBST(BOOST_LIBS_R)
    BOOST_CPPFLAGS="$CPPFLAGS"
    BOOST_LIBS="$LIBS $BOOST_LIBS"
    BOOST_LIBS_R="$LIBS $BOOST_LIBS_R"
    CPPFLAGS="$OLD_CPPFLAGS"
    LIBS=$OLD_LIBS
    AC_LANG_RESTORE
])
        
dnl RS_BOOST_REGEX([ACTION-IF-FOUND], [ACTION-IF-NOT-FOUND])
dnl Test for the Boost regex library
dnl Defines
dnl   BOOST_LDFLAGS to the set of flags required to compile boost_datetime
AC_DEFUN([RS_BOOST_REGEX], 
[
    AC_REQUIRE([RS_BOOST])
  AC_MSG_CHECKING([whether we can use boost_regex library])
  AC_LANG_SAVE
  AC_LANG_CPLUSPLUS
  OLD_CPPFLAGS="$CPPFLAGS"
  CPPFLAGS="$BOOST_CPPFLAGS"
  OLD_LIBS="$LIBS"
  LIBS="-lboost_regex-$boost_libsuff"
    AC_TRY_LINK(
        [ 
        #include <boost/regex.hpp> 
        ],
        [
        using namespace boost;
        cmatch what;
        if(!regex_match("27/01/78",what,regex("(\\\d+)/(\\\d+)/(\\\d+)")))
            return 0;

        return what[1]=="27" && what[2]=="01" && what[3]=="78";
        ], 
        [
        AC_MSG_RESULT([yes])
        ifelse([$1], , :, [$1])
        ],
        [ 
		dnl Try it without the $boost_libsuff
		LIBS="-lboost_regex"
		    AC_TRY_LINK(
		        [ 
		        #include <boost/regex.hpp> 
		        ],
		        [
		        using namespace boost;
		        cmatch what;
		        if(!regex_match("27/01/78",what,regex("(\\\d+)/(\\\d+)/(\\\d+)")))
		            return 0;

		        return what[1]=="27" && what[2]=="01" && what[3]=="78";
		        ], 
		        [
		        AC_MSG_RESULT([yes])
		        ifelse([$1], , :, [$1])
		        ],
		        [ 
		        AC_MSG_RESULT([no])
		        ifelse([$2], , :, [$2])
		        ])
        ])

    AC_SUBST(BOOST_CPPFLAGS)
    AC_SUBST(BOOST_LIBS)
    AC_SUBST(BOOST_LIBS_R)
    BOOST_CPPFLAGS="$CPPFLAGS"
    BOOST_LIBS="$LIBS $BOOST_LIBS"
    BOOST_LIBS_R="$LIBS $BOOST_LIBS_R"
    CPPFLAGS="$OLD_CPPFLAGS"
    LIBS="$OLD_LIBS"
    AC_LANG_RESTORE
])

dnl RS_BOOST_SIGNALS([ACTION-IF-FOUND], [ACTION-IF-NOT-FOUND])
dnl Test for the Boost signals library
AC_DEFUN([RS_BOOST_SIGNALS], 
[
    AC_REQUIRE([RS_BOOST])
  AC_MSG_CHECKING([whether we can use boost_signals library])
  AC_LANG_SAVE
  AC_LANG_CPLUSPLUS
  OLD_CPPFLAGS="$CPPFLAGS"
  CPPFLAGS="$BOOST_CPPFLAGS"
  OLD_LIBS="$LIBS"
  LIBS="-lboost_signals-$boost_libsuff"
    AC_TRY_LINK(
        [ 
        #include <boost/signals.hpp> 
        ],
        [
        boost::signal<void ()> sig;
        return 0;
        ], 
        [
        AC_MSG_RESULT([yes])
        ifelse([$1], , :, [$1])
        ],
        [ 
        LIBS="-lboost_signals"
	    AC_TRY_LINK(
	        [ 
	        #include <boost/signals.hpp> 
	        ],
	        [
	        boost::signal<void ()> sig;
	        return 0;
	        ], 
	        [
	        AC_MSG_RESULT([yes])
	        ifelse([$1], , :, [$1])
	        ],
	        [ 
	        AC_MSG_RESULT([no])
	        ifelse([$2], , :, [$2])
	        ])
        ])

    AC_SUBST(BOOST_CPPFLAGS)
    AC_SUBST(BOOST_LIBS)
    AC_SUBST(BOOST_LIBS_R)
    BOOST_CPPFLAGS="$CPPFLAGS"
    BOOST_LIBS="$LIBS $BOOST_LIBS"
    BOOST_LIBS_R="$LIBS $BOOST_LIBS_R"
    CPPFLAGS="$OLD_CPPFLAGS"
    LIBS="$OLD_LIBS"
    AC_LANG_RESTORE
])

dnl RS_BOOST_PROGRAM_OPTIONS([ACTION-IF-FOUND], [ACTION-IF-NOT-FOUND])
dnl Test for the Boost program_options library
dnl Defines
dnl   BOOST_LDFLAGS to the set of flags required to compile boost_datetime
AC_DEFUN([RS_BOOST_PROGRAM_OPTIONS], 
[
    AC_REQUIRE([RS_BOOST])
  AC_MSG_CHECKING([whether we can use boost_program_options library])
  AC_LANG_SAVE
  AC_LANG_CPLUSPLUS
  OLD_CPPFLAGS="$CPPFLAGS"
  CPPFLAGS="$BOOST_CPPFLAGS"
  OLD_LIBS="$LIBS"
  LIBS="-lboost_program_options-$boost_libsuff"
    AC_TRY_LINK(
        [ 
        #include <boost/program_options.hpp> 
        ],
        [
        using namespace boost::program_options;
        return 0;
        ], 
        [
        AC_MSG_RESULT([yes])
        ifelse([$1], , :, [$1])
        ],
        [ 
        LIBS="-lboost_program_options"
	    AC_TRY_LINK(
	        [ 
	        #include <boost/program_options.hpp> 
	        ],
	        [
	        using namespace boost::program_options;
	        return 0;
	        ], 
	        [
	        AC_MSG_RESULT([yes])
	        ifelse([$1], , :, [$1])
	        ],
	        [ 
	        AC_MSG_RESULT([no])
	        ifelse([$2], , :, [$2])
	        ])
        ])

    AC_SUBST(BOOST_CPPFLAGS)
    AC_SUBST(BOOST_LIBS)
    AC_SUBST(BOOST_LIBS_R)
    BOOST_CPPFLAGS="$CPPFLAGS"
    BOOST_LIBS="$LIBS $BOOST_LIBS"
    BOOST_LIBS_R="$LIBS $BOOST_LIBS_R"
    CPPFLAGS="$OLD_CPPFLAGS"
    LIBS="$OLD_LIBS"
    AC_LANG_RESTORE
])

dnl RS_BOOST_IOSTREAMS([ACTION-IF-FOUND], [ACTION-IF-NOT-FOUND])
dnl Test for the Boost iostreams library
AC_DEFUN([RS_BOOST_IOSTREAMS], 
[
    AC_REQUIRE([RS_BOOST])
  AC_MSG_CHECKING([whether we can use boost_iostreams library])
  AC_LANG_SAVE
  AC_LANG_CPLUSPLUS
  OLD_CPPFLAGS="$CPPFLAGS"
  CPPFLAGS="$BOOST_CPPFLAGS"
  OLD_LIBS="$LIBS"
  LIBS="-lboost_iostreams-$boost_libsuff"
    AC_TRY_LINK(
        [ 
        #include <boost/iostreams/stream.hpp> 
        ],
        [
        using namespace boost::iostreams;
        return 0;
        ], 
        [
        AC_MSG_RESULT([yes])
        ifelse([$1], , :, [$1])
        ],
        [ 
        LIBS="-lboost_iostreams"
	    AC_TRY_LINK(
	        [ 
	        #include <boost/iostreams/stream.hpp> 
	        ],
	        [
	        using namespace boost::iostreams;
	        return 0;
	        ], 
	        [
	        AC_MSG_RESULT([yes])
	        ifelse([$1], , :, [$1])
	        ],
	        [ 
	        AC_MSG_RESULT([no])
	        ifelse([$2], , :, [$2])
	        ])
        ])

    AC_SUBST(BOOST_CPPFLAGS)
    AC_SUBST(BOOST_LIBS)
    AC_SUBST(BOOST_LIBS_R)
    BOOST_CPPFLAGS="$CPPFLAGS"
    BOOST_LIBS="$LIBS $BOOST_LIBS"
    BOOST_LIBS_R="$LIBS $BOOST_LIBS_R"
    CPPFLAGS="$OLD_CPPFLAGS"
    LIBS="$OLD_LIBS"
    AC_LANG_RESTORE
])

dnl RS_BOOST_FILESYSTEM([ACTION-IF-FOUND], [ACTION-IF-NOT-FOUND])
dnl Test for the Boost filesystem library
AC_DEFUN([RS_BOOST_FILESYSTEM], 
[
    AC_REQUIRE([RS_BOOST])
  AC_MSG_CHECKING([whether we can use boost_filesystem library])
  AC_LANG_SAVE
  AC_LANG_CPLUSPLUS
  OLD_CPPFLAGS="$CPPFLAGS"
  CPPFLAGS="$BOOST_CPPFLAGS"
  OLD_LIBS="$LIBS"
  LIBS="-lboost_filesystem-$boost_libsuff"
    AC_TRY_LINK(
        [ 
        #include <boost/filesystem/path.hpp> 
        ],
        [
        using namespace boost::filesystem;
        return 0;
        ], 
        [
        AC_MSG_RESULT([yes])
        ifelse([$1], , :, [$1])
        ],
        [ 
        LIBS="-lboost_filesystem"
	    AC_TRY_LINK(
	        [ 
	        #include <boost/filesystem/path.hpp> 
	        ],
	        [
	        using namespace boost::filesystem;
	        return 0;
	        ], 
	        [
	        AC_MSG_RESULT([yes])
	        ifelse([$1], , :, [$1])
	        ],
	        [ 
	        AC_MSG_RESULT([no])
	        ifelse([$2], , :, [$2])
	        ])
        ])

    AC_SUBST(BOOST_CPPFLAGS)
    AC_SUBST(BOOST_LIBS)
    AC_SUBST(BOOST_LIBS_R)
    BOOST_CPPFLAGS="$CPPFLAGS"
    BOOST_LIBS="$LIBS $BOOST_LIBS"
    BOOST_LIBS_R="$LIBS $BOOST_LIBS_R"
    CPPFLAGS="$OLD_CPPFLAGS"
    LIBS="$OLD_LIBS"
    AC_LANG_RESTORE
])
