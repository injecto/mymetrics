/*
  This module implements a "sounds like" algorithm developed by Lawrence Philips 
  which he published in the June, 2000 issue of C/C++ Users Journal. 
  Double Metaphone is an improved version of Philips' original Metaphone algorithm.

  Copyright 2007, Stephen Lacy slacy@slacy.com

  This code is a derivative work from an implementation by Maurice Aubrey maurice@hevanet.com, 
  and modified to use STL vector and string classes instead of bare pointers.

  This code is edited by Vladimir Glushenkov vio@xakep.ru to UTF-8 strings support.
*/

#include <algorithm>
#include <cwctype>
#include <cstdarg>
#include "dmetaphone.h"

using namespace std;

const unsigned int max_length = 32;

void make_upper(wstring &s) {
  for (unsigned int i = 0; i < s.length(); i++) {
    s[i] = towupper(s[i]);
  }
}

int is_vowel(wstring &s, unsigned int pos)
{
  if (pos >= s.length())
    return 0;

  wchar_t c = s[pos];
  if ((c == L'A') || (c == L'E') || (c == L'I') || (c == L'O') ||
      (c == L'U')  || (c == L'Y')) {
    return 1;
  }

  return 0;
}


int is_slavo_germanic(const wstring &s)
{
  if (s.find(L"W") != wstring::npos
      || s.find(L"K") != wstring::npos
      || s.find(L"CZ") != wstring::npos
      || s.find(L"WITZ") != wstring::npos)
    return 1;

  return 0;
}


wchar_t get_at(const wstring &s, unsigned int pos)
{
  if (pos >= s.length()) {
    return L'\0';
  }

  return s[pos];
}


void set_at(wstring &s, unsigned int pos, wchar_t c)
{
  if (pos >= s.length()) {
    return;
  }

  s[pos] = c;
}


/*
  Caveats: the START value is 0 based
*/
int string_at(wstring &s, unsigned int start, unsigned int length, ...)
{
  wchar_t *test;
  va_list ap;

  if (start >= s.length()) {
    return 0;
  }

  va_start(ap, length);

  do {
    test = va_arg(ap, wchar_t*);
    if (*test && (s.compare(start, length, test) == 0)) {
      return 1;
    }
  } while (wcscmp(test, L"") == 0);

  va_end(ap);

  return 0;
}

vector<wstring> dmetaphone(const wstring &str)
{
  vector<wstring> codes;
  int        length;
  wstring original;
  wstring primary;
  wstring secondary;
  int        current;
  int        last;

  current = 0;
  /* we need the real length and last prior to padding */
  length  = str.length();
  last    = length - 1;
  original = str; // make a copy
  /* Pad original so we can index beyond end */
  original += L"     ";

  primary = L"";
  secondary = L"";

  make_upper(original);

  /* skip these when at start of word */
  if (string_at(original, 0, 2, L"GN", L"KN", L"PN", L"WR", L"PS", L"")) {
    current += 1;
  }

  /* Initial 'X' is pronounced 'Z' e.g. 'Xavier' */
  if (get_at(original, 0) == L'X') {
    primary += L"S";	/* 'Z' maps to 'S' */
    secondary += L"S";
    current += 1;
  }

  /* main loop */
  while ((primary.length() < max_length) || (secondary.length() < max_length)) {
    if (current >= length) {
      break;
    }

    switch (get_at(original, current)) {
    case L'A':
    case L'E':
    case L'I':
    case L'O':
    case L'U':
    case L'Y':
      if (current == 0) {
        /* all init vowels now map to 'A' */
        primary += L"A";
        secondary += L"A";
      }
      current += 1;
      break;

    case L'B':
      /* "-mb", e.g", "dumb", already skipped over... */
      primary += L"P";
      secondary += L"P";

      if (get_at(original, current + 1) == L'B')
        current += 2;
      else
        current += 1;
      break;

    case L'Ç':
      primary += L"S";
      secondary += L"S";
      current += 1;
      break;

    case L'C':
      /* various germanic */
      if ((current > 1) &&
          !is_vowel(original, current - 2) &&
          string_at(original, (current - 1), 3, L"ACH", L"") &&
          ((get_at(original, current + 2) != L'I') &&
           ((get_at(original, current + 2) != L'E') ||
            string_at(original, (current - 2), 6, L"BACHER", L"MACHER", L"")))) {
        primary += L"K";
        secondary += L"K";
        current += 2;
        break;
      }

      /* special case 'caesar' */
      if ((current == 0) && string_at(original, current, 6, L"CAESAR", L"")) {
        primary += L"S";
        secondary += L"S";
        current += 2;
        break;
      }

      /* italian 'chianti' */
      if (string_at(original, current, 4, L"CHIA", L"")) {
        primary += L"K";
        secondary += L"K";
        current += 2;
        break;
      }

      if (string_at(original, current, 2, L"CH", L"")) {
        /* find 'michael' */
        if ((current > 0) && string_at(original, current, 4, L"CHAE", L"")) {
          primary += L"K";
          secondary += L"X";
          current += 2;
          break;
        }

        /* greek roots e.g. 'chemistry', 'chorus' */
        if ((current == 0) &&
            (string_at(original, (current + 1), 5,
                     L"HARAC", L"HARIS", L"") ||
             string_at(original, (current + 1), 3,
                     L"HOR", L"HYM", L"HIA", L"HEM", L"")) &&
            !string_at(original, 0, 5, L"CHORE", L"")) {
          primary += L"K";
          secondary += L"K";
          current += 2;
          break;
        }

        /* germanic, greek, or otherwise 'ch' for 'kh' sound */
        if ((string_at(original, 0, 4, L"VAN ", L"VON ", L"") ||
             string_at(original, 0, 3, L"SCH", L"")) ||
            /*  'architect but not 'arch', 'orchestra', 'orchid' */
            string_at(original, (current - 2), 6,
                    L"ORCHES", L"ARCHIT", L"ORCHID", L"") ||
            string_at(original, (current + 2), 1,
                    L"T", L"S", L"") ||
            ((string_at(original, (current - 1), 1,
                      L"A", L"O", L"U", L"E", L"") ||
              (current == 0)) &&
             /* e.g., 'wachtler', 'wechsler', but not 'tichner' */
             string_at(original, (current + 2), 1, L"L", L"R",
                     L"N", L"M", L"B", L"H", L"F", L"V", L"W", L" ", L""))) {
          primary += L"K";
          secondary += L"K";
        } else {
          if (current > 0) {
            if (string_at(original, 0, 2, L"MC", L"")) {
              /* e.g., "McHugh" */
              primary += L"K";
              secondary += L"K";
            } else {
              primary += L"X";
              secondary += L"K";
            }
          } else {
            primary += L"X";
            secondary += L"X";
          }
        }
        current += 2;
        break;
      }
      /* e.g, 'czerny' */
      if (string_at(original, current, 2, L"CZ", L"") &&
          !string_at(original, (current - 2), 4, L"WICZ", L"")) {
        primary += L"S";
        secondary += L"X";
        current += 2;
        break;
      }

      /* e.g., 'focaccia' */
      if (string_at(original, (current + 1), 3, L"CIA", L"")) {
        primary += L"X";
        secondary += L"X";
        current += 3;
        break;
      }

      /* double 'C', but not if e.g. 'McClellan' */
      if (string_at(original, current, 2, L"CC", L"") &&
          !((current == 1) && (get_at(original, 0) == L'M'))) {
        /* 'bellocchio' but not 'bacchus' */
        if (string_at(original, (current + 2), 1, L"I", L"E", L"H", L"") &&
            !string_at(original, (current + 2), 2, L"HU", L"")) {
          /* 'accident', 'accede' 'succeed' */
          if (((current == 1) && (get_at(original, current - 1) == L'A')) ||
              string_at(original, (current - 1), 5, L"UCCEE", L"UCCES", L"")) {
            primary += L"KS";
            secondary += L"KS";
            /* 'bacci', 'bertucci', other italian */
          } else {
            primary += L"X";
            secondary += L"X";
          }
          current += 3;
          break;
        } else {  /* Pierce's rule */
          primary += L"K";
          secondary += L"K";
          current += 2;
          break;
        }
      }

      if (string_at(original, current, 2, L"CK", L"CG", L"CQ", L"")) {
        primary += L"K";
        secondary += L"K";
        current += 2;
        break;
      }

      if (string_at(original, current, 2, L"CI", L"CE", L"CY", L"")) {
        /* italian vs. english */
        if (string_at(original, current, 3, L"CIO", L"CIE", L"CIA", L"")) {
          primary += L"S";
          secondary += L"X";
        } else {
          primary += L"S";
          secondary += L"S";
        }
        current += 2;
        break;
      }

      /* else */
      primary += L"K";
      secondary += L"K";

      /* name sent in 'mac caffrey', 'mac gregor */
      if (string_at(original, (current + 1), 2, L" C", L" Q", L" G", L""))
        current += 3;
      else
        if (string_at(original, (current + 1), 1, L"C", L"K", L"Q", L"") &&
            !string_at(original, (current + 1), 2, L"CE", L"CI", L""))
          current += 2;
        else
          current += 1;
      break;

    case L'D':
      if (string_at(original, current, 2, L"DG", L"")) {
        if (string_at(original, (current + 2), 1, L"I", L"E", L"Y", L"")) {
          /* e.g. 'edge' */
          primary += L"J";
          secondary += L"J";
          current += 3;
          break;
        } else {
          /* e.g. 'edgar' */
          primary += L"TK";
          secondary += L"TK";
          current += 2;
          break;
        }
      }

      if (string_at(original, current, 2, L"DT", L"DD", L"")) {
        primary += L"T";
        secondary += L"T";
        current += 2;
        break;
      }

      /* else */
      primary += L"T";
      secondary += L"T";
      current += 1;
      break;

    case L'F':
      if (get_at(original, current + 1) == L'F')
        current += 2;
      else
        current += 1;
      primary += L"F";
      secondary += L"F";
      break;

    case L'G':
      if (get_at(original, current + 1) == L'H') {
        if ((current > 0) && !is_vowel(original, current - 1)) {
          primary += L"K";
          secondary += L"K";
          current += 2;
          break;
        }

        if (current < 3) {
          /* 'ghislane', ghiradelli */
          if (current == 0) {
            if (get_at(original, current + 2) == L'I') {
              primary += L"J";
              secondary += L"J";
            } else {
              primary += L"K";
              secondary += L"K";
            }
            current += 2;
            break;
          }
        }
        /* Parker's rule (with some further refinements) - e.g., 'hugh' */
        if (((current > 1) &&
             string_at(original, (current - 2), 1, L"B", L"H", L"D", L"")) ||
            /* e.g., 'bough' */
            ((current > 2) &&
             string_at(original, (current - 3), 1, L"B", L"H", L"D", L"")) ||
            /* e.g., 'broughton' */
            ((current > 3) &&
             string_at(original, (current - 4), 1, L"B", L"H", L""))) {
          current += 2;
          break;
        } else {
          /* e.g., 'laugh', 'McLaughlin', 'cough', 'gough', 'rough', 'tough' */
          if ((current > 2) &&
              (get_at(original, current - 1) == L'U') &&
              string_at(original, (current - 3), 1, L"C",
                      L"G", L"L", L"R", L"T", L"")) {
            primary += L"F";
            secondary += L"F";
          } else if ((current > 0) &&
                     get_at(original, current - 1) != L'I') {
            primary += L"K";
            secondary += L"K";
          }

          current += 2;
          break;
        }
      }

      if (get_at(original, current + 1) == L'N') {
        if ((current == 1) &&
            is_vowel(original, 0) &&
            !is_slavo_germanic(original)) {
          primary += L"KN";
          secondary += L"N";
        } else
          /* not e.g. 'cagney' */
          if (!string_at(original, (current + 2), 2, L"EY", L"") &&
              (get_at(original, current + 1) != L'Y') &&
              !is_slavo_germanic(original)) {
            primary += L"N";
            secondary += L"KN";
          } else {
            primary += L"KN";
            secondary += L"KN";
          }
        current += 2;
        break;
      }

      /* 'tagliaro' */
      if (string_at(original, (current + 1), 2, L"LI", L"") &&
          !is_slavo_germanic(original)) {
        primary += L"KL";
        secondary += L"L";
        current += 2;
        break;
      }

      /* -ges-,-gep-,-gel-, -gie- at beginning */
      if ((current == 0) &&
          ((get_at(original, current + 1) == L'Y') ||
           string_at(original, (current + 1), 2, L"ES", L"EP",
                   L"EB", L"EL", L"EY", L"IB", L"IL", L"IN", L"IE",
                   L"EI", L"ER", L""))) {
        primary += L"K";
        secondary += L"J";
        current += 2;
        break;
      }

      /*  -ger-,  -gy- */
      if ((string_at(original, (current + 1), 2, L"ER", L"") ||
           (get_at(original, current + 1) == L'Y')) &&
          !string_at(original, 0, 6, L"DANGER", L"RANGER", L"MANGER", L"") &&
          !string_at(original, (current - 1), 1, L"E", L"I", L"") &&
          !string_at(original, (current - 1), 3, L"RGY", L"OGY", L"")) {
        primary += L"K";
        secondary += L"J";
        current += 2;
        break;
      }

      /*  italian e.g, 'biaggi' */
      if (string_at(original, (current + 1), 1, L"E", L"I", L"Y", L"") ||
          string_at(original, (current - 1), 4, L"AGGI", L"OGGI", L"")) {
        /* obvious germanic */
        if ((string_at(original, 0, 4, L"VAN ", L"VON ", L"") ||
             string_at(original, 0, 3, L"SCH", L"")) ||
            string_at(original, (current + 1), 2, L"ET", L""))
          {
            primary += L"K";
            secondary += L"K";
          } else {
          /* always soft if french ending */
          if (string_at(original, (current + 1), 4, L"IER ", L"")) {
            primary += L"J";
            secondary += L"J";
          } else {
            primary += L"J";
            secondary += L"K";
          }
        }
        current += 2;
        break;
      }

      if (get_at(original, current + 1) == L'G')
        current += 2;
      else
        current += 1;
      primary += L"K";
      secondary += L"K";
      break;

    case L'H':
      /* only keep if first & before vowel or btw. 2 vowels */
      if (((current == 0) ||
           is_vowel(original, current - 1)) &&
          is_vowel(original, current + 1)) {
        primary += L"H";
        secondary += L"H";
        current += 2;
      }
      else		/* also takes care of 'HH' */
        current += 1;
      break;

    case L'J':
      /* obvious spanish, 'jose', 'san jacinto' */
      if (string_at(original, current, 4, L"JOSE", L"") ||
          string_at(original, 0, 4, L"SAN ", L"")) {
        if (((current == 0) && (get_at(original, current + 4) == L' ')) ||
            string_at(original, 0, 4, L"SAN ", L"")) {
          primary += L"H";
          secondary += L"H";
        } else {
          primary += L"J";
          secondary += L"H";
        }
        current += 1;
        break;
      }

      if ((current == 0) && !string_at(original, current, 4, L"JOSE", L"")) {
        primary += L"J";	/* Yankelovich/Jankelowicz */
        secondary += L"A";
      } else {
        /* spanish pron. of e.g. 'bajador' */
        if (is_vowel(original, current - 1) &&
            !is_slavo_germanic(original) &&
            ((get_at(original, current + 1) == L'A') ||
             (get_at(original, current + 1) == L'O'))) {
          primary += L"J";
          secondary += L"H";
        } else {
          if (current == last) {
            primary += L"J";
            secondary += L"";
          } else {
            if (!string_at(original, (current + 1), 1,
                         L"L", L"T", L"K", L"S", L"N", L"M", L"B", L"Z", L"") &&
                !string_at(original, (current - 1), 1, L"S", L"K", L"L", L"")) {
              primary += L"J";
              secondary += L"J";
            }
          }
        }
      }

      if (get_at(original, current + 1) == L'J')	/* it could happen! */
        current += 2;
      else
        current += 1;
      break;

    case L'K':
      if (get_at(original, current + 1) == L'K')
        current += 2;
      else
        current += 1;
      primary += L"K";
      secondary += L"K";
      break;

    case L'L':
      if (get_at(original, current + 1) == L'L') {
        /* spanish e.g. 'cabrillo', 'gallegos' */
        if (((current == (length - 3)) &&
             string_at(original, (current - 1), 4,
                     L"ILLO", L"ILLA", L"ALLE", L"")) ||
            ((string_at(original, (last - 1), 2, L"AS", L"OS", L"") ||
              string_at(original, last, 1, L"A", L"O", L"")) &&
             string_at(original, (current - 1), 4, L"ALLE", L""))) {
          primary += L"L";
          secondary += L"";
          current += 2;
          break;
        }
        current += 2;
      }
      else
        current += 1;
      primary += L"L";
      secondary += L"L";
      break;

    case L'M':
      if ((string_at(original, (current - 1), 3, L"UMB", L"") &&
           (((current + 1) == last) ||
            string_at(original, (current + 2), 2, L"ER", L""))) ||
          /* 'dumb','thumb' */
          (get_at(original, current + 1) == L'M')) {
        current += 2;
      } else {
        current += 1;
      }
      primary += L"M";
      secondary += L"M";
      break;

    case L'N':
      if (get_at(original, current + 1) == L'N') {
        current += 2;
      } else {
        current += 1;
      }
      primary += L"N";
      secondary += L"N";
      break;

    case L'Ñ':
      current += 1;
      primary += L"N";
      secondary += L"N";
      break;

    case L'P':
      if (get_at(original, current + 1) == L'H') {
        primary += L"F";
        secondary += L"F";
        current += 2;
        break;
      }

      /* also account for "campbell", "raspberry" */
      if (string_at(original, (current + 1), 1, L"P", L"B", L""))
        current += 2;
      else
        current += 1;
      primary += L"P";
      secondary += L"P";
      break;

    case L'Q':
      if (get_at(original, current + 1) == L'Q')
        current += 2;
      else
        current += 1;
      primary += L"K";
      secondary += L"K";
      break;

    case L'R':
      /* french e.g. 'rogier', but exclude 'hochmeier' */
      if ((current == last) &&
          !is_slavo_germanic(original) &&
          string_at(original, (current - 2), 2, L"IE", L"") &&
          !string_at(original, (current - 4), 2, L"ME", L"MA", L"")) {
        primary += L"";
        secondary += L"R";
      } else {
        primary += L"R";
        secondary += L"R";
      }

      if (get_at(original, current + 1) == L'R')
        current += 2;
      else
        current += 1;
      break;

    case L'S':
      /* special cases 'island', 'isle', 'carlisle', 'carlysle' */
      if (string_at(original, (current - 1), 3, L"ISL", L"YSL", L"")) {
        current += 1;
        break;
      }

      /* special case 'sugar-' */
      if ((current == 0) && string_at(original, current, 5, L"SUGAR", L"")) {
        primary += L"X";
        secondary += L"S";
        current += 1;
        break;
      }

      if (string_at(original, current, 2, L"SH", L"")) {
        /* germanic */
        if (string_at(original, (current + 1), 4,
                    L"HEIM", L"HOEK", L"HOLM", L"HOLZ", L"")) {
          primary += L"S";
          secondary += L"S";
        } else {
          primary += L"X";
          secondary += L"X";
        }
        current += 2;
        break;
      }

      /* italian & armenian */
      if (string_at(original, current, 3, L"SIO", L"SIA", L"") ||
          string_at(original, current, 4, L"SIAN", L"")) {
        if (!is_slavo_germanic(original)) {
          primary += L"S";
          secondary += L"X";
        } else {
          primary += L"S";
          secondary += L"S";
        }
        current += 3;
        break;
      }

      /* german & anglicisations, e.g. 'smith' match 'schmidt', 'snider' match 'schneider'
         also, -sz- in slavic language altho in hungarian it is pronounced 's' */
      if (((current == 0) &&
           string_at(original, (current + 1), 1, L"M", L"N", L"L", L"W", L"")) ||
          string_at(original, (current + 1), 1, L"Z", L"")) {
        primary += L"S";
        secondary += L"X";
        if (string_at(original, (current + 1), 1, L"Z", L""))
          current += 2;
        else
          current += 1;
        break;
      }

      if (string_at(original, current, 2, L"SC", L"")) {
        /* Schlesinger's rule */
        if (get_at(original, current + 2) == L'H') {
          /* dutch origin, e.g. 'school', 'schooner' */
          if (string_at(original, (current + 3), 2,
                      L"OO", L"ER", L"EN", L"UY", L"ED", L"EM", L"")) {
            /* 'schermerhorn', 'schenker' */
            if (string_at(original, (current + 3), 2, L"ER", L"EN", L"")) {
              primary += L"X";
              secondary += L"SK";
            } else {
              primary += L"SK";
              secondary += L"SK";
            }
            current += 3;
            break;
          } else {
            if ((current == 0) && !is_vowel(original, 3) &&
                (get_at(original, 3) != L'W')) {
              primary += L"X";
              secondary += L"S";
            } else {
              primary += L"X";
              secondary += L"X";
            }
            current += 3;
            break;
          }
        }

        if (string_at(original, (current + 2), 1, L"I", L"E", L"Y", L"")) {
          primary += L"S";
          secondary += L"S";
          current += 3;
          break;
        }
        /* else */
        primary += L"SK";
        secondary += L"SK";
        current += 3;
        break;
      }

      /* french e.g. 'resnais', 'artois' */
      if ((current == last) &&
          string_at(original, (current - 2), 2, L"AI", L"OI", L"")) {
        primary += L"";
        secondary += L"S";
      } else {
        primary += L"S";
        secondary += L"S";
      }

      if (string_at(original, (current + 1), 1, L"S", L"Z", L""))
        current += 2;
      else
        current += 1;
      break;

    case L'T':
      if (string_at(original, current, 4, L"TION", L"")) {
        primary += L"X";
        secondary += L"X";
        current += 3;
        break;
      }

      if (string_at(original, current, 3, L"TIA", L"TCH", L"")) {
        primary += L"X";
        secondary += L"X";
        current += 3;
        break;
      }

      if (string_at(original, current, 2, L"TH", L"") ||
          string_at(original, current, 3, L"TTH", L"")) {
        /* special case 'thomas', 'thames' or germanic */
        if (string_at(original, (current + 2), 2, L"OM", L"AM", L"") ||
            string_at(original, 0, 4, L"VAN ", L"VON ", L"") ||
            string_at(original, 0, 3, L"SCH", L"")) {
          primary += L"T";
          secondary += L"T";
        } else {
          primary += L"0"; /* yes, zero */
          secondary += L"T";
        }
        current += 2;
        break;
      }

      if (string_at(original, (current + 1), 1, L"T", L"D", L"")) {
        current += 2;
      } else {
        current += 1;
      }
      primary += L"T";
      secondary += L"T";
      break;

    case L'V':
      if (get_at(original, current + 1) == L'V') {
        current += 2;
      } else {
        current += 1;
      }
      primary += L"F";
      secondary += L"F";
      break;

    case L'W':
      /* can also be in middle of word */
      if (string_at(original, current, 2, L"WR", L"")) {
        primary += L"R";
        secondary += L"R";
        current += 2;
        break;
      }

      if ((current == 0) &&
          (is_vowel(original, current + 1) ||
           string_at(original, current, 2, L"WH", L""))) {
        /* Wasserman should match Vasserman */
        if (is_vowel(original, current + 1)) {
          primary += L"A";
          secondary += L"F";
        } else {
          /* need Uomo to match Womo */
          primary += L"A";
          secondary += L"A";
        }
      }

      /* Arnow should match Arnoff */
      if (((current == last) && is_vowel(original, current - 1)) ||
          string_at(original, (current - 1), 5,
                  L"EWSKI", L"EWSKY", L"OWSKI", L"OWSKY", L"") ||
          string_at(original, 0, 3, L"SCH", L"")) {
        primary += L"";
        secondary += L"F";
        current += 1;
        break;
      }

      /* polish e.g. 'filipowicz' */
      if (string_at(original, current, 4, L"WICZ", L"WITZ", L"")) {
        primary += L"TS";
        secondary += L"FX";
        current += 4;
        break;
      }

      /* else skip it */
      current += 1;
      break;

    case L'X':
      /* french e.g. breaux */
      if (!((current == last) &&
            (string_at(original, (current - 3), 3, L"IAU", L"EAU", L"") ||
             string_at(original, (current - 2), 2, L"AU", L"OU", L"")))) {
        primary += L"KS";
        secondary += L"KS";
      }


      if (string_at(original, (current + 1), 1, L"C", L"X", L""))
        current += 2;
      else
        current += 1;
      break;

    case L'Z':
      /* chinese pinyin e.g. 'zhao' */
      if (get_at(original, current + 1) == L'H') {
        primary += L"J";
        secondary += L"J";
        current += 2;
        break;
      } else if (string_at(original, (current + 1), 2, L"ZO", L"ZI", L"ZA", L"") ||
                 (is_slavo_germanic(original) &&
                  ((current > 0) &&
                   get_at(original, current - 1) != L'T'))) {
        primary += L"S";
        secondary += L"TS";
      } else {
        primary += L"S";
        secondary += L"S";
      }

      if (get_at(original, current + 1) == L'Z')
        current += 2;
      else
        current += 1;
      break;

    default:
      current += 1;
    }
    /* printf("PRIMARY: %s\n", primary.str);
       printf("SECONDARY: %s\n", secondary.str);  */
  }


  if (primary.length() > max_length)
    set_at(primary, max_length, '\0');

  if (secondary.length() > max_length)
    set_at(secondary, max_length, '\0');

  codes.push_back(primary);
  codes.push_back(secondary);
  return codes;
}

int dmetaphone_eq(const std::wstring &s1, const std::wstring &s2)
{
  vector<wstring> v1 = dmetaphone(s1);
  vector<wstring> v2 = dmetaphone(s2);
  return equal(v1.begin(), v1.end(), v2.begin());
}
