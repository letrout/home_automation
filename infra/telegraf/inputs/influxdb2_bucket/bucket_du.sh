#!/bin/bash
subdir=$1
bucket=$2
BUCKET_DU=/influxdb2/bucket_du.txt
MEASUREMENT="bucket"
#size_k=($( grep ${subdir} ${BUCKET_DU} |grep ${bucket} |cut -d ' ' -f 1 ))
size_k=($( grep ${subdir}/${bucket} ${BUCKET_DU} |cut -d ' ' -f 1 ))
time=$( find $BUCKET_DU -printf '%p %Cs\n' |cut -d ' ' -f 2 )
time_ns="$time""000000000"
echo "${MEASUREMENT},bucket=${bucket},subdir=${subdir} size_k=${size_k} ${time_ns}"
