#include "Abcd.h"
#include <string>

int main() {
    Abcd<std::string> abcd;
    for (int j = 1; j <= 6; j++) abcd.add("yes", "yes");
    for (int j = 1; j <= 2; j++) abcd.add("no", "no");
    for (int j = 1; j <= 5; j++) abcd.add("maybe", "maybe");
    abcd.add("maybe", "no");
    abcd.report();

    return 0;
}
