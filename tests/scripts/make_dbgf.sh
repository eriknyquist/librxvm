#!/bin/bash

if [ $# -ne 2 ] && [ $# -ne 3 ]
then
    printf 'Usage: %s <regex> <input> [<archive_name>]\n' "$0"
fi

outfile=""
regex="$1"
input="$2"
stamp=$(date +'%d-%m-%Y_%H%M%S')

if [ $# -eq 3 ]
then
    name="${3%.*}"
else
    name="rvm_dbgf_$stamp"
fi

make clean all CFLAGS=-DDBGF
make clean all -C examples
[ -d ".rvm_dbgf" ] || mkdir ".rvm_dbgf"

examples/regexvm_match "$regex" "$input"
mv ".rvm_dbgf" ".$name"
printf "$regex" > ".$name/.rgx"

tar -pczf "$name.tgz" ".$name"

make clean
make clean -C examples

if [ -n "$name" ]
then
    rm -rf ".$name"
fi

printf 'Playback file %s.tgz created.\n' "$name"
