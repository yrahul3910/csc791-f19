#ifndef ZEROR_CLASSIFIER_H
#define ZEROR_CLASSIFIER_H

#include "Tbl.h"
#include "Row.h"

class ZeroRClassifier
{
    Tbl tbl;

public:
    ZeroRClassifier() = default;

    void add_header(std::string line)
    {
        tbl.add_header(line);
    }

    void add_row(std::string line)
    {
        tbl.add_row(line);
    }

    std::string classify()
    {
        auto col = tbl.get_classification_column();
        std::string mode = col.get_mode();

        return mode;
    }
};

#endif