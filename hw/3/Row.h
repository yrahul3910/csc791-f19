#ifndef ROW_H
#define ROW_H

#include <vector>
#include <iostream>

class Row
{
    std::vector<std::string> cells;
    std::vector<std::string> cooked;
    int dom = 0;

public:
    Row(std::vector<std::string> v) : cells(v) {}
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

#endif