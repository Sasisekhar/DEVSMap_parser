#ifndef __DEVSMAP_PARSER_HPP__
#define __DEVSMAP_PARSER_HPP__

#include "AtomicParser.hpp"
#include "CoupledParser.hpp"
#include <filesystem>

template<typename AMP = AtomicParser, typename CMP = CoupledParser>
class Parser {
    private:
    // Returns:
    //   true upon success.
    //   false upon failure, and set the std::error_code & err accordingly.
    bool CreateDirectoryRecursive(std::string const & dirName, std::error_code & err) {
        err.clear();
        if (!std::filesystem::create_directories(dirName, err))
        {
            if (std::filesystem::exists(dirName))
            {
                // The folder already exists:
                err.clear();
                return true;    
            }
            return false;
        }
        return true;
    }
    
    /**
     * Returns the file type by reading the filename
     */
    std::string file_type_from_name(const std::filesystem::directory_iterator dir_entry){
        std::vector<std::string> result;
        std::stringstream ss(dir_entry.path());
        std::string segment;

        //split by '/'
        while (std::getline(ss, segment, '/')) {
            result.push_back(segment);
        }

        std::stringstream ss1(result.back());
        result.clear();
        //split by '.'
        while (std::getline(ss1, segment, '.')) {
            result.push_back(segment);
        }

        if(result.back() !=  "json") {
            std::cerr << "NON JSON FILE PROVIDED IN DIRECTORY" << std::endl;
            //!TODO EXIT ON EXCEPTION   
            // return -1;
        }

        std::stringstream ss2(result.at(0));
        result.clear();
        //split by '_'
        while (std::getline(ss2, segment, '_')) {
            result.push_back(segment);
        }

        return result.back();
    }
    
    public:
    Parser(std::string experiment_file, std::string output_directory) {

        std::error_code err;
        if (!CreateDirectoryRecursive(output_directory, err)) {
            std::cout << "CreateDirectoryRecursive FAILED, err: " << err.message() << std::endl;
        }
    
        const std::filesystem::path DEVSMap_path{directory};
    
        for(auto const& dir_entry: std::filesystem::directory_iterator{DEVSMap_path}) {
            std::vector<object_t> dummy; //dummy state set

            
            auto file_type = file_type_from_name(dir_entry);
            if(file_type == "atomic") {
                auto parser = AMP(dir_entry.path(), dummy);
                std::string filename = "parser_output/" + parser.model_name + ".hpp";
                std::ofstream file(filename.c_str());
                file << parser.make_model() << std::endl;
                file.close();
            } else if(file_type == "coupled") {
                auto parser = CMP(dir_entry.path(), dummy);
                std::string filename = "parser_output/" + parser.model_name + ".hpp";
                std::ofstream file(filename.c_str());
                file << parser.make_model() << std::endl;
                file.close();
            } else {
                std::cout << dir_entry.path() << " not supported" << std::endl;
            }
    
    
    
        }
    }
};

#endif