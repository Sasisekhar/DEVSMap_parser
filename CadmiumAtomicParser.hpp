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

    std::vector<Token> tokenize_condition(const std::string& condition) {
        std::vector<Token> tokens;
        std::regex token_regex(R"((==|!=|<=|>=|&&|\|\||[()<>\+\-\*/%])|([A-Za-z_]\w*)|(\d+\.\d+|\d+)|(\".*?\"|\'.*?\'))");
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
                // Initially mark as unknown; we'll classify next
                token.type = TokenType::UNKNOWN;
            }
    
            tokens.push_back(token);
            s = match.suffix();
        }
    
        return tokens;
    }

    void classify_tokens(std::vector<Token>& tokens,
                     const std::vector<object_t>& state_set,
                     const json& parameters) {

        std::unordered_set<std::string> state_vars, params;
        for (const auto& s : state_set) state_vars.insert(s.variable);
        for (auto& [key, _] : parameters.items()) params.insert(key);

        for (auto& token : tokens) {
            if (token.type == TokenType::UNKNOWN) {
                if (state_vars.find(token.value) != state_vars.end()) {
                    token.type = TokenType::STATE_VARIABLE;
                } else if (params.find(token.value) != params.end()) {
                    token.type = TokenType::PARAMETER;
                } else {
                    // If not found anywhere, assume constant or error
                    token.type = TokenType::CONSTANT; // Or handle as an error
                }
            }
        }
    }

    std::string reconstruct_condition(const std::vector<Token>& tokens, const std::string& state_obj = "state") {
        std::ostringstream oss;
    
        for (auto& token : tokens) {
            switch (token.type) {
                case TokenType::OPERATOR:
                    oss << " " << token.value << " ";
                    break;
                case TokenType::STATE_VARIABLE:
                    oss << state_obj << "." << token.value;
                    break;
                case TokenType::PARAMETER:
                    oss << token.value;
                    break;
                case TokenType::CONSTANT:
                    oss << token.value;
                    break;
                default:
                    oss << token.value; // Fallback
            }
        }
        return oss.str();
    }

    
    std::string generate_if_else_processed(const std::shared_ptr<transition_t>& transition,
                                       const std::string& state_obj,
                                       const std::vector<object_t>& state_set,
                                       const json& parameters,
                                       const std::string& indent = "\t") {
        std::ostringstream oss;

        if (!transition->condition.empty()) {
            auto tokens = tokenize_condition(transition->condition);
            classify_tokens(tokens, state_set, parameters);
            std::string processed_condition = reconstruct_condition(tokens, state_obj);

            if (transition->condition == "otherwise") {
                oss << indent << "else {\n";
            } else {
                oss << indent << "if (" << processed_condition << ") {\n";
            }
        }

        // State assignments
        for (const auto& state : transition->new_state) {

            auto expr_tokens = tokenize_condition(state.expression);
            classify_tokens(expr_tokens, state_set, parameters);
            auto processed_expression = reconstruct_condition(expr_tokens, state_obj);

            oss << indent << "\t" << state_obj << "." << state.state_variable << " = "
                << processed_expression << ";\n";
        }

        // Nested conditions
        for (const auto& nested_transition : transition->nested) {
            oss << generate_if_else_processed(nested_transition, state_obj, state_set, parameters, indent + "\t");
        }

        if (!transition->condition.empty()) {
            oss << indent << "}\n";
        }

        return oss.str();
    }


    public:
    CadmiumAtomicParser(std::string fileName): AtomicParser(fileName, false) {}

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
            oss << generate_if_else_processed(transition, "state", state_set, parameters, "\t\t");
        }
        oss << "\t}\n";

        return oss.str();
    }
    
    std::string make_external_transition() {
        return "TODO";
    }
    
    std::string make_confluent_transition() {
        return "TODO";
    }
    
    std::string make_lambda() {
        return "TODO";
    }
    
    std::string make_ta() {
        return "TODO";
    }
    
    std::string make_model() {
        return "TODO";
    }
    

};

#endif //CADMIUM_ATOMIC_PARSER_HPP