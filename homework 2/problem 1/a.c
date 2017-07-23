#include <mpi.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>
#define eprintf(...) fprintf(stderr, __VA_ARGS__)

#define MAX_LENGTH 1000000

char mem[MAX_LENGTH];

int main(int argc, char **argv) {
	if (argc == 1) {
		eprintf("usage: %s <Length> [<cnt>]\n", argv[0]);
		return 0;
	}
	int Length;
	sscanf(argv[1], "%d", &Length);
	int cnt;
	if (argc == 2)
		cnt = 1e8 / Length;
	else
		sscanf(argv[2], "%d", &cnt);

	MPI_Init(NULL, NULL);
	int world_rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
	int world_size;
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);
	if (world_size != 2) {
		eprintf("World size must be two for %s\n", argv[0]);
		MPI_Abort(MPI_COMM_WORLD, 1);
	}

	FILE *f;
	f = fopen(world_rank == 0 ? "tv0" : "tv1", "w");
	if (f == NULL) {
		eprintf("open file %s failed\n", world_rank == 0 ? "tv0" : "tv1");
		MPI_Abort(MPI_COMM_WORLD, 1);
	}

	struct timeval tv;
	int i;
	for (i = 0; i < cnt; i++) {
		if (world_rank == 0) {
			if (gettimeofday(&tv, NULL) == -1) {
				eprintf("process %d error: gettimeofday returns errno = %d\n", world_rank, errno);
				MPI_Abort(MPI_COMM_WORLD, 1);
			}
			fwrite((void*)&tv, sizeof(tv), 1, f);
			MPI_Send((void*)mem, Length, MPI_BYTE,
					1, 0, MPI_COMM_WORLD);
		}
		else {
			MPI_Recv((void*)mem, Length, MPI_BYTE,
					0, 0, MPI_COMM_WORLD,
					MPI_STATUS_IGNORE);
			if (gettimeofday(&tv, NULL) == -1) {
				eprintf("process %d error: gettimeofday returns errno = %d\n", world_rank, errno);
				MPI_Abort(MPI_COMM_WORLD, 1);
			}
			fwrite((void*)&tv, sizeof(tv), 1, f);
		}
	}
	fclose(f);

	MPI_Finalize();

	if (world_rank == 0) {
		double total = 0;
		FILE *f0 = fopen("tv0", "r"), *f1 = fopen("tv1", "r");
		for (int i = 0; i < cnt; i++) {
			struct timeval tv0, tv1, dv;
			fread((void*)&tv0, sizeof(tv0), 1, f0);
			fread((void*)&tv1, sizeof(tv1), 1, f1);
			timersub(&tv1, &tv0, &dv);
			total += dv.tv_sec * 1e6 + dv.tv_usec;
		}
		printf("Length: %d  Comm time: %.6lf us  Comm Bandwidth: %.6lf Mb/s\n",
			Length, total / cnt, Length / (total / cnt) * 8);
	}
}
