#!/bin/bash
# Run as sudo bash benchmark.sh

function clear_cache {
    sync
    echo 3 > /proc/sys/vm/drop_caches
    blockdev --flushbufs /dev/sda
    # hdparam -F /dev/sda
}

mkdir -p results
rm large_db.bartdb
./build/test/benchmark create large_db 100000

for i in $(seq 0 9);
do
    clear_cache
    ./build/test/benchmark read large_db 60 > "results/read_${i}_1.txt"

    clear_cache
    ./build/test/benchmark put large_db 60 > "results/put_${i}_1.txt"
done