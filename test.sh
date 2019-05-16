#!/usr/bin/bash

# Each expression produce an number equal to its position in the array + 1
declare -a tests=(
    "1;"
    "8 - 10 + 4;"
    "3 * 14 / 2 - 18;"
    "                    4                             ;"
    "1 + 1 + 1 + 1 + 5 / 5;"
    "18 / (6 / (4 - 2));"
    "(7);"
    "a = 100 - 36; b = a / 8; b;"
)

len=${#tests[@]}

for ((i=1; i < ${len} + 1; i++)); do
    in=${tests[$i - 1]}
    out=`echo "$in" | ./lang -`
    if [[ $i != $out ]]; then
        echo "Test \`$in\` failed: $i != $out"
        exit 1
    fi
done

echo "All Tests Passed!"
