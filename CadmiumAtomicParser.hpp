/**
 * Cadmium atomic parser for DEVSMap
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
 */

#ifndef CADMIUM_ATOMIC_PARSER_HPP
#define CADMIUM_ATOMIC_PARSER_HPP

#include "AtomicParser.hpp"

class CadmiumAtomicParser : public AtomicParser {
    private:
    std::string indent = "";

    std::string reconstruct_condition(const std::vector<Token>& tokens, 
                                  const std::string& state_obj = "state") {
    std::ostringstream oss;

    std::regex bag_regex(R"((\w+)\.bag\((-?\d+)\))");
    std::regex bagSize_regex(R"((\w+)\.bagSize\(\))");
    std::smatch match;

    for (auto& token : tokens) {
        switch (token.type) {
            case TokenType::OPERATOR:
                oss << " " << token.value << " ";
                break;

            case TokenType::STATE_VARIABLE:
                oss << state_obj << "." << token.value;
                break;

            case TokenType::PARAMETER:
                oss << "params." << token.value;
                break;

            case TokenType::INPUT_PORT:
                // Check for bagSize()
                if (std::regex_match(token.value, match, bagSize_regex)) {
                    std::string port_name = match[1];
                    oss << port_name << "->getBag().size()";
                }
                // Check for bag(index)
                else if (std::regex_match(token.value, match, bag_regex)) {
                    std::string port_name = match[1];
                    int index = std::stoi(match[2]);

                    if (index >= 0) {
                        oss << port_name << "->getBag().at(" << index << ")";
                    } else {
                        oss << port_name << "->getBag().at(" 
                            << port_name << "->getBag().size() - " << -index << ")";
                    }
                } else {
                    oss << token.value;
                }
                break;

            case TokenType::OUTPUT_PORT:
                oss << token.value;
                break;

            case TokenType::CONSTANT:
                oss << token.value;
                break;

            default:
                oss << token.value;
        }
    }

    return oss.str();
}

    
    std::string generate_if_else(const std::shared_ptr<transition_t>& transition,
                                       const std::string& state_obj,
                                       const std::string& indent = "\t") {
        std::ostringstream oss;

        if (!transition->condition.empty()) {
            auto tokens = tokenize_classify(transition->condition);
            std::string processed_condition = reconstruct_condition(tokens, state_obj);

            if (transition->condition == "otherwise") {
                oss << indent << "else {\n";
            } else {
                oss << indent << "if (" << processed_condition << ") {\n";
            }
        }

        // State assignments
        for (const auto& state : transition->new_state) {

            auto expr_tokens = tokenize_classify(state.expression);
            auto processed_expression = reconstruct_condition(expr_tokens, state_obj);

            oss << indent << "\t" << state_obj << "." << state.state_variable << " = "
                << processed_expression << ";\n";
        }

        // Nested conditions
        for (const auto& nested_transition : transition->nested) {
            oss << generate_if_else(nested_transition, state_obj, indent + "\t");
        }

        if (!transition->condition.empty()) {
            oss << indent << "}\n";
        }

        return oss.str();
    }


    public:
    CadmiumAtomicParser(std::string fileName, bool flag): AtomicParser(fileName, flag) {}

    std::string make_state() {
        std::string struct_name = model_name + "State";
        std::string state_struct = indent + "struct " + struct_name + " {\n";

        for(auto& sv : state_set) {
            state_struct += indent + "\t" + sv.datatype + " " + sv.variable + ";\n";
        }

        state_struct += "\n";

        // Default constructor
        state_struct += indent + "\t" + struct_name + "() : ";
        for(size_t i = 0; i < state_set.size(); ++i) {
            state_struct += state_set[i].variable + "()";
            state_struct += (i < state_set.size() - 1) ? ", " : " {}\n";
        }

        state_struct += indent + "};\n\n";

        // operator<< overload
        state_struct += indent + "std::ostream& operator<<(std::ostream& out, const " + struct_name + "& s) {\n";
        state_struct += indent + "\tout << \"{\"";
        for(size_t i = 0; i < state_set.size(); ++i) {
            state_struct += " << s." + state_set[i].variable;
            if(i < state_set.size() - 1)
                state_struct += " << \", \"";
        }
        state_struct += " << \"}\";\n";
        state_struct += indent + "\treturn out;\n";
        state_struct += indent + "}\n";

        return state_struct;
    }

    std::string make_ports() {
        return "TODO";
    }
    
    std::string make_internal_transition() {
        std::ostringstream oss;

        oss << "\tvoid internalTransition(" << model_name << "State& state) const override {\n";

        for(auto& transition : dint) {
            oss << generate_if_else(transition, "state", "\t\t");
        }
        oss << "\t}\n";

        return oss.str();
    }
    
    std::string make_external_transition() {
        std::ostringstream oss;

        oss << "\tvoid externalTransition(" << model_name << "State& state, double e) const override {\n";

        for(auto& transition : dext) {
            oss << generate_if_else(transition, "state", "\t\t");
        }
        oss << "\t}\n";

        return oss.str();
    }
    
    std::string make_confluent_transition() {
        std::ostringstream oss;

        oss << "\tvoid confluentTransition(" << model_name << "State& state, double e) const override {\n";

        for(auto& transition : dext) {
            oss << generate_if_else(transition, "state", "\t\t");
        }
        oss << "\t}\n";

        return oss.str();
    }
    
    std::string make_lambda() {
        std::ostringstream oss;

        oss << "\tvoid externalTransition(" << model_name << "State& state, double e) const override {\n";

        for(auto& transition : dext) {
            oss << generate_if_else(transition, "state", "\t\t");
        }
        oss << "\t}\n";

        return oss.str();
    }
    
    std::string make_ta() {
        return "TODO";
    }
    
    std::string make_model() {
        return "TODO";
    }
    

};

#endif //CADMIUM_ATOMIC_PARSER_HPP