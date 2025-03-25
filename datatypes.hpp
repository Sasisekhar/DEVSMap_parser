#ifndef DATATYPES_CONSTANTS_HPP
#define DATATYPES_CONSTANTS_HPP

#include <ostream>
#include <string>
#include <vector>
#include <memory>

enum class TokenType {
    OPERATOR,
    STATE_VARIABLE,
    INPUT_PORT,
    OUTPUT_PORT,
    PARAMETER,
    CONSTANT,
    UNKNOWN
};
std::ostream& operator<<(std::ostream& out, const TokenType value){
    std::string type;

    if(value == TokenType::OPERATOR) {
        type = "OPERATOR";
    } else if(value == TokenType::STATE_VARIABLE) {
        type = "STATE_VARIABLE";
    }  else if(value == TokenType::INPUT_PORT) {
        type = "INPUT_PORT";
    } else if(value == TokenType::OUTPUT_PORT) {
        type = "OUTPUT_PORT";
    } else if(value == TokenType::PARAMETER) {
        type = "PARAMETER";
    } else if(value == TokenType::CONSTANT) {
        type = "CONSTANT";
    } else if(value == TokenType::UNKNOWN) {
        type = "UNKNOWN";
    }

    return out << type;
}

struct Token {
    std::string value;
    TokenType type;
};
std::ostream& operator<<(std::ostream& out, Token o) {
    out << "{ " << o.value << " " << o.type << " }";
    return out;
}

struct object_t {
    std::string variable;
    std::string datatype;

    object_t(std::string v, std::string dt): variable(v), datatype(dt) {}
};
std::ostream& operator<<(std::ostream& out, object_t o) {
    out << "{" << o.variable << ":" << o.datatype << "}";
    return out;
}

struct state_t{
    std::string state_variable;
    std::string expression;

    state_t(std::string v, std::string dt): state_variable(v), expression(dt) {}
};
std::ostream& operator<<(std::ostream& out, state_t sv) {
    out << "{" << sv.state_variable << " : " << sv.expression << "}";
    return out;
}

struct transition_t{
    std::string condition;
    std::vector<state_t> new_state;
    std::vector<std::shared_ptr<transition_t>> nested;

    transition_t(std::string c = "") : condition(std::move(c)) {}
};
std::ostream& operator<<(std::ostream& out, const transition_t& t) {
    if (!t.condition.empty()) {
        out << "Condition [" << t.condition << "]:\n";
    }

    if (!t.new_state.empty()) {
        out << "{ ";
        for (const auto& state : t.new_state) {
            out << state << " ";
        }
        out << "}\n";
    }

    if (!t.nested.empty()) {
        for (const auto& transition : t.nested) {
            out << "  " << *transition;
        }
    }

    return out;
}

struct ta_t {
    std::string condition;
    std::string expression; // Only non-empty at leaf nodes
    std::vector<std::shared_ptr<ta_t>> nested;

    ta_t(std::string c = "", std::string e = "")
        : condition(std::move(c)), expression(std::move(e)) {}
};
std::ostream& operator<<(std::ostream& out, const ta_t& t) {
    if (!t.condition.empty())
        out << t.condition << ": ";
    
    if (!t.nested.empty()) {
        out << "{\n";
        for (const auto& nested : t.nested)
            out << "  " << *nested << "\n";
        out << "}";
    } else {
        out << t.expression;
    }
    return out;
}

struct component_t {
    std::string model_name;
    std::string component_name;

    component_t(std::string m, std::string c): model_name(m), component_name(c) {}
};
std::ostream& operator<<(std::ostream& out, component_t o) {
    out << o.model_name << ":" << o.component_name;
    return out;
}

struct port_t {
    std::string component;
    std::string port;

    port_t(std::string c, std::string p): component(c), port(p) {}   
};
std::ostream& operator<<(std::ostream& out, port_t p) {
    out << p.component << "." << p.port;
    return out;
}

struct coupling_t {
    port_t from;
    port_t to;

    coupling_t(port_t f, port_t t): from(f), to(t) {}
};
std::ostream& operator<<(std::ostream& out, coupling_t c) {
    out << c.from << " --> " << c.to;
    return out;
}

#endif //DATATYPES_CONSTANTS_HPP