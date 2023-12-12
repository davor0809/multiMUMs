#include <string>
#include <iostream>
#include <vector>
#include <fstream>
#include <algorithm>
#include "simpleMultiMUM.hpp"
#include "streamMultiMUM.hpp"
#include "suffixArrayMultiMUM.hpp"
#include "simpleMultiMUMParallel.hpp"
#include "streamMultiMUMParallel.hpp"
#include "suffixArrayMultiMUMParallel.hpp"

using namespace std;
using namespace std::chrono;
using timer = std::chrono::high_resolution_clock;

int main(int argc, char* argv[]) {
    string s;
    vector<string> strings;
    int multiMUMs;
    // the number of strings
    int m;

    ifstream input("input");

    input >> m;
    for (int i = 0; i < m; i++) {
        input >> s;
        strings.push_back(s);
    }

    auto start = timer::now();
    auto stop = start;

    start = timer::now();

    // comment out the algorithm that you want to use

    multiMUMs = simple::calculateMultiMUMs(strings);
    // multiMUMs = stream::calculateMultiMUMs(strings);
    // multiMUMs = sa::calculateMultiMUMs(strings);
    // multiMUMs = simpleParallel::calculateMultiMUMs(strings);
    // multiMUMs = streamParallel::calculateMultiMUMs(strings);
    // multiMUMs = saParallel::calculateMultiMUMs(strings);

    stop = timer::now();
    cout << "Duration: " << duration_cast<milliseconds>(stop - start).count() << endl;

    cout << "Number of multiMUMs: " << multiMUMs << endl;
}