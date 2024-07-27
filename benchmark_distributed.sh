#!/bin/bash
# Run as sudo bash benchmark_distribution_read.sh

function benchmark_distrubtion {
    rm server*.bartdb

    killall client
    killall server

    # Spin up $1 servers
    servers=""
    for i in $(seq 0 $(($1-1)));
    do
        ./build/src/server/server server${i} --port 400${i} &
        servers="${servers},127.0.0.1:400${i}"
    done

    sleep 2

    ./build/src/client/client STRESS_WRITE 10000 --hosts ${servers} -rp $2 > "results/distributed_stress_write_$1_$2.txt"

    servers=${servers:1}
    echo $servers

    sleep 1

    ./build/src/client/client STRESS_READ 0 --hosts ${servers} -rp $2 > "results/distributed_stress_read_$1_$2.txt" &

    sleep 20

    killall client
    killall server

    sleep 2
}

rm server*.bartdb

benchmark_distrubtion 1 1
benchmark_distrubtion 2 1
benchmark_distrubtion 3 1
benchmark_distrubtion 4 1
benchmark_distrubtion 5 1
benchmark_distrubtion 2 2
benchmark_distrubtion 3 2
benchmark_distrubtion 4 2
benchmark_distrubtion 5 2
benchmark_distrubtion 3 3
benchmark_distrubtion 4 3
benchmark_distrubtion 5 3
benchmark_distrubtion 4 4
benchmark_distrubtion 5 4
benchmark_distrubtion 5 5
