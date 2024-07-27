#!/bin/bash
# Run as sudo bash benchmark_distribution_read.sh

function create_db {

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

    ./build/src/client/client STRESS_WRITE 10000 --hosts $servers -rp $1

    killall client
    killall server
}

function benchmark_distrubtion {

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

    # Spin up $2 clients
    for i in $(seq 0 $(($2-1)));
    do
        if i=0; then
            ./build/src/client/client STRESS_READ 0 --hosts ${servers} -rp $1 > "results/distributed_stress_server_client_$1_$2.txt" &
        else
            ./build/src/client/client STRESS_READ 0 --hosts ${servers} -rp $1 &
        fi
    done

    sleep 20

    killall client
    killall server

    sleep 2
}

./build/src/server/server server${i} --port 4001 &
./build/src/client/client STRESS_WRITE 10000 --hosts 127.0.0.1:4001 -rp 1

create_db 1
benchmark_distrubtion 1 100
benchmark_distrubtion 1 200
benchmark_distrubtion 1 300
benchmark_distrubtion 1 400
benchmark_distrubtion 1 500

create_db 2
benchmark_distrubtion 2 100
benchmark_distrubtion 2 200
benchmark_distrubtion 2 300
benchmark_distrubtion 2 400
benchmark_distrubtion 2 500

create_db 3
benchmark_distrubtion 3 100
benchmark_distrubtion 3 200
benchmark_distrubtion 3 300
benchmark_distrubtion 3 400
benchmark_distrubtion 3 500

create_db 4
benchmark_distrubtion 4 100
benchmark_distrubtion 4 200
benchmark_distrubtion 4 300
benchmark_distrubtion 4 400
benchmark_distrubtion 4 500

create_db 5
benchmark_distrubtion 5 100
benchmark_distrubtion 5 200
benchmark_distrubtion 5 300
benchmark_distrubtion 5 400
benchmark_distrubtion 5 500
