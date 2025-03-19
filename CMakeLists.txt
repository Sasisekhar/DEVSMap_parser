cmake_minimum_required(VERSION 3.16)

project(cpp_playground)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)

FILE(GLOB Examples RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.cpp)
foreach(exampleSrc ${Examples})
    get_filename_component(exampleName ${exampleSrc} NAME_WE)
    add_executable(${exampleName} ${exampleSrc})
    target_include_directories(${exampleName} PRIVATE "." "include" "$ENV{CADMIUM}" "$ENV{CADMIUM}/../json/include")
    target_compile_options(${exampleName} PUBLIC -std=gnu++2b)
endforeach(exampleSrc)