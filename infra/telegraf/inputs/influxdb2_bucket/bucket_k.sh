#!/bin/bash
# Measure the disk usage for each influxdb2 bucket, and write line protocol input
# For use as a telegraf exec input
# Measurement:
#    bucket
# Tags:
#    dir (dir under db dir, typically 'data' and 'wal')
#    bucket (bucket hex identifier)
# Fields: 
#    size_kB: bucket size in kB, per 'du -sk'

is_subdir() {
	case $1 in
      ( "data" | "wal" ) echo 1 ;;
      ( * ) echo 0 ;;
    esac 
}

is_bucket() {
    case $1 in
      ( *[!0-9A-Fa-f]* | "" ) echo 1 ;;
      ( * )                
        case ${#1} in
          ( 32 | 40 ) echo 0 ;;
          ( * )       echo 1 ;;
        esac
    esac    
}

if [ ! -d $1 ]; then
	exit 1
fi

MEASUREMENT="bucket"
size=$(du -sk $1 |cut -f 1)
bucket=$(basename $1)
subdir=$(basename $(dirname $1))
if [ $( is_subdir $subdir ) == 1 -a $( is_bucket $bucket ) ]; then
	echo "${MEASUREMENT},bucket=${bucket},subdir=${subdir} size_k=${size}"
fi