#ifndef ABCD_H
#define ABCD_H

#include <vector>
#include <set>
#include <utility>
#include <algorithm>
#include <cstdio>

/**
 * Takes the classification results and the targets and generates a report.
 */
template <typename T>
class Abcd
{
    std::vector<std::pair<T, T>> preds_targets;
    std::set<T> uniques;

public:
    void add(T target, T pred)
    {
        preds_targets.push_back(std::make_pair(pred, target));

        uniques.emplace(pred);
        uniques.emplace(target);
    }

    void report(std::string db = "db", std::string rx = "rx")
    {
        std::printf(" %5s | %5s | %5s | %5s | %5s | %5s | %5s | %4s | %4s | %4s | %4s | %4s | %4s | class\n",
                    "db", "rx", "num", "a", "b", "c", "d", "acc", "pre", "pd", "pf", "f", "g");
        std::printf(" %5s | %5s | %5s | %5s | %5s | %5s | %5s | %4s | %4s | %4s | %4s | %4s | %4s | -----\n",
                    "----", "----", "----", "----", "----", "----", "----", "----", "----", "----", "----",
                    "----", "----");

        for (const T &value : uniques)
        {
            int a = 0; // target = pred = -
            int b = 0; // target = +, pred = -
            int c = 0; // target = -, pred = +
            int d = 0; // target = pred = +

            for (const std::pair<T, T> &pair : preds_targets)
            {
                if (pair.first != value && pair.second != value)
                    ++a;
                if (pair.first != value && pair.second == value)
                    ++b;
                if (pair.first == value && pair.second != value)
                    ++c;
                if (pair.first == value && pair.second == value)
                    ++d;
            }

            double pd = (b + d) ? static_cast<double>(d) / (b + d) : 0;
            double pf = (a + c) ? static_cast<double>(c) / (a + c) : 0;
            double pn = (a + c) ? static_cast<double>(b + d) / (a + c) : 0;
            double prec = (c + d) ? static_cast<double>(d) / (c + d) : 0;
            double g = (1 - pf + pd) ? static_cast<double>(2 * pd * (1 - pf)) / (1 - pf + pd) : 0;
            double f = (prec + pd) ? static_cast<double>(2 * prec * pd) / (prec + pd) : 0;
            double acc = (a + b + c + d) ? static_cast<double>(a + d) / (a + b + c + d) : 0;

            std::printf(" %5s | %5s | %5d | %5d | %5d | %5d | %5d | %4.2f | %4.2f | %4.2f | %4.2f | %4.2f | %4.2f | %s\n",
                        db.c_str(), rx.c_str(), a + b + c + d, a, b, c, d, acc, prec, pd, pf, f, g, value.c_str());
        }
    }
};

#endif