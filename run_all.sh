#!/bin/bash

for test in tests/*.in; do
	echo
	echo --- input-file : ${test##*/} ---
	if [[ $1 == 1 ]]; then
		./main.run < $test
	else
		./main.run < $test > ${test%.in}.myout
	fi
	sleep 0.5
done
