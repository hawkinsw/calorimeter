#!/bin/sh
#

input_file=$1
shift
output_file=$1

hexdump=`which hexdump`

$hexdump -C $input_file > $output_file
