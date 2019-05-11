#!/bin/bash

touch thrash.txt
for test in tests/*.in; do
	rm ${test%.in}.myout >> thrash.txt
	rm ${test%.in}.diff >> thrash.txt
	if [[ $1 == 1 ]]; then
		./main.run < $test
	else
		./main.run < $test > ${test%.in}.myout
	fi
	sleep 0.5
	diff  ${test%.in}.out ${test%.in}.myout > ${test%.in}.diff
	if [ -s "${test%.in}.diff" ]
	then
		echo input-file : ${test##*/} :: SHIT...
	else
		echo input-file : ${test##*/} :: SUCCESS
	fi
done
rm thrash.txt
