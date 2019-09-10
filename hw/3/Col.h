#ifndef COL_H
#define COL_H

#include <string>

class Col
{
protected:
    // Number of columns. Used to find column number of new cols
    static unsigned int count;

    // Column number
    unsigned int col = 0;
    std::string text;

public:
    virtual void operator+=(double val) {}
    virtual void operator+=(char val) {}
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

#endif