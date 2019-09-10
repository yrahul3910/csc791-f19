#ifndef SYM_H
#define SYM_H

#include "Col.h"
#include <map>
#include <cmath>
#include <algorithm>
#include <numeric>
#include <iostream>
#include <vector>

template <typename T = char>
class Sym : public Col
{
    int n;
    T mode;
    std::map<T, size_t> counts; // counts of each symbol
    using pair_type = typename decltype(counts)::value_type;

public:
    Sym()
    {
        n = 0;
    }

    // Adds a char. Uses code from https://stackoverflow.com/a/9371137
    void operator+=(char val) override
    {
        ++n;
        ++counts[val];

        pair_type pair = *std::max_element(counts.begin(),
                                           counts.end(),
                                           [](const pair_type &p1, const pair_type &p2) { return p1.second < p2.second; });
        mode = pair.first;
    }

    double SymEnt()
    {
        if (n == 0)
        {
            std::cerr << "Error: No symbols.\n";
            std::exit(-1);
        }

        if (n == 1)
            return 0;

        // Get probability values
        std::vector<double> probabilities;
        std::transform(counts.begin(), counts.end(), std::back_inserter(probabilities),
                       [=](const pair_type &pair) { return static_cast<double>(pair.second) / n; });

        // Compute entropy
        double entropy = std::accumulate(probabilities.begin(),
                                         probabilities.end(),
                                         0.0,
                                         [](double old, double prob) { return old - prob * std::log2(prob); });

        return entropy;
    }
};

#endif