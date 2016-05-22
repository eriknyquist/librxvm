#!/bin/bash

if [ $# -ne 1 ]
then
    printf 'Usage: %s <playback_archive>\n' "$0"
    exit 1
fi

delay=0.05
clr="\033c"
archive=$(basename $1)
name="${archive%.*}"

trap cleanup INT

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

printf "$clr"
print_keys

while true
do
    i=1
    for f in $(ls .$name/* | sort -V)
    do
        printf "$clr"
        printf "$regex  $i / $num\n\n"
        i=$((i + 1))
        cat "$f"
        sleep "$delay"
    done
done

cleanup
