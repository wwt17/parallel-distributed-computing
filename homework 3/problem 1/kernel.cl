__kernel void matrixVectorMul(__global float *A, __global float *b, __global float *c, const int n) {
	int i = get_global_id(0), j;
	float sum = 0;
	for (j = 0; j < n; j++) {
		sum += A[i * n + j] * b[j];
	}
	c[i] = sum;
}
