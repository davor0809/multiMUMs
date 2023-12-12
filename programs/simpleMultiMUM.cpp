#include <sdsl/suffix_trees.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <deque>
#include <algorithm>
#include "simpleMultiMUM.hpp"

using namespace sdsl;
using namespace std;
using namespace simple;

using namespace std::chrono;
using timer = std::chrono::high_resolution_clock;

/*
cst-functions:
cst.is_leaf(v): if v is a leaf
cst.lb(v): index leftmost leaf
cst.rb(v): index rightmost leaf
cst.degree(v): number of children
cst.depth(v): string-depth of node (number of characters in path)
cst.node_depth(v): node-depth of node (distance from root)
cst.edge(v,d): d-th character (1-indexed) of edge that ends in node v
cst.id(v): unique identification number for node
cst.inv_id(i): returns node with corresponding unique id i
cst.sn(v): suffix-array-value (start-index of this suffix) corresponding to leaf-node, if v is no leaf-node: returns suffix-array-value of first leaf-node in subtree under v

cst.select_leaf(i): returns i-th leaf (1-based)
cst.parent(v): returns parent-node
cst.sibling(v): the next right sibling
cst.select_child(v,i): returns i-th child of v
cst.child(v,c): returns child where edge-label (u,v) starts with character c
cst.sl(v): suffix-link of node
cst.wl(v): weiner-link
cst.leftmost_leaf(v)
cst.rightmost_leaf(v)

*/

pair<vector<int>, vector<int>> simple::calculateMUMs(string s1, string s2, vector<int> uniqueStringsMin, vector<int> uniqueStringsMax, bool firstRun) {
    cst_sct3<csa_bitcompressed<>, lcp_bitcompressed<>> cst;

    string s;
    s = s1 + "#" + s2;
    int n, n1, n2;
    n1 = s1.length();
    n2 = s2.length();
    n = s.length();

    vector<int> newMin(n1, 0);
    vector<int> newMax(n1, 0);

    construct_im(cst, s, 1);

    // implement breadth-first-search
    deque<sdsl::bp_interval<sdsl::int_vector_size_type>> q;

    // now calculate MUMs
    q.clear();
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
        // add all children
        for (auto child = cst.children(current).begin(); child != cst.children(current).end(); child++) {
            q.push_back(*child);
        }
    }

    // clear suffix-tree
    util::clear(cst);
    return make_pair(newMin, newMax);
}

int simple::calculateMultiMUMs(vector<string> strings) {
    int multiMUMs = 0;
    int m = strings.size();

    if (m < 2) {
        // only one string, there are no multiMUMs
        return multiMUMs;
    }

    vector<int> uniqueMin(0);
    vector<int> uniqueMax(0);
    for (int i = 1; i < m; i++) {
        auto updatedInfo = calculateMUMs(strings[0], strings[i], uniqueMin, uniqueMax, i == 1);
        uniqueMin = updatedInfo.first;
        uniqueMax = updatedInfo.second;
    }

    int matchedUntil = -1;
    // now go over min and max unique positions and save MUMs (calculate number of multiMUMs)
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