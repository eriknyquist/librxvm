#!/bin/bash

if [ $# -ne 2 ]
then
    echo "Usage: $0 <rxvm_fsearch executable> <output.pdf>"
    exit 1
fi

size=0
sizes="1024000 1024000 8192000 40960000 51200000 153600000 256000000 256000000 256000000 256000000"
test_iter=10
#pattern="a+b+c+d+x+y+d+f+"
pattern="erikkk"
filename=.__chars_per_sec_testfile.txt
plotfile=.__rxvm_fsearch.gnuplot
psfile=.__rxvm_fsearch_speedplot.ps
plotscript=tests/scripts/speed.plot

commit=$(git show | head -1)

get_ms() {
    date +%s%N | cut -b1-13
}

gen_file() {
    cat /dev/urandom | base64 | head -c"$2" >> "$1"
    ls -l "$1" | awk '{print $5}'
}

do_test() {
    exp="$1"
    fname="$2"
    fsize="$3"
    fsearch="$4"

    i=0
    running=0
    running_msecs=0

    echo "Using expression '$exp'"
    while [ $i -lt $test_iter ]
    do
        begin=$(get_ms)
        "$fsearch" "$exp" "$fname"
        end=$(get_ms)

        msecs=$((end - begin))
        secs=$(echo "scale=10;$msecs/1000" | bc -l)
        raw=$(echo "scale=10;$fsize/$secs" | bc -l)
        chars_per_sec=$(echo "$raw" | awk -F'.' '{print $1}')
        printf "iteration %s: %.2f secs, %'.f characters per second\n" $((i + 1)) $secs $chars_per_sec


        running_msecs=$((running_msecs + msecs))
        running=$((running + chars_per_sec))
        i=$((i + 1))
    done

    msecs=$(echo "scale=10;$running_msecs/$test_iter" | bc -l | awk -F'.' '{print $1}')
    echo "$fsize $msecs" >> "$plotfile"

    avg=$(echo "scale=10;$running/$test_iter" | bc -l | awk -F'.' '{print $1}')
    printf "Average: %'.f characters per second\n\n" "$avg"
}

[ -n "$commit" ] && echo "$commit"

for S in $sizes
do
    size=$((size + S))
    echo -e "\nGenerating test file (using size of $size bytes)..."
    fsize=$(gen_file "$filename" "$S")
    hsize=$(ls -lh "$filename" | awk '{print $5}')
    echo "Testing executable $1, using a $hsize plain text file"
    echo -e "Results averaged over $test_iter iterations\n"

    do_test "$pattern" "$filename" "$fsize" "$1"
done

gnuplot "$plotscript"
ps2pdf "$psfile" "$2"

rm -f "$plotfile" "$psfile" "$filename"
