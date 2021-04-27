#/bin/bash

set -e

for i in $(seq 1 $1); do
    printf "\0\0\0\5\000ls ." | nc 127.0.0.1 8000 > /dev/null &
    echo "Succ create session $i"
done

echo "Finish connect $1 sessions"

# relate to env
pgid=$(ps ax -o '%p %r' | awk -vp=$$ '$1==p{print $2}')
cnt=$(ps -j | grep "$pgid" | grep nc | wc -l)
echo "Still holds $cnt sessions"

kill 0
