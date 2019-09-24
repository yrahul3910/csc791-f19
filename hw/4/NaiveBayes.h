#ifndef NAIVE_BAYES_H
#define NAIVE_BAYES_H

#include "Tbl.h"
#include <map>
#include <string>
#include <numeric>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

class NaiveBayes
{
    std::map<std::string, Tbl> class_tables;
    Tbl master_table;

public:
    void add_row(std::string line)
    {
        std::vector<std::string> tokens;
        boost::split(tokens, line, boost::is_any_of(", "));

        if (tokens.size() == 0)
            return;

        std::string class_name = tokens[tokens.size() - 1];

        // If there's no table for the current class, create one.
        if (class_tables.find(class_name) == class_tables.end())
            class_tables[class_name] = Tbl();

        class_tables[class_name].add_row(line);
        master_table.add_row(line);
    }

    std::string classify(std::string line)
    {
        std::vector<std::string> tokens;
        boost::split(tokens, line, boost::is_any_of(", "));

        if (tokens.size() == 0)
            return "null";

        // TODO: Check the right number of columns

        std::string most_likely_class = "";
        double highest_posterior = -1;
        for (std::pair<std::string, Tbl> pair : class_tables)
        {
            // Compute the prior
            int total_rows = std::accumulate(class_tables.begin(),
                                             class_tables.end(),
                                             0, [](int prev, std::pair<std::string, Tbl> pair2) {
                                                 return prev + pair2.second.size();
                                             });
            double prior = static_cast<double>(pair.second.size()) / total_rows;

            // Compute the likelihood
            double likelihood = 1.0;
            /*
                for (column in columns)
                    if (column is Sym)
                        likelihood *= count(value matches) / pair.second.size()
                    else
                        likelihood *= 1 / (sqrt(2*pi) * sigma) * exp(-(val - mu)^2 / 2 * sigma^2)
            */

           double posterior = prior * likelihood;
           if (posterior > highest_posterior)
           {
               highest_posterior = posterior;
               most_likely_class = pair.first;
           }
        }
    }
};

#endif