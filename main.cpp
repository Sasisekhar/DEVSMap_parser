#include <iostream>
#include "CadmiumAtomicParser.hpp"
#include "CadmiumCoupledParser.hpp"
#include "DEVSMap_Parser.hpp"

int main(int argc, char** argv) {

    if(argc < 2) {
        std::cerr << "Error: Too few arguments. Typical usage:\n" << argv[0] << " <Path to JSON file>" << std::endl;
        return 0;
    }

    Parser<CadmiumAtomicParser, CadmiumCoupledParser> parser(argv[1]);

    return 0;
}