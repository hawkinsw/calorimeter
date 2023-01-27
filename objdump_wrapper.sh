#!/bin/sh
#

input_file=$1
shift
output_file=$1

objdump=`which objdump`

echo "input file: " $input_file
echo "output file: " $output_file

$objdump -D $input_file > $output_file
