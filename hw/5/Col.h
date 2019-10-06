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
    int n;

public:
    virtual void operator+=(std::string val) {}
    virtual void operator-=(std::string val) {}
    virtual void print() {}
    virtual double variety() = 0;

    Col()
    {
        col = ++count;
    }

    Col(std::string t) : text(t)
    {
        col = ++count;
    }

    int size() 
    {
        return n;
    }
};
unsigned int Col::count = 0;

#endif