#! /bin/bash

if [ -d "build" ]; then rm -Rf build; fi
mkdir -p build
cd build || exit
rm -rf *
cmake ..
make
cd ..
echo "Compilation done. Executable(s) in the bin folder"