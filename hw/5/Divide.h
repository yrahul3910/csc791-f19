#ifndef DIVIDE_H
#define DIVIDE_H

#include <vector>
#include <iostream>
#include <algorithm>
#include <utility>
#include <cmath>
#include <string>
#include "Num.h"

template <class ColType = Num, class ValType = double>
class Divide
{
    std::vector<std::pair<double, double>> list;
    typedef std::pair<double, ValType> pair_type;
    int step;
    double epsilon;
    ValType start, stop;
    double gain = 0;
    std::vector<ColType> ranges;

public:
    std::vector<ColType> get_ranges() const
    {
        return ranges;
    }

    Divide(std::vector<double> x, std::vector<ValType> y)
    {
        if (x.size() != y.size())
        {
            std::cerr << "Error: Sizes must match.\n";
            return;
        }

        for (int i = 0; i < x.size(); i++)
            list.push_back(std::make_pair(x[i], y[i]));

        std::sort(list.begin(), list.end(), [](pair_type x, pair_type y) { return x.second < y.second; });

        ColType before;
        for (double val : y)
            before += std::to_string(val);

        step = static_cast<int>(std::sqrt(list.size()));
        epsilon = before.variety() * 0.3;
        stop = list[list.size() - 1].second;
        start = list[0].second;

        divide(1, list.size(), before, 1);
    }

    double divide(double low, double high, ColType before, double rank)
    {
        ColType left;
        ColType right;
        for (int i = low; i < high; i++)
            right += std::to_string(list[i].second);

        double best = before.variety();
        int cut = -1;

        for (int j = low; j < high; j++)
        {
            left += std::to_string(list[j].second);
            right -= std::to_string(list[j].second);

            if (left.size() >= step)
            {
                if (right.size() >= step)
                {
                    ValType now = list[j - 1].second;
                    ValType after = list[j].second;

                    if (now == after)
                        continue;

                    if (std::abs(right.get_mean() - left.get_mean()) >= epsilon &&
                        after - start >= epsilon &&
                        stop - now >= epsilon)
                    {
                        int n = left.size() + right.size();
                        double expect = left.size() / n * left.variety() + right.size() / n * right.variety();

                        if (expect * 1.025 < best)
                        {
                            best = expect;
                            cut = j;
                        }
                    }
                }
            }
        }

        if (cut != -1)
        {
            ColType ls, rs;
            for (int i = low; i < cut; i++)
                ls += std::to_string(list[i].second);
            for (int i = cut; i < high; i++)
                rs += std::to_string(list[i].second);
            
            rank = divide(low, cut, ls, rank) + 1;
            rank = divide(cut, high, rs, rank);
        }
        else
        {
            gain += before.size() * before.variety();
            ranges.push_back(before);
        }
        return rank;
    }
};

#endif