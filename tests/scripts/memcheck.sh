#!/bin/sh

VBIN=valgrind

vloc="$(type -p $VBIN)"

if [ -z "$vloc" ]
then
    printf "\n$VBIN not installed. You must install $VBIN to use this script\n"
fi

if [ $# -lt 1 ]
then
    printf "\nusage: $0 <testapp> [<args>]\n"
    exit 1
fi

$VBIN -v --leak-check=full --show-leak-kinds=all --track-origins=yes \
--leak-check-heuristics=all ./"$@"
