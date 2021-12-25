#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

void show_matrix(double *m, int rows, int columns) {
    for (int i = 0; i < rows * columns; i++) {
        if (i != 0 && i % columns == 0)
            printf("\n");
        printf("%8.3f ", m[i]);
    }
    printf("\n-----\n");
}

double *vector_from_file(char *filename, int size) {
    FILE *f = fopen(filename, "r+");
    double *v = malloc(sizeof(double) * size);
    for (int i = 0; i < size; i++) {
        fscanf(f, "%lf", &(v[i]));
    }
    fclose(f);
    return v;
}

MPI_Request master_requests[3];
MPI_Request *workers_requests;

void send_data_to_worker(int worker, double *data, int batch_size, double *vector, int vector_size) {
    MPI_Isend(&batch_size, 1, MPI_INT, worker, 0, MPI_COMM_WORLD, &master_requests[2]);
    MPI_Isend(data, batch_size, MPI_DOUBLE, worker, 1, MPI_COMM_WORLD, &master_requests[0]);
    MPI_Isend(vector, vector_size, MPI_DOUBLE, worker, 2, MPI_COMM_WORLD, &master_requests[1]);
}

void run_master(int matrix_size, int world_size) {
    printf("run master\n");
    char matrix_path[255], vector_path[255];
    sprintf(matrix_path, "../inputs/matrix-%d", matrix_size);
    sprintf(vector_path, "../inputs/vector-%d", matrix_size);

    double *v = vector_from_file("/home/michael/CLionProjects/hpc_2/inputs/vector-3000", matrix_size);
    double *m = vector_from_file("/home/michael/CLionProjects/hpc_2/inputs/matrix-3000", matrix_size * matrix_size);


    double start = MPI_Wtime();
    int workers_count = world_size - 1;
    int lines_in_batch = matrix_size / workers_count;
    int batch_size = matrix_size * lines_in_batch;

    for (int i = 0; i < workers_count - 1; i++) {
        send_data_to_worker(i + 1, &m[i * batch_size], batch_size, v, matrix_size);
    }

    int lines_in_last_batch = lines_in_batch + matrix_size % workers_count;
    int last_batch_size = batch_size + (matrix_size * (matrix_size % workers_count));
    send_data_to_worker(workers_count, &m[(workers_count - 1) * batch_size], last_batch_size, v, matrix_size);

    double *product = malloc(sizeof(double) * matrix_size);
    for (int i = 0; i < workers_count - 1; i++) {
        MPI_Irecv(&product[i * lines_in_batch], lines_in_batch, MPI_DOUBLE, i + 1, 3, MPI_COMM_WORLD,
                  &workers_requests[i]);
    }
    MPI_Irecv(&product[(workers_count - 1) * lines_in_batch], lines_in_last_batch, MPI_DOUBLE, workers_count, 3,
              MPI_COMM_WORLD, &workers_requests[workers_count - 1]);

    MPI_Waitall(workers_count, workers_requests, MPI_STATUSES_IGNORE);
    double end = MPI_Wtime();

    printf("ANSWER:\n");
    show_matrix(product, matrix_size, 1);
    printf("TOTAL TIME: %.5lf\n", MPI_Wtime() - start);

    free(m);
    free(v);
    free(product);
}

double *matrix_on_vector(const double *m, int rows, int columns, const double *v) {
    double *product = malloc(sizeof(double) * rows);
    for (int i = 0; i < rows; i++) {
        product[i] = 0;
    }

    for (int i = 0; i < rows; i++) {
        int row = i * columns;
        for (int j = 0; j < columns; j++) {
            product[i] += v[j] * m[row + j];
        }
    }

    return product;
}

void run_worker(int rank, int matrix_size) {
    printf("run worker %d\n", rank);

    int batch_size;
    MPI_Recv(&batch_size, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    double *m = malloc(sizeof(double) * batch_size);
    MPI_Irecv(m, batch_size, MPI_DOUBLE, MPI_ANY_SOURCE, 1, MPI_COMM_WORLD, &master_requests[0]);

    double *v = malloc(sizeof(double) * matrix_size);
    MPI_Irecv(v, matrix_size, MPI_DOUBLE, MPI_ANY_SOURCE, 2, MPI_COMM_WORLD, &master_requests[1]);

    MPI_Waitall(2, master_requests, MPI_STATUSES_IGNORE);

    int rows = batch_size / matrix_size;
    double *product = matrix_on_vector(m, rows, matrix_size, v);

    MPI_Send(product, rows, MPI_DOUBLE, 0, 3, MPI_COMM_WORLD);

    free(m);
    free(v);
    free(product);
}

int main(int argc, char **argv) {
    MPI_Init(NULL, NULL);

    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    workers_requests = malloc(sizeof(MPI_Request) * (world_size - 1));

    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    char *end;
    int matrix_size = 3000;//(int) strtol(argv[1], &end, 10);


    if (rank == 0) {
        run_master(matrix_size, world_size);
    } else {
        run_worker(rank, matrix_size);
    }

    MPI_Finalize();
    free(workers_requests);
}
