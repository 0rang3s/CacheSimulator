#!/bin/bash

# USAGE: 
# To run all the tests 
#   ./runtest.sh 
# To run a single test, e.g., 03
#   ./runtest.sh 03

TESTS="00 01 02 03 04"
EXE=cachex

if [ -x $EXE ]; then
	EXECDIR=.
elif [ -x  cmake-build-debug/$EXE ]; then
	EXECDIR=cmake-build-debug
else
	echo Cannot find $EXE
	exit
fi

if [ $1"X" == "X" ]; then
	for i in $TESTS; do
		./tests/bench.sh $i $EXECDIR $EXE
	done
else
	./tests/bench.sh $1 $EXECDIR $EXE
fi
