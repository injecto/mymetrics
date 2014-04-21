#ifdef STANDARD
/* STANDARD is defined, don't use any mysql functions */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#ifdef __WIN__
typedef unsigned __int64 ulonglong;	/* Microsofts 64 bit types */
typedef __int64 longlong;
#else
typedef unsigned long long ulonglong;
typedef long long longlong;
#endif /*__WIN__*/
#else
#include <my_global.h>
#include <my_sys.h>
#if defined(MYSQL_SERVER)
#include <m_string.h>		/* To get strmov() */
#else
/* when compiled as standalone */
#include <string.h>
#define strmov(a,b) stpcpy(a,b)
#define bzero(a,b) memset(a,0,b)
#endif
#endif
#include <mysql.h>
#include <ctype.h>

#include "levenshtein.h"
#include "dmetaphone.h"
#include "jarowinkler.h"
#include "dice.h"

#include <clocale>
#include <cstdlib>
#include <cassert>
#include <cmath>
#include <mutex>

using namespace std;

extern "C" {
  longlong levenshtein(UDF_INIT *initid, UDF_ARGS *args, char *is_null, char *error);
  my_bool levenshtein_init(UDF_INIT *initid, UDF_ARGS *args, char *message);
  void levenshtein_deinit(UDF_INIT *initid);
  
  longlong double_metaphone_eq(UDF_INIT *initid, UDF_ARGS *args, char *is_null, char *error);
  my_bool double_metaphone_eq_init(UDF_INIT *initid, UDF_ARGS *args, char *message);
  void double_metaphone_eq_deinit(UDF_INIT *initid);

  double jaro_winkler(UDF_INIT *initid, UDF_ARGS *args, char *is_null, char *error);
  my_bool jaro_winkler_init(UDF_INIT *initid, UDF_ARGS *args, char *message);
  void jaro_winkler_deinit(UDF_INIT *initid);

  double dice(UDF_INIT *initid, UDF_ARGS *args, char *is_null, char *error);
  my_bool dice_init(UDF_INIT *initid, UDF_ARGS *args, char *message);
  void dice_deinit(UDF_INIT *initid);

}
  
  mutex locale_mx;

  my_bool init(UDF_INIT *initid, UDF_ARGS *args, char *message) {
    lock_guard<mutex> guard(locale_mx);
    
    if (!strcmp("C", setlocale(LC_ALL, 0)) && !setlocale(LC_ALL, "en_US.UTF-8")) {
      strcpy(message, "Can't change default locale to UTF-8");
      return 1;
    }
    
    initid->maybe_null = 0;
    if (args->arg_count != 2 || args->arg_type[0] != STRING_RESULT || args->arg_type[1] != STRING_RESULT) {
      strcpy(message, "This function requires two string arguments");
      return 1;
    }

    if (!args->args[0] || !args->args[1]) {
      strcpy(message, "This function requires non-NULL arguments");
      return 1;
    }
    
    return 0;
  }

  wstring from_cstr(const char* s, size_t l) {
    wstring ws(l, L' ');
    mbstowcs(&ws[0], s, l);
    return ws;
  }

  longlong levenshtein(UDF_INIT *initid, UDF_ARGS *args, char *is_null, char *error) {
    wstring s1 = from_cstr(args->args[0], args->lengths[0]);
    wstring s2 = from_cstr(args->args[1], args->lengths[1]);
    return levenshtein_dist(s1.c_str(), s2.c_str());
  }

  my_bool levenshtein_init(UDF_INIT *initid, UDF_ARGS *args, char *message) {
    return init(initid, args, message);
  }

  void levenshtein_deinit(UDF_INIT *initid) {}

  longlong double_metaphone_eq(UDF_INIT *initid, UDF_ARGS *args, char *is_null, char *error) {
    wstring s1 = from_cstr(args->args[0], args->lengths[0]);
    wstring s2 = from_cstr(args->args[1], args->lengths[1]);
    return dmetaphone_eq(s1.c_str(), s2.c_str());
  }

  my_bool double_metaphone_eq_init(UDF_INIT *initid, UDF_ARGS *args, char *message) {
    return init(initid, args, message);
  }

  void double_metaphone_eq_deinit(UDF_INIT *initid) {}

  double jaro_winkler(UDF_INIT *initid, UDF_ARGS *args, char *is_null, char *error) {
    wstring s1 = from_cstr(args->args[0], args->lengths[0]);
    wstring s2 = from_cstr(args->args[1], args->lengths[1]);
    return jaro_winkler_dist(s1.c_str(), s2.c_str());
  }

  my_bool jaro_winkler_init(UDF_INIT *initid, UDF_ARGS *args, char *message) {
    return init(initid, args, message);
  }

  void jaro_winkler_deinit(UDF_INIT *initid) {}

  double dice(UDF_INIT *initid, UDF_ARGS *args, char *is_null, char *error) {
    wstring s1 = from_cstr(args->args[0], args->lengths[0]);
    wstring s2 = from_cstr(args->args[1], args->lengths[1]);
    return dice_coeff(s1.c_str(), s2.c_str());
  }

  my_bool dice_init(UDF_INIT *initid, UDF_ARGS *args, char *message) {
    return init(initid, args, message);
  }

  void dice_deinit(UDF_INIT *initid) {}

int main(int argc, const char* argv[]) {
  assert(levenshtein_dist(L"ООО Рога и копыта", L"Рога и копыта, ООО") == 9);

  assert(dmetaphone_eq(L"mère", L"mer"));
  assert(dmetaphone_eq(L"peke", L"pique"));
  assert(!dmetaphone_eq(L"bloat", L"float"));

  assert(floor(100 * jaro_winkler_dist(L"ООО Рага и копыта", L"Рога и копыта, ООО")) == 70.0);

  assert(floor(100 * dice_coeff(L"ООО Рага и копыта", L"Рога и копыта, ООО")) == 70.0);
  return 0;
}

