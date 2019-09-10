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

    std::vector<std::vector<double>> table;
    std::vector<std::string> headers;
    std::vector<int> skip_indices;

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
            if (!line.empty() && line.find_first_not_of(' ') != std::string::npos)
            {
                // We found it!
                boost::char_separator<char> sep(", ");
                boost::tokenizer<boost::char_separator<char>> tok(line, sep);

                for (auto it = tok.begin(); it != tok.end(); it++)
                    headers.push_back(*it);

                break;
            }
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
            ++line_no;
            // Check for blank lines
            if (line.empty() || line.find_first_not_of(' ') == std::string::npos)
                continue;

            std::vector<double> values;
            boost::char_separator<char> sep(", ");
            boost::tokenizer<boost::char_separator<char>> tok(line, sep);

            // Read all tokens in the line, but discard commas and spacing
            for (auto it = tok.begin(); it != tok.end(); it++)
            {
                double val;

                // If ?, we replace it with 0 for now
                if (*it == "?")
                    val = 0;
                else
                    val = std::stod(*it);
                values.push_back(val);
            }

            // Check that the row contained the right number of items
            if (values.size() != headers.size())
            {
                std::cout << "Exception: at line " << line_no << "\n";
                std::cout << "Message: Rows with missing or extra values, skipping.\n";
                continue;
            }

            rows.push_back(Row(values));
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
                          [&](std::vector<double> &row) {
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

        // Create columns
        for (std::string x : headers)
        {
            // We don't need to check for the header containing ? here
            // since we've removed those columns and headers already.
            cols.emplace_back(new Num(x));
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
    }
};

#endif //HW_TBL_H
