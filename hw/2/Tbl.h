// Compile with -I $BOOST_ROOT
#ifndef HW_TBL_H
#define HW_TBL_H

#include <cmath>
#include <vector>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>
#include <boost/tokenizer.hpp>
#include <experimental/iterator>
#include <string>

class Col {
    static unsigned int count;

    unsigned int col = 0;
    std::string text;

public:
    Col() {
        col = ++count;
    }

    Col(std::string t) : text(t) {
        col = ++count;
    }
};

unsigned int Col::count = 0;

class Row {
    std::vector<double> cells;
    std::vector<double> cooked;
    int dom = 0;

public:
    Row(std::vector<double> v) : cells(v) {}
};

class Tbl {
    std::vector<Row> rows;
    std::vector<Col> cols;
    std::vector<std::vector<double>> table;
public:
    /**
     * Reads a CSV file and imports the structure to internal representations.
     * Handles exceptions by printing out the error and returning.
     * @param filename - The path to the file
     */
    void read(std::string filename) {
        // Open a file
        std::ifstream fin(filename);

        std::vector<std::string> headers;
        std::string line;

        // Ensure it was successful
        if (!fin.is_open()) {
            std::cout << "Exception: Failed to open file.\n";
            return;
        }

        // Ensure non-empty file
        if (fin.eof()) {
            std::cout << "Exception: Empty file.\n";
            return;
        }

        // Read header line, skipping empty/whitespace lines
        while (!fin.eof()) {
            std::getline(fin, line);
            if (!line.empty() && line.find_first_not_of(' ') != std::string::npos) {
                // We found it!
                boost::tokenizer<> tok(line);

                for (auto it = tok.begin(); it != tok.end(); it++)
                    headers.push_back(*it);

                break;
            }
        }

        // Build the column headers
        for (std::string x : headers)
            cols.push_back(Col(x));

        // Check for EOF
        // In an ideal-world, we wouldn't have to idiot-proof software to this extent, good grief.
        if (fin.eof()) {
            std::cout << "Warning: CSV only contains header line.\n";
            return;
        }

        // Read other lines
        while (std::getline(fin, line)) {
            // Check for blank lines
            if (line.empty() || line.find_first_not_of(' ') == std::string::npos)
                continue;

            std::vector<double> values;
            boost::tokenizer<> tok(line);

            // Read all tokens in the line, but discard commas and spacing
            for (auto it = tok.begin(); it != tok.end(); it++) {
                if (*it == "?") continue;
                double val = std::stod(*it);
                values.push_back(val);
            }

            if (values.size() != headers.size()) {
                std::cout << "Exception: Rows with missing values.\n";
                return;
            }

            rows.push_back(Row(values));
            table.push_back(values);
        }
        
        for (std::string x : headers)
            cols.push_back(Col(x));
    }

    void dump() {
        // Part 1: print list of lists representation
        for (std::vector<double> row : table) {
            // Uses the C++17 experimental ostream_joiner
            std::copy(row.begin(), 
                      row.end(),
                      std::experimental::make_ostream_joiner(std::cout, ", "));
            std::cout << std::endl;
        }
    }
};

class Num : public Col {
    double mean;
    double M2;  // M_{2, n} from Wikipedia
    int n;  // Number of elements
    double sd;

public:
    Num() {
        mean = 0;
        n = 0;
        M2 = 0;
    }

    // Returns sample sd
    double get_var() {
        if (n < 2) return 0;

        sd = std::sqrt(M2 / (n - 1));
        return M2 / (n - 1);
    }

    // Returns mean
    double get_mean() {
        return mean;
    }

    /**
     * Updates the mean and M2 values using
     * Welford's online algorithm.
     * https://en.wikipedia.org/wiki/Algorithms_for_calculating_variance#Welford's_online_algorithm
     *
     * @param val - The new value
     */
    void operator+=(double val) {
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
    void operator-=(double val) {
        if (n < 2) {
            n = 0;
            mean = 0;
            M2 = 0;
        } else {
            n--;
            double delta = val - mean;
            mean -= delta / n;
            M2 -= delta * (val - mean);
        }
    }
};

#endif //HW_TBL_H
