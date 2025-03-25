/**
 * Coupled Parser for DEVSMap
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
 */

#ifndef COUPLED_PARSER_HPP
#define COUPLED_PARSER_HPP

#include <fstream>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>
#include "datatypes.hpp"

using json = nlohmann::json;

const std::string fixed_keys_coupeld[] = {"include_sets"};

/////////////////////////////////////PARSER/////////////////////////////////////

class CoupledParser {

    protected:
    std::vector<std::string> sets;
    json model;
    std::vector<object_t> input;
    std::vector<object_t> output;
    std::vector<component_t> components;
    std::vector<coupling_t> eic;
    std::vector<coupling_t> eoc;
    std::vector<coupling_t> ic;


    private:
    json DEVSMap;

    void parse_top_level() {
        std::vector<std::string> custom_keys;
        //! find custom keys and collect parameters and include set
        for(auto& [key, value] : DEVSMap.items()){

            bool contains = false;
            for(auto& fixed_key : fixed_keys_coupeld) {
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

    void parse_xy() {
        for(auto& [key, value] : model.items()) {
            if(key == "x") {
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

    void parse_components() {
        for(auto& [key, value] : model.items()) {
            if(key == "components") {
                for(auto& [mn, cn] : value.items()) {
                    components.push_back(component_t(mn, cn));
                }
            }
        }
    }

    void parse_couplings() {
        for(auto& [key, value] : model.items()) {
            if(key == "ic") {
                for(auto& coupling : value.get<std::vector<json>>()) {
                    std::string c1, c2, p1, p2;
                    for(auto& [k, val] : coupling.items()) {
                        if(k == "port_from") {
                            p1 = val;
                        }
                        if(k == "port_to") {
                            p2 = val;
                        }
                        if(k == "component_from") {
                            c1 = val;
                        }
                        if(k == "component_to") {
                            c2 = val;
                        }
                    }
                    ic.push_back(coupling_t(port_t(c1, p1), port_t(c2, p2)));
                }
            } else if(key == "eic") {
                for(auto& coupling : value.get<std::vector<json>>()) {
                    std::string c2, p1, p2;
                    for(auto& [k, val] : coupling.items()) {
                        if(k == "port_from") {
                            p1 = val;
                        }
                        if(k == "port_to") {
                            p2 = val;
                        }
                        if(k == "component_to") {
                            c2 = val;
                        }
                    }
                    eic.push_back(coupling_t(port_t(model_name, p1), port_t(c2, p2)));
                }
            } else if(key == "eoc") {
                for(auto& coupling : value.get<std::vector<json>>()) {
                    std::string c1, p1, p2;
                    for(auto& [k, val] : coupling.items()) {
                        if(k == "port_from") {
                            p1 = val;
                        }
                        if(k == "port_to") {
                            p2 = val;
                        }
                        if(k == "component_from") {
                            c1 = val;
                        }
                    }
                    eoc.push_back(coupling_t(port_t(c1, p1), port_t(model_name, p2)));
                }
            }
        }
    }
    
    public:
    std::string model_name;

    CoupledParser(std::string fileName, bool verbose = true) {
        std::ifstream coupledFile(fileName);
        DEVSMap = json::parse(coupledFile);
        coupledFile.close();

        parse_top_level();

        model = DEVSMap.at(model_name);

        parse_xy();
        parse_components();
        parse_couplings();

        if (verbose) {
            std::cout << "model name: " << model_name << "\n";
            std::cout << "\nsets: ";
            std::copy(sets.begin(), sets.end(), std::ostream_iterator<std::string>(std::cout, " "));
            std::cout << "\ninput: ";
            std::copy(input.begin(), input.end(), std::ostream_iterator<object_t>(std::cout, " "));
            std::cout << "\noutput: ";
            std::copy(output.begin(), output.end(), std::ostream_iterator<object_t>(std::cout, " "));
            std::cout << "\ncomponents:\n\t";
            std::copy(components.begin(), components.end(), std::ostream_iterator<component_t>(std::cout, "\n\t"));
            std::cout << "\nic: ";
            std::copy(ic.begin(), ic.end(), std::ostream_iterator<coupling_t>(std::cout, "; "));
            std::cout << "\neic: ";
            std::copy(eic.begin(), eic.end(), std::ostream_iterator<coupling_t>(std::cout, "; "));
            std::cout << "\neoc: ";
            std::copy(eoc.begin(), eoc.end(), std::ostream_iterator<coupling_t>(std::cout, "; "));
            std::cout << std::endl;
        }

    }


    virtual std::string make_ports() = 0;
    virtual std::string make_components() = 0;
    virtual std::string make_couplings() = 0;
    virtual std::string make_model() = 0;
};

#endif //COUPLED_PARSER_HPP