#!/bin/bash
if [ $# != 3 ]
then
	echo "usage: <m> <n> <world_size>"
	exit
fi
/usr/local/bin/mpicxx MatrixVectorMultiply.cpp -o MatrixVectorMultiply
./gen matrix_a $1 $2
./gen matrix_b $2 1
/usr/local/bin/mpirun -n $3 ./MatrixVectorMultiply
