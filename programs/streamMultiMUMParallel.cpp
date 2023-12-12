#include <sdsl/suffix_trees.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <deque>
#include <algorithm>
#include <thread>
#include "streamMultiMUMParallel.hpp"

using namespace sdsl;
using namespace std;
using namespace streamParallel;

void streamParallel::calculateLeftsideMUMs(cst_sct3<csa_bitcompressed<>, lcp_bitcompressed<>>* suffix_tree, int n1, string s2, vector<int>* newMin, vector<int>* newMax) {
    int* excludeMax = new int[n1];
    memset(excludeMax, 0, n1 * sizeof(*excludeMax));

    cst_sct3<csa_bitcompressed<>, lcp_bitcompressed<>> cst = *suffix_tree;
    int n2;
    n2 = s2.length();

    // current node
    auto cn = cst.root();
    // now iterate through s2
    // matched until j (0-indexed)
    int j = -1;
    // w is the number of characters that are on edge from cn to nn and that have been matched
    int w = 0;
    for (int i = 0; i < n2; i++) {
        if (j + 1 < i) {
            j = i - 1;
        }
        auto nn = cn;
        int cndepth = cst.depth(cn);
        // match old information against suffix-tree (cut characters from last iteration), must be possible
        while (w > 0) {
            nn = cst.child(cn, s2[i + cndepth]);
            auto edgeLength = cst.depth(nn) - cst.depth(cn);
            if (edgeLength <= w) {
                w -= edgeLength;
                cn = nn;
                cndepth = cst.depth(cn);
            } else {
                break;
            }
        }
        // now match s2 as long against suffix-tree until mismatch
        // already matched until oldj, increase j to point to next unmatched character
        int oldj = j;
        while (j + 1 < n2) {
            // w characters from edge from cn to nn have already been matched
            // is not at node, but on edge (1 <= w <= edgeLength(cn, nn))
            if (w > 0) {
                // we are currently on an edge from before-matching
                // nn is already set
            } else {
                // we are currently on a node
                // next node
                nn = cst.child(cn, s2[j + 1]);
                // check that next node exists
                if (cst.depth(nn) == 0) {
                    // character has not been matched, still at node
                    w = 0;
                    nn = cn;
                    break;
                }
            }
            
            // get number of characters on edge
            auto edgeLength = cst.depth(nn) - cst.depth(cn);
            // check that all characters on edge match
            int k = 1;
            while (true) {
                if (w + k > edgeLength) {
                    // whole edge matched
                    k--;
                    break;
                }
                if (j + 1 + k - 1 >= n2 || cst.edge(nn, cst.depth(cn) + w + k) != s2[j + 1 + k - 1]) {
                    // we have either end of s2 reached or
                    // we have mismatch and k - 1 new matches
                    k--;
                    break;
                }
                k++;
            }
            // check if everything has matched and set current node to next node
            if (w + k == edgeLength) {
                j += k;
                cn = nn;
                w = 0;
            } else {
                // only k characters have been matched and j points to old matching-point
                j += k;
                w += k;
                break;
            }
        }
        auto nodeToCheck = cn;
        if (w > 0) {
            nodeToCheck = nn;
        }
        if (cst.is_leaf(nodeToCheck)) {
            // there are two possibilities: we are on edge to leaf or on leaf (if w > 0)
            // if we are on edge to leaf, the minLength is depth of current node + 1 and maxLength is j - i + 1
            // if we are on leaf, the minLength is depth of parent + 1 and maxLength is j - i + 1
            // we have found a unique match of certain maximumLength
            int maxLength = j - i + 1;
            int minLength = cst.depth(cst.parent(nodeToCheck)) + 1;
            // starting position in s1 is not known yet
            int index = cst.sn(nodeToCheck);
            int excludeFrom, excludeTo;
            excludeTo = 0;
            // update minLength and maxLength to not contain excluded parts
            if (excludeMax[index] > 0) {
                if (excludeMax[index] >= maxLength) {
                    minLength = 0;
                    maxLength = 0;
                } else if (excludeMax[index] < maxLength && excludeMax[index] >= minLength) {
                    minLength = excludeMax[index] + 1;
                } else {
                    // do not change minLength and maxLength
                }
            }
            if (minLength > 0) {
                if ((*newMin)[index] > 0) {
                    // already changed, only update
                    excludeMax[index] = min((*newMax)[index], maxLength);
                    if ((*newMax)[index] == maxLength) {
                        // not unique until maxLength
                        (*newMin)[index] = 0;
                        (*newMax)[index] = 0;
                    } else {
                        // new minimum is set
                        (*newMin)[index] = excludeMax[index] + 1;
                        // new information comes
                        if (maxLength > (*newMax)[index]) {
                            (*newMax)[index] = maxLength;
                        }
                    }
                } else {
                    (*newMin)[index] = minLength;
                    (*newMax)[index] = maxLength;
                }
            }
        }
        // go to suffix-link
        if (w > 0 && cst.depth(cn) == 0) {
            w--;
        }
        cn = cst.sl(cn);
    }
    delete[] excludeMax;
}

int streamParallel::calculateMultiMUMs(vector<string> strings) {
    int multiMUMs = 0;
    int m = strings.size();

    if (m < 2) {
        // only one string, there are no multiMUMs
        return multiMUMs;
    }

    cst_sct3<csa_bitcompressed<>, lcp_bitcompressed<>> cst;
    construct_im(cst, strings[0], 1);
    int n0;
    n0 = strings[0].size();

    vector<vector<int>> arraysMin(m);
    vector<vector<int>> arraysMax(m);
    fill(arraysMin.begin(), arraysMin.end(), vector<int>(n0, 0));
    fill(arraysMax.begin(), arraysMax.end(), vector<int>(n0, 0));

    vector<int> uniqueMin;
    vector<int> uniqueMax;
    thread* threads = new thread[m];
    for (int i = 1; i < m; i++) {
        threads[i] = thread(calculateLeftsideMUMs, &cst, n0, strings[i], &(arraysMin[i]), &(arraysMax[i]));
    }
    for (int i = 1; i < m; i++) {
        threads[i].join();
    }
    delete[] threads;
    util::clear(cst);

    for (int i = 2; i < m; i++) {
        for (int j = 0; j < n0; j++) {
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