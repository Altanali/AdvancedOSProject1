#!/usr/bin/env bash

export TIMEFORMAT=%R
num_runs=10
program="time ./mmap_exec"
program_data='../data.txt'
output_file="results.txt"


> "$output_file"

for i in {0..1}; do

	for j in {0..1}; do
		flags="-f $program_data"
		experiment_type=""
		#File private/shared flag
		if [[ $i -eq 0 ]]; then
			experiment_type+="Private "
		else
			flags+=" -s"
			experiment_type+="Shared "
		fi
		
		#File-backed/anonymous flag
		if [[ $j -eq 0 ]]; then
			experiment_type+="File-Backed"
		else
			flags+=" -a"
			experiment_type+="Anonymous"
		fi

		echo "$experiment_type" | tee -a "$output_file"
		for ((k=0; k<num_runs; k++)); do
			eval "$program $flags" 2>> "$output_file"
		done
	done
done
