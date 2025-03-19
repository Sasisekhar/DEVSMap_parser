/**
 * Atomic Parser for DEVSMap
 * Copyright (C) 2025  Sasisekhar Mangalam Govind
 * ARSLab - Carleton University
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 * 
 * 
 * TODO list:
 * - Assert "Otherwise"
 * - Get initial state
 */

#ifndef ATOMIC_PARSER_HPP
#define ATOMIC_PARSER_HPP

#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <regex>
#include <unordered_set>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

const std::string fixed_keys[] = {"include_sets", "parameters"};

const std::unordered_set<std::string> operators = {
    "==", "!=", "<=", ">=", "<", ">", "&&", "||",
    "(", ")", "+", "-", "*", "/", "%",
    ".bag", ".bagSize"
};

///////////////////////////////////DATATYPES///////////////////////////////////

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

/////////////////////////////////////PARSER/////////////////////////////////////

class AtomicParser {

    protected:
    std::vector<std::string> sets;
    json parameters;
    json model;
    std::vector<object_t> state_set;
    std::vector<object_t> input;
    std::vector<object_t> output;
    std::vector<std::shared_ptr<transition_t>> dint;
    std::vector<std::shared_ptr<transition_t>> dext;
    std::vector<std::shared_ptr<transition_t>> dcon;
    std::vector<std::shared_ptr<transition_t>> lambda;
    std::vector<std::shared_ptr<ta_t>> ta;

    /**
     * Takes the tokens, and classifies them further
     */
    std::vector<Token> tokenize_classify(const std::string& condition) {

        std::vector<Token> tokens;
        std::regex token_regex(R"((==|!=|<=|>=|&&|\|\||[()<>\+\-\*/%])|([A-Za-z_]\w*(?:\.bag\([^\)]+\)|\.bagSize\(\))?)|(\d+\.\d+|\d+)|(\".*?\"|\'.*?\'))");

        std::smatch match;
        std::string s = condition;

        while (std::regex_search(s, match, token_regex)) {
            std::string token_str = match[0];

            Token token;
            token.value = token_str;

            if (operators.find(token_str) != operators.end()) {
                token.type = TokenType::OPERATOR;
            } else if (std::regex_match(token_str, std::regex(R"(\d+\.\d+|\d+)"))) {
                token.type = TokenType::CONSTANT;
            } else if (std::regex_match(token_str, std::regex(R"(\".*?\"|\'.*?\')"))) {
                token.type = TokenType::CONSTANT;
            } else {
                token.type = TokenType::UNKNOWN;
            }

            tokens.push_back(token);
            s = match.suffix();
        }

        // Classify UNKNOWN tokens clearly
        std::unordered_set<std::string> state_vars, params, input_ports, output_ports;
        for (const auto& s : state_set) state_vars.insert(s.variable);
        for (auto& [key, _] : parameters.items()) params.insert(key);
        for (const auto& s : input) input_ports.insert(s.variable);
        for (const auto& s : output) output_ports.insert(s.variable);

        for (auto& token : tokens) {
            if (token.type == TokenType::UNKNOWN) {

                std::string base_var = token.value;

                // Check if token contains .bag() or .bagSize()
                size_t pos = token.value.find('.');
                if (pos != std::string::npos) {
                    base_var = token.value.substr(0, pos);
                }

                if (input_ports.find(base_var) != input_ports.end()) {
                    token.type = TokenType::INPUT_PORT;
                } else if (output_ports.find(base_var) != output_ports.end()) {
                    token.type = TokenType::OUTPUT_PORT;
                } else if (state_vars.find(base_var) != state_vars.end()) {
                    token.type = TokenType::STATE_VARIABLE;
                } else if (params.find(base_var) != params.end()) {
                    token.type = TokenType::PARAMETER;
                } else {
                    token.type = TokenType::CONSTANT;
                }
            }
        }

        return tokens;
    }


    private:
    json DEVSMap;

    void parse_top_level() {
        std::vector<std::string> custom_keys;
        //! find custom keys and collect parameters and include set
        for(auto& [key, value] : DEVSMap.items()){

            bool contains = false;
            for(auto& fixed_key : fixed_keys) {
                if(key == fixed_key) {
                    contains = true;
                }
            }

            if(!contains) {
                custom_keys.push_back(key);
            }

            if(key == "include_sets") {
                if(value.empty()) {
                    std::cerr << "No include_sets; fatal error" << std::endl;
                    throw std::runtime_error("'INCLUDE_SETS' KEY IS MANDATORY");
                }

                for(auto& set : value) {
                    sets.push_back(set);
                }
            }

            if(key == "parameters") {
                parameters = value;
            }
        }

        if(custom_keys.size() > 1) {
            std::cout << "Multiple custom keys defined:\n";

            int i = 0;
            for(auto& key : custom_keys) {
                std::cout << i++ << " " << key << "\n";
            }
            std::cout << "Enter a number between 0 and " << i - 1 << "to choose the model name key: ";
            i = 0;
            std::cin >> i;
            std::cout << std::endl;

            model_name = custom_keys.at(i);
        } else {
            model_name = custom_keys.back();
        }
    }

    void parse_xys() {
        for(auto& [key, value] : model.items()) {
            if(key == "s") {
                for(auto& [sv, dt] : value.items()) {
                    state_set.push_back(object_t(sv, dt));
                }
            } else if(key == "x") {
                for(auto& [x, dt] : value.items()) {
                    input.push_back(object_t(x, dt));
                }
            } else if(key == "y") {
                for(auto& [y, dt] : value.items()) {
                    output.push_back(object_t(y, dt));
                }
            }
        }
    }

    std::shared_ptr<transition_t> parse_transitions(json transition, const std::string& condition) {
        auto return_object = std::make_shared<transition_t>(condition);
    
        for(auto& [key, value] : transition.items()) {
            if(value.is_string()) {
                return_object->new_state.emplace_back(key, value.get<std::string>());
            } else if(value.is_object()) {
                return_object->nested.push_back(parse_transitions(value, key));
            } else {
                throw std::runtime_error("Invalid JSON type encountered in ta parsing.");
            }
        }
    
        return return_object;
    }

    std::shared_ptr<ta_t> parse_ta(const json& ta_json, const std::string& condition = "") {
        auto node = std::make_shared<ta_t>(condition);
    
        if (ta_json.is_string()) {
            node->expression = ta_json.get<std::string>();
        } else if (ta_json.is_object()) {
            for (auto& [key, value] : ta_json.items()) {
                node->nested.push_back(parse_ta(value, key));
            }
        } else {
            throw std::runtime_error("Invalid TA structure encountered.");
        }
    
        return node;
    }

    void parse_transitions() {
        for (auto& [condition, transition_json] : model["delta_int"].items()) {
            dint.push_back(parse_transitions(transition_json, condition));
        }
        for (auto& [condition, transition_json] : model["delta_ext"].items()) {
            dext.push_back(parse_transitions(transition_json, condition));
        }
        for (auto& [condition, transition_json] : model["delta_con"].items()) {
            dcon.push_back(parse_transitions(transition_json, condition));
        }
        for (auto& [condition, transition_json] : model["lambda"].items()) {
            lambda.push_back(parse_transitions(transition_json, condition));
        }
        for (auto& [condition, transition_json] : model["ta"].items()) {
            ta .push_back(parse_ta(transition_json, condition));
        }
    
    }

    public:
    std::string model_name;
    AtomicParser(std::string fileName, bool verbose = true) {
        std::ifstream atomicFile(fileName);
        DEVSMap = json::parse(atomicFile);
        atomicFile.close();

        parse_top_level();

        model = DEVSMap.at(model_name);

        parse_xys();

        parse_transitions();

        if (verbose) {
            std::cout << "model name: " << model_name << "\n";
            std::cout << "sets: ";
            std::copy(sets.begin(), sets.end(), std::ostream_iterator<std::string>(std::cout, " "));
            std::cout << "\nparameters: " << parameters.dump(2) << "\n";
            std::cout << "input: ";
            std::copy(input.begin(), input.end(), std::ostream_iterator<object_t>(std::cout, " "));
            std::cout << "output: ";
            std::copy(output.begin(), output.end(), std::ostream_iterator<object_t>(std::cout, " "));
            std::cout << "\ndelta_int transitions:\n";
            for (const auto& t : dint) {
                std::cout << *t;
            }
            std::cout << "\ndelta_ext transitions:\n";
            for (const auto& t : dext) {
                std::cout << *t;
            }
            std::cout << "\ndelta_con transitions:\n";
            for (const auto& t : dcon) {
                std::cout << *t;
            }
            std::cout << "\nlambda transitions:\n";
            for (const auto& t : lambda) {
                std::cout << *t;
            }
            std::cout << "\tta transitions:\n";
            for (const auto& t : ta) {
                std::cout << *t;
            }
            std::cout << std::endl;
        }

    }
    
    virtual std::string make_state() = 0;
    virtual std::string make_ports() = 0;
    virtual std::string make_internal_transition() = 0;
    virtual std::string make_external_transition() = 0;
    virtual std::string make_confluent_transition() = 0;
    virtual std::string make_lambda() = 0;
    virtual std::string make_ta() = 0;
    virtual std::string make_model() = 0;

};

#endif //ATOIMC_PARSER_HPP