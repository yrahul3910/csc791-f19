// Compile with -I $BOOST_ROOT
#ifndef HW_TBL_H
#define HW_TBL_H

#include <cmath>
#include <vector>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <numeric>
#include <sstream>
#include <limits>
#include <memory>
#include <boost/tokenizer.hpp>
#include <experimental/iterator>
#include <string>

class Col
{
protected:
    static unsigned int count;

    unsigned int col = 0;
    std::string text;

public:
    virtual void operator+=(double val) {}
    virtual void operator+=(std::string val) {}
    virtual void print() {}

    Col()
    {
        col = ++count;
    }

    Col(std::string t) : text(t)
    {
        col = ++count;
    }
};

unsigned int Col::count = 0;

class Num : public Col
{
    double mean;
    double M2; // M_{2, n} from Wikipedia
    int n;     // Number of elements
    double sd;
    double low, hi;

public:
    Num()
    {
        mean = 0;
        n = 0;
        M2 = 0;
        col = ++count;

        low = std::numeric_limits<double>::min();
        hi = std::numeric_limits<double>::max();
    }

    Num(std::string t)
    {
        text = t;
        col = ++count;

        low = std::numeric_limits<double>::max();
        hi = std::numeric_limits<double>::min();
    }

    // Returns sample sd
    double get_var()
    {
        if (n < 2)
            return 0;

        sd = std::sqrt(M2 / (n - 1));
        return M2 / (n - 1);
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
    void operator+=(double val)
    {
        // Workaround for the case where ? is given (and replaced by a 0).
        if (val == 0)
            return;
        
        if (val > hi) hi = val;
        if (val < low) low = val;

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
    void operator-=(double val)
    {
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

class Row
{
    std::vector<double> cells;
    std::vector<double> cooked;
    int dom = 0;

public:
    Row(std::vector<double> v) : cells(v) {}
    void print()
    {
        std::cout << "|  |  cells\n";
        for (int i = 0; i < cells.size(); i++)
        {
            std::cout << "|  |  |  " << i + 1 << ": " << cells[i] << "\n";
        }

        std::cout << "|  |  cooked\n";
        for (int i = 0; i < cooked.size(); i++)
        {
            std::cout << "|  |  |  " << i + 1 << ": " << cooked[i] << "\n";
        }

        std::cout << "|  |  dom: " << dom << "\n";
    }
};

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
        
        for (std::vector<double> values : table)
            rows.push_back(Row(values));

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
