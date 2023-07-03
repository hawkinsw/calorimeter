#!/bin/env bash

objdump=/usr/bin/objdump

infile=$1
shift
outfile=$1
shift

symbols=`objdump -t ${infile} | grep text | grep -e '[0-9A-Za-z]\{16\} g' | awk '{ print $NF}'`

# Erase what is already in the symbols.s file.
echo '' > ${outfile}

# Write the .extern-s for each symbol.
for i in ${symbols}; do
    echo ".extern" $i
done >> ${outfile}

cat <<END >> ${outfile}
    .data
    .globl __patch_eligible
__patch_eligible:
END

# Write the .ascii/.quad-s for each symbol.
for i in ${symbols}; do
    echo -e "\t.ascii \"$i\\\0\""
    echo -e "\t.quad ${i}"
done >> ${outfile}

cat <<END >> ${outfile}
    .byte 0xff
END