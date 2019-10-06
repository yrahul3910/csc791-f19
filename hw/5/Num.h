#ifndef NUM_H
#define NUM_H

#include "Col.h"
#include <iostream>
#include <cmath>
#include <limits>

class Num : public Col
{
    double mean;
    double M2; // M_{2, n} from Wikipedia
    double sd;
    double low, hi;

public:
    Num()
    {
        mean = 0;
        n = 0;
        M2 = 0;

        low = std::numeric_limits<double>::max();
        hi = std::numeric_limits<double>::min();
    }

    Num(std::string t)
    {
        text = t;

        low = std::numeric_limits<double>::max();
        hi = std::numeric_limits<double>::min();
    }

    double get_low() const
    {
        return low;
    }

    double get_high() const
    {
        return hi;
    }

    // Returns sample var
    double get_var()
    {
        if (n < 2)
            return 0;

        return M2 / (n - 1);
    }

    double variety() override
    {
        return std::sqrt(get_var());
    }

    // Returns mean
    double get_mean()
    {
        return mean;
    }

    /**
     * Updates the mean and M2 values using
     * Welford's online algorithm.
     * https://en.wikipedia.org/wiki/Algorithms_for_calculating_variance#Welford's_online_algorithm
     *
     * @param val - The new value
     */
    void operator+=(std::string s) override
    {
        double val = std::stod(s);

        if (val > hi) hi = val;
        if (val < low) low = val;

        // Workaround for the case where ? is given (and replaced by a 0).
        if (val == -999)
            return;

        n++;

        double delta = val - mean;
        mean += delta / n;
        M2 += delta * (val - mean);
    }

    /**
     * Updates the mean and standard deviance using
     * Welford's online algorithm.
     *
     * @param val - The new value
     */
    void operator-=(std::string s) override
    {
        double val = std::stod(s);

        if (n < 2)
        {
            n = 0;
            mean = 0;
            M2 = 0;
        }
        else
        {
            n--;
            double delta = val - mean;
            mean -= delta / n;
            M2 -= delta * (val - mean);
        }
    }

    void print()
    {
        std::cout << "|  |  ";
        std::cout << "col: " << col << "\n";

        std::cout << "|  |  ";
        std::cout << "hi: " << hi << "\n";

        std::cout << "|  |  ";
        std::cout << "low: " << low << "\n";

        std::cout << "|  |  ";
        std::cout << "m2: " << M2 << "\n";

        std::cout << "|  |  ";
        std::cout << "mu: " << mean << "\n";

        std::cout << "|  |  ";
        std::cout << "sd: " << std::sqrt(get_var()) << "\n";

        std::cout << "|  |  ";
        std::cout << "n: " << n << "\n";

        std::cout << "|  |  ";
        std::cout << "text: " << text << "\n";
    }
};

#endif
