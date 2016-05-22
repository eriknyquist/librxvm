#!/bin/bash

if [ $# -ne 1 ]
then
    printf 'Usage: %s <playback archive>\n' "$0"
    exit 1
fi

arr_up='A'
arr_down='B'
arr_left='D'
arr_right='C'
clr="\033c"
archive=$(basename $1)
name="${archive%.*}"

trap cleanup INT

print_keys() {
    printf "Navigation keys\n\n"
    printf "  a: previous frame\n"
    printf "  d: next frame\n"
    printf "  s: first frame\n"
    printf "  w: last frame\n"
    printf "  q: quit\n\n"
    printf "Any other key to show this screen\n"
}

cleanup() {
    if [ -n "$name" ]
    then
        rm -rf ".$name"
    fi
    stty sane
    exit 0
}

tar -xf "$1" || exit 1

regex=$(cat ".$name/.rgx")
num=$(ls ".$name" | wc -l)
i=1

printf "$clr"
print_keys

while true
do
    input=""

    IFS="" read -srn1 input
    if [ "$input" == "d" ] || [ "$input" == "$arr_right" ]
    then
        if [ $i -lt $num ]
        then
            i=$((i + 1))
        fi
    elif [ "$input" == "q" ]
    then
        break
    elif [ "$input" == "a" ] || [ "$input" == "$arr_left" ]
    then
        if [ $i -gt 1 ]
        then
            i=$((i - 1))
        fi
    elif [ "$input" == 's' ] || [ "$input" == "$arr_down" ]
    then
        i=1
    elif [ "$input" == 'w' ] || [ "$input" == "$arr_up" ]
    then
        i="$num"
    else
        printf "$clr"
        echo "$input"
        print_keys
        continue
    fi

    printf "$clr"
    printf "$regex  $i / $num\n\n"
    cat ".$name/$i.dbgf"
done

cleanup
