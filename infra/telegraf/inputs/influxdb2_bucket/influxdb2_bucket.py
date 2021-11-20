"""
Measure the disk usage for each influxdb2 bucket, and write line protocol input
Measurement:
    bucketsize
Tags:
    dir (dir under db dir, typically 'data' and 'wal')
    bucket (bucket hex identifier)
Fields: 
    size_kB: bucket size in kB, per 'du -sk'
"""

import os
import subprocess
import sys
import time

SUBDIRS = ['data', 'wal']


def dirsize(dir):
    return int(subprocess.check_output(['du','-sk', dir]).split()[0].decode('utf-8'))

def main():
    db_dir = sys.argv[1].rstrip('/')
    lp = {}
    time_ns = time.time_ns()
    for subdir in SUBDIRS:
        bucket_dir = db_dir + '/' + subdir
        if os.path.isdir(bucket_dir):
            for bucket in os.listdir(bucket_dir):
                size = dirsize(bucket_dir + '/' + bucket)
                print("%s: %s" % (bucket, size))

if __name__== "__main__":
    main()
