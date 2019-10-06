#ifndef SYM_H
#define SYM_H

#include "Col.h"
#include <map>
#include <cmath>
#include <algorithm>
#include <numeric>
#include <iostream>
#include <vector>

template <typename T = std::string>
class Sym : public Col
{
    T mode;
    int most;
    std::map<T, size_t> counts; // counts of each symbol
    using pair_type = typename decltype(counts)::value_type;

public:
    Sym()
    {
        n = 0;
    }

    Sym(std::string t)
    {
        n = 0;
        text = t;
    }

    T get_mode() const
    {
        return mode;
    }

    // Adds a char. Uses code from https://stackoverflow.com/a/9371137
    void operator+=(std::string val) override
    {
        ++n;
        ++counts[val];

        pair_type pair = *std::max_element(counts.begin(),
                                           counts.end(),
                                           [](const pair_type &p1, const pair_type &p2) { return p1.second < p2.second; });
        mode = pair.first;
        most = pair.second;
    }

    void operator-=(std::string val) override
    {
        --n;
        --counts[val];

        pair_type pair = *std::max_element(counts.begin(),
                                           counts.end(),
                                           [](const pair_type &p1, const pair_type &p2) { return p1.second < p2.second; });
        mode = pair.first;
        most = pair.second;
    }

    double get_likelihood(std::string val)
    {
        return static_cast<double>(counts[val] + 1) / (n + counts.size());
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

    double variety() override
    {
        return SymEnt();
    }

    void print() override
    {
        std::cout << "|  |  cnt\n";

        for (const pair_type& pair : counts)
            std::cout << "|  |  |  " << pair.first << ": " << pair.second << "\n";
        
        std::cout << "|  |  col: " << col << "\n";
        std::cout << "|  |  mode: " << mode << "\n";
        std::cout << "|  |  most: " << most << "\n";
        std::cout << "|  |  n: " << n << "\n";
        std::cout << "|  |  txt: " << text << "\n";
    }
};

#endif