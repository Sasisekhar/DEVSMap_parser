#include <iostream>
#include <string>
#include <filesystem>
#include "CadmiumCoupledParser.hpp"

// Returns:
//   true upon success.
//   false upon failure, and set the std::error_code & err accordingly.
bool CreateDirectoryRecursive(std::string const & dirName, std::error_code & err)
{
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

int main(int argc, char** argv) {

    if(argc < 2) {
        std::cerr << "Error: Too few arguments. Typical usage:\n" << argv[0] << " <Path to JSON file>" << std::endl;
        return 0;
    }
    
    // auto parser = CadmiumAtomicParser(argv[1], false);
    // auto parser = CadmiumCoupledParser(argv[1], false);

    std::error_code err;
    if (!CreateDirectoryRecursive("parser_output", err)) {
        std::cout << "CreateDirectoryRecursive FAILED, err: " << err.message() << std::endl;
    }

    const std::filesystem::path DEVSMap_path{argv[1]};

    for(auto const& dir_entry: std::filesystem::directory_iterator{DEVSMap_path}) {
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
            return -1;
        }

        std::stringstream ss2(result.at(0));
        result.clear();
        //split by '_'
        while (std::getline(ss2, segment, '_')) {
            result.push_back(segment);
        }

        std::string file_type = result.back();

        if(file_type == "atomic") {
            auto parser = CadmiumAtomicParser(dir_entry.path(), false);
            std::string filename = "parser_output/" + parser.model_name + ".hpp";
            std::ofstream file(filename.c_str());
            file << parser.make_model() << std::endl;
            file.close();
        } else if(file_type == "coupled") {
            auto parser = CadmiumCoupledParser(dir_entry.path(), false);
            std::string filename = "parser_output/" + parser.model_name + ".hpp";
            std::ofstream file(filename.c_str());
            file << parser.make_model() << std::endl;
            file.close();
        } else {
            std::cout << dir_entry.path() << " not supported" << std::endl;
        }



    }

    return 0;
}