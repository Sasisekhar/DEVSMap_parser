#include "CadmiumAtomicParser.hpp"

int main() {
    
    auto parser = CadmiumAtomicParser("counter.json", false);
    std::cout << parser.make_state() << std::endl;
    std::cout << std::endl;
    std::cout << parser.make_internal_transition() << std::endl;
    std::cout << std::endl;
    std::cout << parser.make_external_transition() << std::endl;
    std::cout << std::endl;
    std::cout << parser.make_confluent_transition() << std::endl;

    return 0;
}