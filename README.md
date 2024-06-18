# BartDB
A basic on disk B-Tree implementation for class

BTree library source code is found under `src/btreelib`. Much of the source depends on templates so the implementation is mostly in header files.

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
bash benchmark.sh
```

To compile benchmarking results run `test/analysis.ipynb`