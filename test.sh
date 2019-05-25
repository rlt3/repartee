#!/bin/bash

# Each expression produce an number equal to its position in the array + 1
declare -a tests=(
    "         1 ;           "
    "8 - 10 + 4;"
    "int a = 14; int b = 18; 3 * a / 2 - b;"
    "int a = 4 - (4 * 2); 0 - a;"
)

#declare -a tests=(
#    "a = 10 / 2; a == 5;"
#    "8 - 10 + 4;"
#    "3 * 14 / 2 - 18;"
#    "                    4                             ;"
#    "1 + 1 + 1 + 1 + 5 / 5;"
#    "18 / (6 / (4 - 2));"
#    "(7);"
#    "a = 100 - 36; b = a / 8; b;"
#    "a = (1 == 1); b = 4; ((a + b) * 2) - 1;"
#    "a = 4 + (10 / (2 + 8)) == 5 * (4 + 5) / 9; a * 10;"
#    "if (1) { foo = 1; 11; } else { bar = 2; 12; }"
#    "if (0) { foo = 1; 11; } else { bar = 2; 12; }"
#    "if (1) { if (1) { 13; } else { 14; } }"
#    "if (1) { if (0) { 13; } else { 14; } }"
#    "if (1 or 0) { 15; } else { 16; }"
#    "if (1 and 0) { 15; } else { 16; }"
#    "a=1;b=1;c=0; if (a or (b and c)) { 17; } else { 18; }"
#    "a=0;b=1;c=0; if (a or (b and c)) { 17; } else { 18; }"
#    "a=1;b=1;c=0; if (a and (b or c)) { 19; } else { 20; }"
#    "a=0;b=1;c=0; if (a and (b or c)) { 19; } else { 20; }"
#    "a=1;b=1;c=0;d=1; if (a and (b or (c and d))) { 21; } else { 22; }"
#    "a=1;b=0;c=1;d=0; if (a and (b or (c and d))) { 21; } else { 22; }"
#    "a=0;b=5;c=a or b; if (c) { 23; } else { 0; }"
#)

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
