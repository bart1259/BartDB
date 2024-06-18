# BartDB
A basic on disk B-Tree implementation for class

BTree library source code is found under `src/btreelib`

All test and benchmark code is found under `test`

This project relies on `g++` `make` and `cmake`

To compile:
```sh
mkdir build
cd build
cmake ..
make
```

To run unit/integration tests
```sh
cd build
./test/b_tree_tests
./test/disk_test
```

To run benchmarking
```sh
sh benchmark.sh
```