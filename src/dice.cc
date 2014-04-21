/*
 * dice coefficient = bigram overlap * 2 / bigrams in a + bigrams in b
 * (C) 2007 Francis Tyers 
 * Modifications made by Stefan Koshiw 2010
 * and Vladimir Glushenkov 2014
 */

#include "dice.h"
#include <set>
#include <algorithm>

using namespace std;

double dice_coeff(const wstring& s1, const wstring& s2) {
    set<wstring> s1bi, s2bi;
    
    if (s1.length() == 0 || s2.length() == 0)
        return 0;
    for (unsigned int i = 0; i < (s1.length() - 1); i++)
        s1bi.insert(s1.substr(i, 2));
    for (unsigned int i = 0; i < (s2.length() - 1); i++) 
        s2bi.insert(s2.substr(i, 2));

    int intersection = 0;
    for (set<wstring>::iterator it = s2bi.begin(); it != s2bi.end(); ++it)
        intersection += s1bi.count(*it);

    return (double)(intersection * 2) / (double)(s1bi.size() + s2bi.size());
}
