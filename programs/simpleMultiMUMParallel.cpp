#include <sdsl/suffix_trees.hpp>
#include <string>
#include <deque>
#include <thread>
#include <vector>
#include "simpleMultiMUMParallel.hpp"

using namespace sdsl;
using namespace std;
using namespace simpleParallel;

void simpleParallel::calculateMUMs(string s1, string s2, vector<int>* newMin, vector<int>* newMax) {
    cst_sct3<csa_bitcompressed<>, lcp_bitcompressed<>> cst;
    string s;
    s = s1 + "#" + s2;
    int n, n1, n2;
    n1 = s1.length();
    n2 = s2.length();
    n = s.length();

    construct_im(cst, s, 1);

    // implement breadth-first-search
    deque<sdsl::bp_interval<sdsl::int_vector_size_type>> q;
    // now calculate MUMs
    q.push_back(cst.root());
    while (q.size() > 0) {
        auto current = q.front();
        q.pop_front();
        auto children = cst.children(current);
        // check 2 children
        if (children.size() == 2) {
            // both children must be leafs
            auto child1 = children[0];
            auto child2 = children[1];
            if (cst.is_leaf(child1) && cst.is_leaf(child2)) {
                // the start-indexes of the suffixes corresponding to the leafs
                auto index1 = cst.sn(child1);
                auto index2 = cst.sn(child2);
                if ((index1 <= n1 && index2 > n1) || index1 > n1 && index2 <= n1) {
                    // we have found a MUM
                    // the start-index of the unique match in s1
                    int index;
                    if (index1 < index2) {
                        index = index1;
                    } else {
                        index = index2;
                    }
                    int childDepth = cst.depth(current);
                    auto parent = cst.parent(current);
                    int parentDepth = cst.depth(parent);
                    int minLength = parentDepth + 1;
                    int maxLength = childDepth;
                    (*newMin)[index] = minLength;
                    (*newMax)[index] = maxLength;
                }
            }
        }
        // add all children
        for (auto child = cst.children(current).begin(); child != cst.children(current).end(); child++) {
            q.push_back(*child);
        }
    }

    // clear suffix-tree
    util::clear(cst);
}

int simpleParallel::calculateMultiMUMs(vector<string> strings) {
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
    // now go over min and max unique positions and save MUMs (calculate number of multiMUMs)
    for (int i = 0; i < strings[0].size(); i++) {
        // check that there is a unique matching string starting at this position
        if (uniqueMax[i] > 0 && matchedUntil < i + uniqueMax[i] - 1) {
            // found new multiMUM
            multiMUMs++;
            matchedUntil = i + uniqueMax[i] - 1;
        }
    }
    return multiMUMs;
}