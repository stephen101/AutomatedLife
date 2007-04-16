AC_DEFUN([AC_CXX_HAVE_TR1_UNORDERED_MAP],
[AC_CACHE_CHECK(whether the compiler has tr1/unordered_map and has working swap(),
ac_cv_cxx_have_tr1_unordered_map,
[AC_REQUIRE([AC_CXX_NAMESPACES])
  AC_LANG_SAVE
  AC_LANG_CPLUSPLUS
  AC_TRY_COMPILE([#include <tr1/unordered_map>
#ifdef HAVE_NAMESPACES
  using namespace std;
#endif],[tr1::unordered_map<int, int> t1, t2; t1 = t2; return 0;],
  ac_cv_cxx_have_tr1_unordered_map=yes, ac_cv_cxx_have_tr1_unordered_map=no)
  AC_LANG_RESTORE
  ])
  if test "$ac_cv_cxx_have_tr1_unordered_map" = yes; then
  AC_DEFINE(HAVE_TR1_UNORDERED_MAP,,[define if the compiler has tr1/unordered_map])
  fi
])
