#ifndef ZEROR_CLASSIFIER_H
#define ZEROR_CLASSIFIER_H

#include "Tbl.h"
#include "Row.h"

class ZeroRClassifier
{
    const Tbl& tbl;

public:
    ZeroRClassifier(const Tbl& tbl) : tbl(tbl) { }

    std::string classify()
    {
        auto col = tbl.get_classification_column();
        std::string mode = col.get_mode();

        return mode;
    }
};

#endif