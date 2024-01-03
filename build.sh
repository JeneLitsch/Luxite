cmake . -D CMAKE_CXX_COMPILER=clang++ -D param=linux-release -B build -S .
cd build
make 
cd ..
