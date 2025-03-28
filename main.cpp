#include <iostream>
#include "CadmiumAtomicParser.hpp"
#include "CadmiumCoupledParser.hpp"
#include "DEVSMap_Parser.hpp"

int main(int argc, char** argv) {

    if(argc < 3) {
        std::cerr << "Error: Too few arguments. Typical usage:\n" << argv[0] << " <Path to Experiment JSON file> <Output directory>" << std::endl;
        return 0;
    }

    Parser<CadmiumAtomicParser, CadmiumCoupledParser> parser(argv[1], argv[2]);

    return 0;
}