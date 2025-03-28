/**
 * Cadmium coupled parser for DEVSMap
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
 */

#ifndef CADMIUM_COUPLED_PARSER_HPP
#define CADMIUM_COUPLED_PARSER_HPP

#include "CoupledParser.hpp"
#include "CadmiumAtomicParser.hpp"

class CadmiumCoupledParser : public CoupledParser {
    private:
    std::string indent = "";
    std::string file_path;

    public:
    CadmiumCoupledParser(std::string fileName, std::vector<object_t> state_set, bool flag = false): CoupledParser(fileName, state_set, flag) {
        std::vector<std::string> result;
        std::stringstream ss(fileName);
        std::string segment;

        while (std::getline(ss, segment, '/')) {
            result.push_back(segment);
        }

        file_path = "";

        for(size_t i; i < result.size() - 1; i++) {
            file_path += result.at(i) + "/";
        }
        
    }

    std::string make_ports() {
        std::ostringstream oss;

        for(auto& port : input) {
            oss << "\tPort<" << port.datatype << "> " << port.variable << ";\n";
        }
        for(auto& port : output) {
            oss << "\tPort<" << port.datatype << "> " << port.variable << ";\n";
        }

        //Constructor
        oss << "\t" << model_name << "(const std::string& id) : Coupled(id) {\n";

        for(auto& port : input) {
            oss << "\t\t" << port.variable << " = addInPort<" << port.datatype << ">(\"" << port.variable << "\");\n";
        }
        for(auto& port : output) {
            oss << "\t\t" << port.variable << " = addOutPort<" << port.datatype << ">(\"" << port.variable << "\");\n";
        }

        oss << std::endl;


        return oss.str();
    }
    
    std::string make_components() {
        std::ostringstream oss;

        for(auto& component: components) {
            oss << "\t\tauto " << component.component_name << " = addComponent<" << component.model_name << ">(\"" << component.component_name << "\");\n";
        }

        return oss.str();
    }

    std::string make_couplings() {

        std::ostringstream oss;

        for(auto& coupling: ic) {
            oss << "\t\taddCoupling(" << coupling.from.component << "->" << coupling.from.port << ", " << coupling.to.component << "->" << coupling.to.port << ");\n";
        }
        for(auto& coupling: eic) {
            oss << "\t\taddCoupling(" << coupling.from.port << ", " << coupling.to.component << "->" << coupling.to.port << ");\n";
        }
        for(auto& coupling: eoc) {
            oss << "\t\taddCoupling(" << coupling.from.component << "->" << coupling.from.port << ", " << coupling.to.port << ");\n";
        }
        
        return oss.str();
    }
    
    std::string make_model() {
        std::ostringstream oss;

        std::string MODEL_NAME = model_name;
        std::transform(MODEL_NAME.begin(), MODEL_NAME.end(), MODEL_NAME.begin(), ::toupper);

        oss << "#ifndef __DEVSMAP__PARSER__" << MODEL_NAME << "__HPP__\n";
        oss << "#define __DEVSMAP__PARSER__" << MODEL_NAME << "__HPP__\n\n";
        oss << "#include <iostream>\n#include \"cadmium/modeling/devs/coupled.hpp\"\n";
        
        for(auto component: components) {
            oss << "#include \"" << component.model_name + ".hpp\"\n";
        }

        oss << std::endl;

        oss << "using namespace cadmium;\n\n";

        oss << "struct " << model_name << ": public Coupled {\n\n";
        
        oss << make_ports() << std::endl;

        oss << make_components() << std::endl;

        oss << make_couplings() << std::endl;

        oss << "\t}\n};\n#endif //__DEVSMAP__PARSER__" << MODEL_NAME << "__HPP__\n";

        return oss.str();
    }
    

};

#endif