#ifndef __DEVSMAP_PARSER_HPP__
#define __DEVSMAP_PARSER_HPP__

#include "AtomicParser.hpp"
#include "CoupledParser.hpp"
#include <filesystem>

template<typename AMP = AtomicParser, typename CMP = CoupledParser>
class Parser {
    private:
    json model_under_test;
    json experimental_frame;

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
    std::string file_type_from_name(const std::filesystem::path dir_entry){
        std::vector<std::string> result;
        std::stringstream ss(dir_entry);
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
    
    /**
     * Returns the file type by reading the filename
     */
    std::string get_path(const std::string filename){
        std::vector<std::string> result;
        std::stringstream ss(filename);
        std::string segment;

        std::string directory;

        //split by '/'
        while (std::getline(ss, segment, '/')) {
            result.push_back(segment);
        }

        std::stringstream ss1(result.back());
        
        std::vector<std::string> result2;
        //split by '.'
        while (std::getline(ss1, segment, '.')) {
            result2.push_back(segment);
        }

        if(result2.back() !=  "json") {
            std::cerr << "NON JSON FILE PROVIDED IN DIRECTORY" << std::endl;
            //!TODO EXIT ON EXCEPTION
        } else {
            for(size_t i = 0; i < result.size() - 1; i++) { //rejoin everything but last
                directory += result.at(i) + "/";
            }
        }

        return directory;
    }

    public:
    Parser(std::string experiment_file, std::string output_directory) {

        std::error_code err;
        if (!CreateDirectoryRecursive(output_directory + "/include", err)) {
            std::cerr << "CreateDirectoryRecursive FAILED, err: " << err.message() << std::endl;
        }
        
        std::ifstream experimentFile(experiment_file);
        auto DEVSMap = json::parse(experimentFile);
        experimentFile.close();

        model_under_test = DEVSMap.at("model_under_test");
        experimental_frame = DEVSMap.at("experimental_frame");

        //std::clog << model_under_test.dump(2) << std::endl << experimental_frame.dump(2) << std::endl;



        const std::filesystem::path DEVSMap_path{get_path(experiment_file)};

        if(experimental_frame.empty()) {
            std::cerr << "NO EXPERIMENTAL FRAME IN EXPERIMENT" << std::endl;
        }

        
    
        // for(auto const& dir_entry: std::filesystem::directory_iterator{DEVSMap_path}) {
        //     std::vector<object_t> dummy; //dummy state set

            
        //     auto file_type = file_type_from_name(dir_entry);
        //     if(file_type == "atomic") {
        //         auto parser = AMP(dir_entry.path(), dummy);
        //         std::string filename = output_directory + "/include/" + parser.model_name + ".hpp";
        //         std::ofstream file(filename.c_str());
        //         file << parser.make_model() << std::endl;
        //         file.close();
        //     } else if(file_type == "coupled") {
        //         auto parser = CMP(dir_entry.path(), dummy);
        //         std::string filename = output_directory + "/include/" + parser.model_name + ".hpp";
        //         std::ofstream file(filename.c_str());
        //         file << parser.make_model() << std::endl;
        //         file.close();
        //     } else {
        //         std::cout << dir_entry.path() << " not supported" << std::endl;
        //     }
    
    
    
        // }
    }
};

#endif