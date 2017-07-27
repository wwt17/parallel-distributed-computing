m=10000
n=10000

result: a matrix_A matrix_b
	./a >result

a: a.cpp
	g++ a.cpp -o a -lcl -Wall

matrix_A: gen
	./gen matrix_A $m $n

matrix_b: gen
	./gen matrix_b $n 1

clean:
	rm -rf a matrix_A matrix_b result
