#include <string>
#include <vector>
#include <sdsl/suffix_trees.hpp>
using namespace std;
using namespace sdsl;

namespace streamParallel {
    void calculateLeftsideMUMs(cst_sct3<csa_bitcompressed<>, lcp_bitcompressed<>>*, int, string, vector<int>*, vector<int>*);

    int calculateMultiMUMs(vector<string>);
}