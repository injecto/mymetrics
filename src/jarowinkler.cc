/* 
 * Copyright (C) 2011 Miguel Serrano
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * This code edited by Vladimir Glushenkov vio@xakep.ru to UTF-8 strings support
 */

#include "jarowinkler.h"

#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((a) > (b) ? (a) : (b))

double jaro_winkler_dist(const wchar_t *s1, const wchar_t *s2, double scaling_factor) {
    int i, j, l;
    int m = 0, t = 0;
    int s1l = wcslen(s1);
    int s2l = wcslen(s2);
    int s1flags[s1l], s2flags[s2l];
    int range = MAX(0, MAX(s1l, s2l) / 2 - 1);
    double dw;

    if (!s1l || !s2l)
        return 0.0;

    for (i = 0; i < s2l; i++)
        s2flags[i] = 0;

    for (i = 0; i < s1l; i++)
        s1flags[i] = 0;

    /* calculate matching characters */
    for (i = 0; i < s2l; i++) {
        for (j = MAX(i - range, 0), l = MIN(i + range + 1, s1l); j < l; j++) {
            if (s2[i] == s1[j] && !s1flags[j]) {
                s1flags[j] = 1;
                s2flags[i] = 1;
                m++;
                break;
            }
        }
    }

    if (!m)
        return 0.0;

    /* calculate character transpositions */
    l = 0;
    for (i = 0; i < s2l; i++) {
        if (s2flags[i] == 1) {
            for (j = l; j < s1l; j++) {
                if (s1flags[j] == 1) {
                    l = j + 1;
                    break;
                }
            }
            if (s2[i] != s1[j])
                t++;
        }
    }
    t /= 2;

    /* Jaro distance */
    dw = (((double)m / s1l) + ((double)m / s2l) + ((double)(m - t) / m)) / 3.0;

    /* calculate common string prefix up to 4 chars */
    l = 0;
    for (i = 0; i < MIN(MIN(s1l, s2l), 4); i++)
        if (s1[i] == s2[i])
            l++;

    /* Jaro-Winkler distance */
    dw = dw + (l * scaling_factor * (1 - dw));

    return dw;
}

double jaro_winkler_dist(const wchar_t *s1, const wchar_t *s2) {
    return jaro_winkler_dist(s1, s2, 0.1);
}
