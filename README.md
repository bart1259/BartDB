# BartDB
A simple on disk database implementation. The implementation includes an on disk BTree, a database server to retrieve data from the BTree, a distributed database client which can store data across multiple nodes in a cluster and interactively query the database server, benchmarking code to test the disk, network, and distributed cluster performance of BartDB, and tests to verify the implementation. 

BTree library source code is found under `src/btreelib`. Much of the source depends on templates so the implementation is mostly in header files.

Database server and client code is found under `src/server` and `src/client` respectively.

All test and benchmark code is found under `test`

This project relies on `g++` `make` and `cmake`

## Compilation
To compile:
```sh
mkdir build
cd build
cmake ..
make
```

## Testing
To run unit/integration tests
```sh
cd build
./test/b_tree_tests
./test/disk_test
./test/packet_test
```

## Benchmarking

### Disk Benchmarking
To run benchmarking
```sh
sudo bash benchmark_disk.sh
```
To compile benchmarking results run `test/disk_analysis.ipynb`

### Network Benchmarking
To run benchmarking
```sh
sudo bash benchmark_network.sh
```
To compile benchmarking results run `test/network_analysis.ipynb`

## Running Database Server

Running the database server
```sh
./src/server/server testdb --port 6122
```

## Running Database Client

Running the database client
```sh
./src/client/client --hosts 127.0.0.1:6122
```

Using the database client
```sh
> PUT HELLO WORLD
OK
> CONTAINS HELLO
TRUE
> CONTAINS YELLOW
FALSE
> GET HELLO
OK
WORLD
> EXIT
Exiting...
```
To stress test the server, the client can be run in stress mode

```sh
./src/client/client STRESS_READ 0 --hosts 127.0.0.1:6122
```