#include "Tbl.h"
#include "ZeroRClassifier.h"
#include "NaiveBayes.h"
#include "Abcd.h"
#include <fstream>
#include <string>

int main() {
    NaiveBayes nb;

    std::ifstream fin("weathernon.csv");
    std::string line;

    std::getline(fin, line);
    nb.add_header(line);

    Abcd<std::string> report;

    // Read 4 rows
    for (int i = 0; i < 5 && !fin.eof(); i++)
    {
        std::getline(fin, line);
        nb.add_row(line);
    }

    while (!fin.eof())
    {
        std::getline(fin, line);
        std::string pred = nb.classify(line);
        nb.add_row(line);

        std::string target = line.substr(line.find_last_of(',') + 1);
        report.add(target, pred);
    }

    report.report();

    return 0;
}
