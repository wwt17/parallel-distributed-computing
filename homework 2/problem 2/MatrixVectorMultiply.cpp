#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <mpi.h>
#define eprintf(...) fprintf(stderr, __VA_ARGS__)

/*
 *   Function 'rerror' is called when the program detects an
 *   error and wishes to print an appropriate message and exit.
 */

void rerror (char *s)
{
   printf ("%s\n", s);
   exit (-1);
}

/*
 *   Function 'allocate_matrix", passed the number of rows and columns,
 *   allocates a two-dimensional matrix of floats.
 */   

void allocate_matrix (float ***subs, int rows, int cols) {
   int    i;
   float *lptr, *rptr;
   float *storage;

   storage = (float *) malloc (rows * cols * sizeof(float));
   *subs = (float **) malloc (rows * sizeof(float *));
   for (i = 0; i < rows; i++)
      (*subs)[i] = &storage[i*cols];
   return;
}

/*
 *   Given the name of a file containing a matrix of floats, function
 *   'read_matrix' opens the file and reads its contents.
 */

void read_matrix (
   const char    *s,          /* File name */
   float ***subs,       /* 2D submatrix indices */
   int     *m,          /* Number of rows in matrix */
   int     *n)          /* Number of columns in matrix */
{
   char     error_msg[80];
   FILE    *fptr;          /* Input file pointer */

   fptr = fopen (s, "r");
   if (fptr == NULL) {
      sprintf (error_msg, "Can't open file '%s'", s);
      rerror (error_msg);
   }
   fread (m, sizeof(int), 1, fptr);
   fread (n, sizeof(int), 1, fptr);
   allocate_matrix (subs, *m, *n);
   fread ((*subs)[0], sizeof(float), *m * *n, fptr);
   fclose (fptr);
   return;
}

float** MatrixVectorMultiply(float **a, float **b, int m, int n) {
	float **c;
	allocate_matrix(&c, m, 1);
	for (int i = 0; i < m; i++) {
		float tmp = 0;
		for (int j = 0; j < n; j++)
			tmp += a[i][j] * b[j][0];
		c[i][0] = tmp;
	}
	return c;
}

int main() {
	MPI_Init(NULL, NULL);
	int world_size;
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);
	int world_rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

	float **a, **b;
	int m, n, m_, n_;
	int *displs;
	float **a_;
	double running_time = -MPI_Wtime();
	if (world_rank == 0) {
		read_matrix("matrix_a", &a, &m, &n);
		read_matrix("matrix_b", &b, &m_, &n_);
		if (m % world_size != 0) {
			eprintf("m = %d must be divisible by world_size = %d\n", m, world_size);
			MPI_Abort(MPI_COMM_WORLD, 1);
		}
		m_ = m / world_size;
	}
	MPI_Bcast(&m_, 1, MPI_INT,
			0, MPI_COMM_WORLD);
	MPI_Bcast(&n, 1, MPI_INT,
			0, MPI_COMM_WORLD);
	eprintf("process %d received m_ = %d n = %d\n", world_rank, m_, n);
	allocate_matrix(&a_, m_, n);
	eprintf("process %d scattering matrix a\n", world_rank);
	MPI_Scatter(world_rank == 0 ? a[0] : NULL, m_ * n, MPI_FLOAT,
			a_[0], m_ * n, MPI_FLOAT,
			0, MPI_COMM_WORLD);
	eprintf("process %d end scattering matrix a\n", world_rank);
	if (world_rank != 0) {
		allocate_matrix(&b, n, 1);
	}
	eprintf("process %d broadcasting vector b\n", world_rank);
	MPI_Bcast(b[0], n * 1, MPI_FLOAT,
			0, MPI_COMM_WORLD);
	eprintf("process %d end broadcasting vector b\n", world_rank);
	
	eprintf("process %d begins calculating\n", world_rank);
	float **c_ = MatrixVectorMultiply(a_, b, m_, n);
	eprintf("process %d ends calculating\n", world_rank);

	float **c;
	if (world_rank == 0) {
		allocate_matrix(&c, m, 1);
	}
	eprintf("process %d gathering c\n", world_rank);
	MPI_Gather(c_[0], m_ * 1, MPI_FLOAT,
		world_rank == 0 ? c[0] : NULL, m_ * 1, MPI_FLOAT,
		0, MPI_COMM_WORLD);
	eprintf("process %d end gathering c\n", world_rank);
	if (world_rank == 0) {
		running_time += MPI_Wtime();
		printf("running_time = %lf\n", running_time);

		FILE *file_c;
		file_c = fopen("output", "w");
		for (int i = 0; i < m; i++) {
			fprintf(file_c, "%lf\n", c[i][0]);
		}
		fclose(file_c);

		for (int i = 0; i < m; i++) {
			float correct_c = 0;
			for (int j = 0; j < n; j++)
				correct_c += a[i][j] * b[j][0];
			if (fabs(c[i][0] - correct_c) > 1e-6)
				eprintf("wrong answer: c[%d] = %lf  correct = %lf\n", i, c[i][0], correct_c);
		}
	}

	MPI_Finalize();
	return 0;
}
