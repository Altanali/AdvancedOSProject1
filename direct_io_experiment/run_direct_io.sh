#!/usr/bin/env bash

export TIMEFORMAT=%R
num_runs=10
program="time ./direct_io"
program_data='../data.txt'
output_file="results.txt"


> "$output_file"

for i in {0..1}; do

	for j in {0..1}; do
		flags="-f $program_data"
		experiment_type=""
		#File private/shared flag
		if [[ $i -eq 0 ]]; then
			experiment_type+="Random "
		else
			flags+=" -s"
			experiment_type+="Sequential "
		fi
		
		#File-backed/anonymous flag
		if [[ $j -eq 0 ]]; then
			experiment_type+="Reading"
		else
			flags+=" -w"
			experiment_type+="Writing"
		fi

		printf "$experiment_type\n" | tee -a "$output_file"
		for ((k=0; k<num_runs; k++)); do
			eval "$program $flags" 2>> "$output_file"
		done
		printf "\n"
	done
done
