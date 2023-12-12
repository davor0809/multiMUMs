#include <iostream>
#include <string>
#include <algorithm>
#include <sdsl/suffix_arrays.hpp>
#include <sdsl/lcp.hpp>
#include "suffixArrayMultiMUM.hpp"

using namespace std;
using namespace sdsl;
using namespace sa;

/*
csa.size(): length
csa[i]: suffix-start of lexicographically i-th suffix (0-indexed)
*/

pair<vector<int>, vector<int>> sa::calculateMUMs(string s1, string s2, vector<int> uniqueStringsMin, vector<int> uniqueStringsMax, bool firstRun) {
    csa_bitcompressed<> csa;
    lcp_bitcompressed<> lcp;
    vector<bool> domain;

    int n1, n2;
    string s;
    s = s1 + '#' + s2;
    n1 = s1.length();
    n2 = s2.length();

    vector<int> newMin(n1, 0);
    vector<int> newMax(n1, 0);

    construct_im(csa, s, 1);
    construct_im(lcp, s, 1);

    for (int i = 0; i < csa.size(); i++) {
        domain.push_back(csa[i] < n1);
    }

    for (int i = 1; i < csa.size(); i++) {
        if (domain[i-1] != domain[i]) {
            if (lcp[i] > lcp[i - 1] && (i + 1 >= csa.size() || lcp[i] > lcp[i + 1])) {
                int index;
                if (domain[i - 1]) {
                    index = csa[i - 1];
                } else {
                    index = csa[i];
                }
                int minLength, maxLength;
                int commonBefore, commonAfter;
                commonBefore = lcp[i - 1];
                if (i + 1 < lcp.size()) {
                    commonAfter = lcp[i + 1];
                } else {
                    commonAfter = 0;
                }
                minLength = max(commonBefore, commonAfter) + 1;
                maxLength = lcp[i];
                if (firstRun) {
                    newMin[index] = minLength;
                    newMax[index] = maxLength;
                } else {
                    // check that the one found is in bounds
                    int oldMin = uniqueStringsMin[index];
                    int oldMax = uniqueStringsMax[index];
                    if (oldMin > 0 && !(oldMax < minLength) && !(oldMin > maxLength)) {
                        // everything fits in bounds
                        if (oldMin > minLength) {
                            minLength = oldMin;
                        }
                        if (oldMax < maxLength) {
                            maxLength = oldMax;
                        }
                        newMin[index] = minLength;
                        newMax[index] = maxLength;
                    }
                }
            }
        }
    }

    util::clear(csa);
    util::clear(lcp);

    return make_pair(newMin, newMax);
}

int sa::calculateMultiMUMs(vector<string> strings) {
    int multiMUMs = 0;
    int m = strings.size();

    if (m < 2) {
        // only one string, there are no multiMUMs
        return multiMUMs;
    }

    vector<int> uniqueMin;
    vector<int> uniqueMax;
    for (int i = 1; i < m; i++) {
        auto updatedInfo = calculateMUMs(strings[0], strings[i], uniqueMin, uniqueMax, i == 1);
        uniqueMin = updatedInfo.first;
        uniqueMax = updatedInfo.second;
    }

    int matchedUntil = -1;
    // now go over min and max unique positions and save MUMs
    for (int i = 0; i < strings[0].size(); i++) {
        // check that there is a unique matching string starting at this position
        if (uniqueMin[i] > 0 && matchedUntil < i + uniqueMax[i] - 1) {
            // found new multiMUM
            multiMUMs++;
            matchedUntil = i + uniqueMax[i] - 1;
        }
    }
    return multiMUMs;
}