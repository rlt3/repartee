#!/usr/bin/bash

set -e

declare -a tests=(
    "1"
    "8 - 10 + 4"
    "3 * 14 / 2 - 18"
    "                    4                             "
    "1 + 1 + 1 + 1 + 5 / 5"
)

len=${#tests[@]}

for ((i=1; i < ${len} + 1; i++)); do
    in=${tests[$i - 1]}
    echo "$in"
    out=`echo "$in" | ./lang -`
done

echo "All Tests Passed!"
