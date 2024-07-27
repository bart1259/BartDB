#!/bin/bash
# Run as sudo bash benchmark_network.sh

function run_network_test {
    echo "Running test with $1 clients"
    ./build/src/server/server large_db -p 6122 &

    sleep 1s

    for i in $(seq 0 $(($1-1)));
    do
        ./build/src/client/client STRESS_READ -h 127.0.0.1:6122 > "results/network_stress_${i}_${1}.txt" &
    done

    sleep 20s

    killall client
    killall server

    sleep 1s
}

# mkdir -p results
# rm large_db.bartdb
# ./build/test/benchmark create large_db 100000

run_network_test 5
run_network_test 10
run_network_test 20
run_network_test 50
run_network_test 100