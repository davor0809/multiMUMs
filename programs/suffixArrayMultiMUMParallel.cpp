#include <iostream>
#include <string>
#include <sdsl/suffix_arrays.hpp>
#include <sdsl/lcp.hpp>
#include <thread>
#include <vector>
#include "suffixArrayMultiMUMParallel.hpp"

using namespace std;
using namespace sdsl;
using namespace saParallel;

void saParallel::calculateMUMs(string s1, string s2, vector<int>* newMin, vector<int>* newMax) {
    csa_bitcompressed<> csa;
    lcp_bitcompressed<> lcp;
    vector<bool> domain;

    int n1, n2;
    string s;
    s = s1 + '#' + s2;
    n1 = s1.length();
    n2 = s2.length();

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
                (*newMin)[index] = minLength;
                (*newMax)[index] = maxLength;
            }
        }
    }

    util::clear(csa);
    util::clear(lcp);
}

int saParallel::calculateMultiMUMs(vector<string> strings) {
    int multiMUMs = 0;
    int m = strings.size();

    if (m < 2) {
        // only one string, there are no multiMUMs
        return multiMUMs;
    }

    int n1 = strings[0].size();
    vector<vector<int>> arraysMin(m);
    vector<vector<int>> arraysMax(m);
    fill(arraysMin.begin(), arraysMin.end(), vector<int>(n1, 0));
    fill(arraysMax.begin(), arraysMax.end(), vector<int>(n1, 0));

    vector<int> uniqueMin;
    vector<int> uniqueMax;
    thread* threads = new thread[m];
    for (int i = 1; i < m; i++) {
        threads[i] = thread(calculateMUMs, strings[0], strings[i], &(arraysMin[i]), &(arraysMax[i]));
    }
    for (int i = 1; i < m; i++) {
        threads[i].join();
    }
    delete[] threads;

    for (int i = 2; i < m; i++) {
        for (int j = 0; j < n1; j++) {
            if (arraysMax[i-1][j] > 0 && arraysMax[i][j] > 0) {
                // unique match found in both
                // merge them together
                int maximum = min(arraysMax[i-1][j], arraysMax[i][j]);
                int minimum = max(arraysMin[i-1][j], arraysMin[i][j]);
                if (minimum <= maximum) {
                    arraysMax[i][j] = maximum;
                    arraysMin[i][j] = minimum;
                } else {
                    arraysMax[i][j] = 0;
                    arraysMin[i][j] = 0;
                }
            } else {
                arraysMax[i][j] = 0;
                arraysMin[i][j] = 0;
            }
        }
    }

    int matchedUntil = -1;
    uniqueMin = arraysMin[m - 1];
    uniqueMax = arraysMax[m - 1];
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