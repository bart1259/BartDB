#!/bin/bash
# Run as sudo bash benchmark.sh

# function clear_cache {
#     sync
#     echo 3 > /proc/sys/vm/drop_caches
#     blockdev --flushbufs /dev/sda
#     # hdparam -F /dev/sda
# }

# mkdir -p results
# rm large_db.bartdb
# ./build/test/benchmark create large_db 100000

./build/src/server/server large_db -p 6120 &

sleep 1s

for i in $(seq 0 49);
do
    ./build/src/client/client STRESS -p 6120 -h 127.0.0.1 > "results/network_stress_50_${i}_1.txt" &
done

sleep 20s

killall client
killall server