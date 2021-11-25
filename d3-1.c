#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define BLOCK_LOW(id, p, n) ((id) * (n) / (p))
#define BLOCK_HIGH(id, p, n) (BLOCK_LOW((id) + 1, p, n) - 1)
#define BLOCK_SIZE(id, p, n) (BLOCK_LOW((id) + 1, p, n) - BLOCK_LOW(id, p, n))
#define BLOCK_OWNER(index, p, n) (((p)*(index)+1)-1)/(n))

int printArray(int t[], int len)
{
    for (int i = 0; i < len; i++)
    {
        printf("%3i ", t[i]);
    }
    printf("\n");
    return 0;
}

int main(int argc, char *argv[])
{
    int id, p, n, low_value, high_value, size, proc0_size, index, prime, first, count, global_count;
    char *marked;
    double elapsed_time;

    MPI_Init(&argc, &argv);
    MPI_Barrier(MPI_COMM_WORLD);
    elapsed_time = -MPI_Wtime();
    MPI_Comm_rank(MPI_COMM_WORLD, &id);
    MPI_Comm_size(MPI_COMM_WORLD, &p);

    if (argc != 2)
    {
        if (!id)
            printf("Command line: %s <m>\n", argv[0]);
        MPI_Finalize();
        exit(1);
    }
    n = atoi(argv[1]);
    low_value = 2 + BLOCK_LOW(id, p, n - 1);
    high_value = MIN(2 + BLOCK_HIGH(id, p, n - 1) + 2, n);
    size = BLOCK_SIZE(id, p, n - 1);

    //printf("id: %2i low: %5i high: %5i size: %5i\n", id, low_value, high_value, size);

    proc0_size = (n - 1) / p;
    if ((2 + proc0_size) < (int)sqrt((double)n))
    {
        if (!id)
            printf("Too many processes\n");
        MPI_Finalize();
        exit(1);
    }

    marked = (char *)malloc(size);
    if (marked == NULL)
    {
        printf("Cannot allocate enough memory\n");
        MPI_Finalize();
        exit(1);
    }

    for (int i = 0; i < size; i++)
        marked[i] = 0;
    if (!id)
        index = 0;
    prime = 2;
    do
    {
        if (prime * prime > low_value)
            first = prime * prime - low_value;
        else
        {
            if (!(low_value % prime))
                first = 0;
            else
                first = prime - (low_value % prime);
        }
        for (int i = first; i < size; i += prime)
            marked[i] = 1;
        if (!id)
        {
            while (marked[++index])
                ;
            prime = index + 2;
        }
        MPI_Bcast(&prime, 1, MPI_INT, 0, MPI_COMM_WORLD);
    } while (prime * prime <= n);
    
    for (int i = 0; i < size; i++)
    {
        if (marked[i] == 0 && marked[i + 2] == 0)
        {
            printf("(%3i, %3i) ", low_value + i, low_value + i + 2);
        }
    }

    elapsed_time += MPI_Wtime();
    MPI_Barrier(MPI_COMM_WORLD);
    if (!id)
    {
        printf("\nTotal elapsed time: %10.6f\n", elapsed_time);
    }
    MPI_Finalize();
    return 0;
}
