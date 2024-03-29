// Compile with -I $BOOST_ROOT
#ifndef HW_TBL_H
#define HW_TBL_H

#include "Row.h"
#include "Col.h"
#include "Num.h"
#include "Sym.h"
#include <algorithm>
#include <fstream>
#include <numeric>
#include <sstream>
#include <memory>
#include <boost/tokenizer.hpp>
#include <experimental/iterator>

class Tbl
{
    std::vector<Row> rows;
    // from https://stackoverflow.com/a/8777747
    std::vector<std::shared_ptr<Col>> cols;

    std::vector<std::vector<std::string>> table;
    std::vector<std::string> headers;
    std::vector<int> skip_indices;

    std::vector<int> goals, xs, nums, syms, w;

    // Helper functions
    void remove_comments(std::string &line)
    {
        // Ignore comments
        size_t comment_pos;
        if ((comment_pos = line.find("#")) != std::string::npos)
            line = line.substr(0, comment_pos);
    }

    bool tokenize_header(const std::string &line)
    {
        if (!line.empty() && line.find_first_not_of(' ') != std::string::npos)
        {
            // We found it!
            boost::char_separator<char> sep(", ");
            boost::tokenizer<boost::char_separator<char>> tok(line, sep);

            for (auto it = tok.begin(); it != tok.end(); it++)
                headers.push_back(*it);

            return true;
        }

        return false;
    }

    std::vector<std::string> tokenize_line(const std::string &line)
    {
        std::vector<std::string> values;
        boost::char_separator<char> sep(", ");
        boost::tokenizer<boost::char_separator<char>> tok(line, sep);

        // Read all tokens in the line, but discard commas and spacing
        for (auto it = tok.begin(); it != tok.end(); it++)
        {
            std::string val;

            // If ?, we replace it with 0 for now
            if (*it == "?")
                val = "";
            else
                val = *it;
            values.push_back(val);
        }

        return values;
    }

public:
    Sym<> &get_classification_column() const
    {
        if (goals.size() == 0)
            throw "Error: No goals\n";

        // Assume the last goal is our target
        size_t col_index = goals[goals.size() - 1];
        Sym<> &ptr = dynamic_cast<Sym<> &>(*cols[col_index].get());
        return ptr;
    }

    int size() const
    {
        return rows.size();
    }

    std::vector<int> get_skip_columns() const
    {
        return skip_indices;
    }

    std::vector<int> get_nums() const
    {
        return nums;
    }

    std::vector<int> get_syms() const
    {
        return syms;
    }

    double get_column_likelihood(int idx, double val) const
    {
        Num& ptr = dynamic_cast<Num&>(*cols[idx].get());
        double mu = ptr.get_mean();
        double sd = std::sqrt(ptr.get_var());

        return std::exp(-std::pow(val - mu, 2) / (2 * sd * sd)) / (sd * std::sqrt(2 * 3.14159));
    }

    double get_column_likelihood(int idx, std::string val) const
    {
        Sym<>& ptr = dynamic_cast<Sym<>&>(*cols[idx].get());  
        return ptr.get_likelihood(val);
    }

    void print_num_stats() const
    {
        std::cout << "[";
        for (int idx : nums)
        {
            Num& ptr = dynamic_cast<Num&>(*cols[idx].get());
            std::cout << ptr.get_mean() << ", ";
        }
        std::cout << "]\n[";
        for (int idx : nums)
        {
            Num& ptr = dynamic_cast<Num&>(*cols[idx].get());
            std::cout << ptr.get_var() << ", ";
        }
        std::cout << "]";
    }

    /**
     * Populates the headers, skip_indices, cols, nums, syms, goals, xs, and 
     * w member variables.
     */
    void add_header(std::string line)
    {
        // Check if header already exists
        if (headers.size() != 0) return;

        remove_comments(line);
        tokenize_header(line);

        // Deal with ? columns
        // Uses the same idea as https://stackoverflow.com/a/12990554
        std::vector<int> q_pos(headers.size());
        std::iota(q_pos.begin(), q_pos.end(), 0);
        auto it = std::copy_if(q_pos.begin(), q_pos.end(), q_pos.begin(),
                               [=](int index) {
                                   return headers[index].find('?') != std::string::npos;
                               });
        q_pos.resize(std::distance(q_pos.begin(), it));

        // Indices must be in reverse sorted order when removing elements
        std::reverse(q_pos.begin(), q_pos.end());

        // Copy the result to our private member variable
        skip_indices = q_pos;

        // Create columns
        for (int i = 0; i < headers.size(); i++)
        {
            std::string x = headers[i];

            // We don't need to check for the header containing ? here
            // since we've removed those columns and headers already.
            if (x.find("<") != std::string::npos ||
                x.find(">") != std::string::npos ||
                x.find("$") != std::string::npos)
            {
                cols.emplace_back(new Num(x));
                nums.push_back(i);
            }
            else
            {
                cols.emplace_back(new Sym<>(x));
                syms.push_back(i);
            }

            // We may as well update the xs, syms, nums, etc. columns here
            if (x.find("<") != std::string::npos ||
                x.find(">") != std::string::npos ||
                x.find("!") != std::string::npos)
            {
                goals.push_back(i);
            }
            else
                xs.push_back(i);

            if (x.find("<") != std::string::npos)
                w.push_back(i);
        }
    }

    /**
     * Adds a row to the table. This is meant to be used by online
     * classifiers. Make sure the add_header() function is called
     * before this. Do not use this with read().
     */
    void add_row(std::string line)
    {
        remove_comments(line);

        // Check for blank lines
        if (line.empty() || line.find_first_not_of(' ') == std::string::npos)
            return;

        std::vector<std::string> values = tokenize_line(line);

        // Check that the row contained the right number of items
        // Since classifiers using this function won't add a header, we
        // need a different check.
        if (table.size() != 0 && values.size() != table[0].size())
        {
            std::cerr << "Exception: Rows with missing or extra values, skipping.\n";
            return;
        }

        // Remove the columns that we need to skip...
        for (int index : skip_indices)
            values.erase(std::next(values.begin(), index));

        // ...then add the row to the table...
        table.push_back(values);

        // ...and to the rows vector...
        rows.push_back(Row(values));

        // ...and finally to the cols vector
        // Populate values
        for (int i = 0; i < values.size(); i++)
        {
            if (std::find(skip_indices.begin(), skip_indices.end(), i) != skip_indices.end())
                continue;
            
            *(cols[i]) += values[i];
        }
    }

    /**
     * Reads a CSV file and imports the structure to internal representations.
     * Handles exceptions by printing out the error and returning.
     * @param filename - The path to the file
     */
    void read(std::string filename)
    {
        // Open a file
        std::ifstream fin(filename);
        std::string line;

        // Ensure it was successful
        if (!fin.is_open())
        {
            std::cerr << "Exception: Failed to open file.\n";
            return;
        }

        // Ensure non-empty file
        if (fin.eof())
        {
            std::cerr << "Exception: Empty file.\n";
            return;
        }

        // Read header line, skipping empty/whitespace lines
        while (!fin.eof())
        {
            std::getline(fin, line);
            remove_comments(line);

            if (tokenize_header(line))
                break;
        }

        // Check for EOF
        // In an ideal-world, we wouldn't have to idiot-proof software to this extent, good grief.
        if (fin.eof())
        {
            std::cerr << "Exception: CSV only contains header line.\n";
            return;
        }

        // Read other lines
        int line_no = 1;
        while (std::getline(fin, line))
        {
            remove_comments(line);

            ++line_no;
            // Check for blank lines
            if (line.empty() || line.find_first_not_of(' ') == std::string::npos)
                continue;

            std::vector<std::string> values = tokenize_line(line);

            // Check that the row contained the right number of items
            if (values.size() != headers.size())
            {
                std::cerr << "Exception: at line " << line_no << "\n";
                std::cerr << "Message: Rows with missing or extra values, skipping.\n";
                continue;
            }

            table.push_back(values);
        }

        // Deal with ? columns
        // Uses the same idea as https://stackoverflow.com/a/12990554
        std::vector<int> q_pos(headers.size());
        std::iota(q_pos.begin(), q_pos.end(), 0);
        auto it = std::copy_if(q_pos.begin(), q_pos.end(), q_pos.begin(),
                               [=](int index) {
                                   return headers[index].find('?') != std::string::npos;
                               });
        q_pos.resize(std::distance(q_pos.begin(), it));

        // Indices must be in reverse sorted order when removing elements
        std::reverse(q_pos.begin(), q_pos.end());

        // Copy the result to our private member variable
        skip_indices = q_pos;

        // Now use idea from https://stackoverflow.com/a/27265146
        for (int index : q_pos)
        {
            std::for_each(table.begin(), table.end(),
                          [&](std::vector<std::string> &row) {
                              row.erase(std::next(row.begin(), index));
                          });
        }

        /* Finally, remove those headers from the headers list itself
           This is okay because (presumably) we won't allow the user to
           add more columns. So if we add a new row, we just look at
           skip_indices, remove those parts (as well as parts with just a ?),
           and then add that to each Num (and add the row itself to rows).
        */
        for (int index : q_pos)
            headers.erase(std::next(headers.begin(), index));

        // Build rows
        for (std::vector<std::string> row : table)
            rows.push_back(Row(row));

        // Create columns
        for (int i = 0; i < headers.size(); i++)
        {
            std::string x = headers[i];

            // We don't need to check for the header containing ? here
            // since we've removed those columns and headers already.
            if (x.find("<") != std::string::npos ||
                x.find(">") != std::string::npos ||
                x.find("$") != std::string::npos)
            {
                cols.emplace_back(new Num(x));
                nums.push_back(i);
            }
            else
            {
                cols.emplace_back(new Sym<>(x));
                syms.push_back(i);
            }

            // We may as well update the xs, syms, nums, etc. columns here
            if (x.find("<") != std::string::npos ||
                x.find(">") != std::string::npos ||
                x.find("!") != std::string::npos)
            {
                goals.push_back(i);
            }
            else
                xs.push_back(i);

            if (x.find("<") != std::string::npos)
                w.push_back(i);
        }

        // Populate values
        for (int i = 0; i < table.size(); i++)
        {
            for (int j = 0; j < table[0].size(); j++)
                *(cols[j]) += table[i][j];
        }
    }

    void dump()
    {
        std::cout << "t.cols\n";

        for (int i = 0; i < cols.size(); i++)
        {
            std::cout << "|  " << i + 1 << "\n";
            cols[i]->print();
        }

        std::cout << "t.rows\n";
        for (int i = 0; i < rows.size(); i++)
        {
            std::cout << "|  " << i + 1 << "\n";
            rows[i].print();
        }

        std::cout << "t.my\n";
        std::cout << "|  goals\n";
        for (int idx : goals)
            std::cout << "|  |  " << idx + 1 << "\n";

        std::cout << "|  nums\n";
        for (int idx : nums)
            std::cout << "|  |  " << idx + 1 << "\n";

        std::cout << "|  syms\n";
        for (int idx : syms)
            std::cout << "|  |  " << idx + 1 << "\n";

        std::cout << "|  w\n";
        for (int idx : w)
            std::cout << "|  |  " << idx + 1 << ": -1\n";

        std::cout << "|  xs\n";
        for (int idx : xs)
            std::cout << "|  |  " << idx + 1 << "\n";
    }
};

#endif //HW_TBL_H
