#ifndef NAIVE_BAYES_H
#define NAIVE_BAYES_H

#include "Tbl.h"
#include <map>
#include <string>
#include <numeric>
#include <algorithm>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

class NaiveBayes
{
    std::map<std::string, Tbl> class_tables;
    Tbl master_table;
    std::string header_line; // unprocessed header string
    std::vector<int> skip_indices, nums, syms;

public:
    void add_header(std::string line)
    {
        // We'll deal with the individual tables later.
        master_table.add_header(line);
        skip_indices = master_table.get_skip_columns();
        nums = master_table.get_nums();
        syms = master_table.get_syms();
        header_line = line;
    }

    void print_num_stats()
    {
        for (const std::pair<std::string, Tbl>& pair : class_tables)
        {
            std::cout << pair.first << ":\n";
            pair.second.print_num_stats();
            std::cout << "\n";
        }
    }

    void add_row(std::string line)
    {
        std::vector<std::string> tokens;
        boost::split(tokens, line, boost::is_any_of(", "));

        if (tokens.size() == 0)
            return;

        std::string class_name = tokens[tokens.size() - 1];

        // If there's no table for the current class, create one.
        if (class_tables.find(class_name) == class_tables.end())
        {
            class_tables[class_name] = Tbl();
            class_tables[class_name].add_header(header_line);
        }

        class_tables[class_name].add_row(line);
        master_table.add_row(line);
    }

    std::string classify(std::string line)
    {
        std::vector<std::string> tokens;
        boost::split(tokens, line, boost::is_any_of(", "));

        if (tokens.size() == 0)
            return "null";

        std::string most_likely_class = "";
        double highest_log_posterior = -1e4;

        for (const std::pair<std::string, Tbl>& pair : class_tables)
        {
            // Compute the prior
            int total_rows = std::accumulate(class_tables.begin(),
                                             class_tables.end(),
                                             0, [](int prev, std::pair<std::string, Tbl> pair2) {
                                                 return prev + pair2.second.size();
                                             });
            double prior = static_cast<double>(pair.second.size()) / total_rows;

            // Compute the likelihood
            double log_likelihood = 0;
            for (int i = 0; i < tokens.size(); i++)
            {
                if (std::find(skip_indices.begin(), skip_indices.end(), i) != skip_indices.end())
                    continue;

                if (std::find(nums.begin(), nums.end(), i) != nums.end())
                {
                    // It's a num
                    log_likelihood += std::log(pair.second.get_column_likelihood(i, std::stod(tokens[i])));
                }
                else
                {
                    // It's a sym
                    log_likelihood += std::log(pair.second.get_column_likelihood(i, tokens[i]));
                }
            }
            //std::cout << "(log_likelihood=" << log_likelihood << ") ";

            double log_posterior = std::log(prior) + log_likelihood;
            if (log_posterior > highest_log_posterior)
            {
                highest_log_posterior = log_posterior;
                most_likely_class = pair.first;
            }
            //std::cout << log_posterior << " ";
        }
        //std::cout << "\n";
        return most_likely_class;
    }
};

#endif