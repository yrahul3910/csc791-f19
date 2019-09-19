#include "Tbl.h"
#include "ZeroRClassifier.h"
#include <string>

int main() {
    Tbl tbl;
    tbl.read("diabetes.csv");

    ZeroRClassifier classifier(tbl);
    std::cout << "Class of any new sample is: " << classifier.classify() << std::endl;

    return 0;
}
