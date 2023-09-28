#!/usr/bin/env bash

cd /usr/home
export TIMEFORMAT=%R

time ./direct_io -f file-1g >> results_docker_overlay.txt
