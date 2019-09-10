#include "Tbl.h"

int main() {
    /*Tbl table;
    table.read("input-2.txt");
    table.dump(); */

    std::string str = "aaaabbc";
    Sym sym;

    for (const char& c : str)
        sym += c;
    
    std::cout << sym.SymEnt() << std::endl;

    return 0;
}
