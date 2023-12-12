#include <string>
#include <vector>
#include <sdsl/suffix_trees.hpp>
using namespace std;
using namespace sdsl;

namespace stream {
    pair<vector<int>, vector<int>> calculateLeftsideMUMs(cst_sct3<csa_bitcompressed<>, lcp_bitcompressed<>>*, int, string, vector<int>, vector<int>, bool);

    int calculateMultiMUMs(vector<string>);
}