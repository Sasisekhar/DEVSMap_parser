#include "CadmiumAtomicParser.hpp"

int main() {
    
    auto parser = CadmiumAtomicParser("counter.json", false);
    std::string filename = parser.model_name + ".hpp";
    std::ofstream file(filename.c_str());
    file << parser.make_model();
    file.close();

    return 0;
}