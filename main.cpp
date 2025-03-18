#include "CadmiumAtomicParser.hpp"

int main() {
    
    auto parser = CadmiumAtomicParser("counter.json");
    std::cout << parser.make_state() << std::endl;
    std::cout << std::endl;
    std::cout << parser.make_internal_transition() << std::endl;

    return 0;
}