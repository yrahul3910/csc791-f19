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
    std::vector<std::unique_ptr<Col>> cols;

    std::vector<std::vector<std::string>> table;
    std::vector<std::string> headers;
    std::vector<int> skip_indices;

    std::vector<size_t> goals, xs, nums, syms, w;

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
            std::cout << "Exception: Failed to open file.\n";
            return;
        }

        // Ensure non-empty file
        if (fin.eof())
        {
            std::cout << "Exception: Empty file.\n";
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
            std::cout << "Exception: CSV only contains header line.\n";
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
                std::cout << "Exception: at line " << line_no << "\n";
                std::cout << "Message: Rows with missing or extra values, skipping.\n";
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
                cols.emplace_back(new Sym(x));
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
        for (size_t idx : goals)
            std::cout << "|  |  " << idx + 1 << "\n";

        std::cout << "|  nums\n";
        for (size_t idx : nums)
            std::cout << "|  |  " << idx + 1 << "\n";

        std::cout << "|  syms\n";
        for (size_t idx : syms)
            std::cout << "|  |  " << idx + 1 << "\n";

        std::cout << "|  w\n";
        for (size_t idx : w)
            std::cout << "|  |  " << idx + 1 << ": -1\n";

        std::cout << "|  xs\n";
        for (size_t idx : xs)
            std::cout << "|  |  " << idx + 1 << "\n";
    }
};

#endif //HW_TBL_H
