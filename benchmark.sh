#!/bin/bash

function clear_cache {
    sync
    echo 3 > /proc/sys/vm/drop_caches
    blockdev --flushbufs /dev/sda
    # hdparam -F /dev/sda
}

mkdir -p results

rm large_db.bartdb
./build/test/benchmark create large_db 100000
clear_cache
./build/test/benchmark read large_db 10 > results/read_0_1.txt
./build/test/benchmark read large_db 10 > results/read_0_2.txt
./build/test/benchmark read large_db 10 > results/read_0_3.txt

clear_cache
./build/test/benchmark put large_db 10 > results/put_0_1.txt
./build/test/benchmark put large_db 10 > results/put_0_2.txt
./build/test/benchmark put large_db 10 > results/put_0_3.txt




rm large_db.bartdb
./build/test/benchmark create large_db 100000
clear_cache
./build/test/benchmark read large_db 10 > results/read_1_1.txt
./build/test/benchmark read large_db 10 > results/read_1_2.txt
./build/test/benchmark read large_db 10 > results/read_1_3.txt

clear_cache
./build/test/benchmark put large_db 10 > results/put_1_1.txt
./build/test/benchmark put large_db 10 > results/put_1_2.txt
./build/test/benchmark put large_db 10 > results/put_1_3.txt




rm large_db.bartdb
./build/test/benchmark create large_db 100000
clear_cache
./build/test/benchmark read large_db 10 > results/read_2_1.txt
./build/test/benchmark read large_db 10 > results/read_2_2.txt
./build/test/benchmark read large_db 10 > results/read_2_3.txt

clear_cache
./build/test/benchmark put large_db 10 > results/put_2_1.txt
./build/test/benchmark put large_db 10 > results/put_2_2.txt
./build/test/benchmark put large_db 10 > results/put_2_3.txt