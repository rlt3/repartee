#!/usr/bin/bash

set -e

declare -a tests=(
    "1"
    "-8 - 10 + 4"
    "3 * 14 / 2 - 18"
    "                    4                             "
    "1 + 1 + 1 + 1 + 5 / 5"
)

len=${#tests[@]}

for ((i=1; i < ${len} + 1; i++)); do
    in=${tests[$i - 1]}
    out=`echo "$in" | ./lang -`
    if [ "$out" -ne "$i" ]; then
        echo "Test '$in' did not return $i"
        exit 1
    fi
done

echo "All Tests Passed!"
