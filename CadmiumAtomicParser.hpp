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
 * - Add include sets
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
                oss << token.value;
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

    /**
     * @brief Generates the if-else ladder for the transition functions and the output function
     * 
     * @param vec_transition 
     * @param state_obj 
     * @param transition_flag 
     * @param indent 
     * @return std::string 
     */
    std::string generate_if_else(   const std::vector<std::shared_ptr<transition_t>> vec_transition,
                                    const std::string& state_obj,
                                    const bool transition_flag,
                                    const std::string& indent = "\t") {
        std::ostringstream oss;
        bool first_flag = true;
        bool only_otherwise = false;

        for(auto& transition : vec_transition) {
            if (!transition->condition.empty()) {
                auto tokens = tokenize_classify(transition->condition);
                std::string processed_condition = reconstruct_condition(tokens, state_obj);

                if (transition->condition == "otherwise") {
                    if(vec_transition.size() > 1) { //if only otherwise, no else{}
                        oss << indent << "else {\n";
                    } else {
                        oss << "\n";
                        only_otherwise = true;
                    }
                } else {
                    if(first_flag){ //first if, then else if
                        first_flag = false;
                        oss << indent << "if (" << processed_condition << ") {\n";
                    } else {
                        oss << indent << "else if (" << processed_condition << ") {\n";
                    }
                }
            }

            if(transition_flag){
                // State assignments
                for (const auto& state : transition->new_state) {
                    auto variable = reconstruct_condition(tokenize_classify(state.state_variable), state_obj);
                    auto expression = reconstruct_condition(tokenize_classify(state.expression), state_obj);

                    oss << indent << "\t" << variable << " = " << expression << ";\n";
                }
            } else {
                for (const auto& state : transition->new_state) {
                    auto variable = reconstruct_condition(tokenize_classify(state.state_variable), state_obj);
                    auto expression = reconstruct_condition(tokenize_classify(state.expression), state_obj);

                    oss << indent << "\t" << variable << "->addMessage(" << expression << ");\n";
                }
            }

            // Nested conditions
            oss << generate_if_else(transition->nested, state_obj, transition_flag, indent + "\t");

            if (!transition->condition.empty()) {
                if(only_otherwise) {
                    oss << "\n";
                } else {
                    oss << indent << "}\n";
                }
            }

        }

        return oss.str();
    }

    /**
     * @brief Generates the if-else ladder for the time advance fucntion
     * 
     * @param vec_transition 
     * @param state_obj 
     * @param transition_flag 
     * @param indent 
     * @return std::string 
     */
    std::string generate_if_else(   const std::vector<std::shared_ptr<ta_t>> vec_transition,
                                    const std::string& state_obj,
                                    const std::string& indent = "\t") {
        std::ostringstream oss;
        bool first_flag = true;
        bool only_otherwise = false;

        for(auto& transition : vec_transition) {
            if (!transition->condition.empty()) {
                auto tokens = tokenize_classify(transition->condition);
                std::string processed_condition = reconstruct_condition(tokens, state_obj);

                if (transition->condition == "otherwise") {
                    if(vec_transition.size() > 1) { //if only otherwise, no else{}
                        oss << indent << "else {\n";
                    } else {
                        oss << "\n";
                        only_otherwise = true;
                    }
                } else {
                    if(first_flag){ //first if, then else if
                        first_flag = false;
                        oss << indent << "if (" << processed_condition << ") {\n";
                    } else {
                        oss << indent << "else if (" << processed_condition << ") {\n";
                    }
                }
            }

            
            if(transition->expression != "") {
                auto expression = reconstruct_condition(tokenize_classify(transition->expression), state_obj);
                oss << indent << "\treturn " << expression << ";\n";
            }

            // Nested conditions
            oss << generate_if_else(transition->nested, state_obj, indent + "\t");

            if (!transition->condition.empty()) {
                if(only_otherwise) {
                    oss << "\n";
                } else {
                    oss << indent << "}\n";
                }
            }

        }

        return oss.str();
    }

    public:
    CadmiumAtomicParser(std::string fileName, bool flag): AtomicParser(fileName, flag) {}

    std::string make_state() {
        std::string struct_name = model_name + "State";
        std::string state_struct = "struct " + struct_name + " {\n";

        for(auto& sv : state_set) {
            state_struct += "\t" + sv.datatype + " " + sv.variable + ";\n";
        }

        state_struct += "\n";

        // Default constructor
        state_struct += "\t" + struct_name + "() : ";
        for(size_t i = 0; i < state_set.size(); ++i) {
            state_struct += state_set[i].variable + "()";
            state_struct += (i < state_set.size() - 1) ? ", " : " {}\n";
        }

        state_struct += "};\n";

        // operator<< overload
        state_struct += "std::ostream& operator<<(std::ostream& out, const " + struct_name + "& s) {\n";
        state_struct += "\tout << \"{\"";
        for(size_t i = 0; i < state_set.size(); ++i) {
            state_struct += " << s." + state_set[i].variable;
            if(i < state_set.size() - 1)
                state_struct += " << \", \"";
        }
        state_struct += " << \"}\";\n";
        state_struct += "\treturn out;\n";
        state_struct += "}\n";

        return state_struct;
    }

    std::string make_ports() {
        std::ostringstream oss;

        for(auto& port : input) {
            oss << "\tPort<" << port.datatype << "> " << port.variable << ";\n";
        }
        for(auto& port : output) {
            oss << "\tPort<" << port.datatype << "> " << port.variable << ";\n";
        }

        oss << std::endl;

        //Constructor
        oss << "\t" << model_name << "(const std::string id) : Atomic<" << model_name << "State>(id, " << model_name << "State()) {\n";

        for(auto& port : input) {
            oss << "\t\t" << port.variable << " = addInPort<" << port.datatype << ">(\"" << port.variable << "\");\n";
        }
        for(auto& port : output) {
            oss << "\t\t" << port.variable << " = addOutPort<" << port.datatype << ">(\"" << port.variable << "\");\n";
        }

        oss << "\t}\n";


        return oss.str();
    }
    
    std::string make_internal_transition() {
        std::ostringstream oss;

        oss << "\tvoid internalTransition(" << model_name << "State& state) const override {\n";
        oss << generate_if_else(dint, "state", _GLIBCXX_TR1_BETA_FUNCTION_TCC, "\t\t");
        oss << "\t}\n";

        return oss.str();
    }
    
    std::string make_external_transition() {
        std::ostringstream oss;

        oss << "\tvoid externalTransition(" << model_name << "State& state, double e) const override {\n";
        oss << generate_if_else(dext, "state", true, "\t\t");
        oss << "\t}\n";

        return oss.str();
    }
    
    std::string make_confluent_transition() {
        std::ostringstream oss;

        oss << "\tvoid confluentTransition(" << model_name << "State& state, double e) const override {\n";
        oss << generate_if_else(dcon, "state", true, "\t\t");
        oss << "\t}\n";

        return oss.str();
    }
    
    std::string make_lambda() {
        std::ostringstream oss;

        oss << "\tvoid output(const " << model_name << "State& state) const override {\n";
        oss << generate_if_else(lambda, "state", false, "\t\t");
        oss << "\t}\n";

        return oss.str();
    }
    
    std::string make_ta() {
        std::ostringstream oss;

        oss << "\t[[nodiscard]] double timeAdvance(const " << model_name << "State& state) const override {\n";
        oss << generate_if_else(ta, "state", "\t\t");
        oss << "\t}\n";

        return oss.str();
    }
    
    std::string make_model() {
        std::ostringstream oss;

        std::string MODEL_NAME = model_name;
        std::transform(MODEL_NAME.begin(), MODEL_NAME.end(), MODEL_NAME.begin(), ::toupper);

        oss << "#ifndef __DEVSMAP__PARSER__" << MODEL_NAME << "__HPP__\n";
        oss << "#define __DEVSMAP__PARSER__" << MODEL_NAME << "__HPP__\n\n";
        oss << "#include <iostream>\n#include \"cadmium/modeling/devs/atomic.hpp\"\n\n";

        oss << "using namespace cadmium;\n\n";

        oss << make_state() << std::endl;

        oss << "class " << model_name << ": public Atomic<" << model_name << "State>{\n\n";
        oss << "\tpublic:\n\n";
        
        oss << make_ports() << std::endl; //also constructor

        oss << make_internal_transition() << std::endl;
        oss << make_external_transition() << std::endl;
        oss << make_confluent_transition() << std::endl;
        oss << make_lambda() << std::endl;
        oss << make_ta() << std::endl;

        oss << "};\n\n";

        oss << "#endif //__DEVSMAP__PARSER__" << MODEL_NAME << "_HPP__\n";

        return oss.str();
    }
    

};

#endif //CADMIUM_ATOMIC_PARSER_HPP