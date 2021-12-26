#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "mpi.h"

#define DEFAULT_ITERATIONS 64
#define GRID_WIDTH 256
#define DIM 16 

int mod(int a, int b)
{
    int r = a % b;
    return r < 0 ? r + b : r;
}

int main(int argc, char **argv)
{

    int global_grid[256] =
        {0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
         0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
         1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
         0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
         0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
         0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
         0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
         0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
         0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
         0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
         0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
         0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
         0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
         0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
         0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
         0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    // MPI Standard variable
    int num_procs;
    int ID, j;
    int iters = 0;
    int num_iterations;

    // Setup number of iterations
    if (argc == 1)
    {
        num_iterations = DEFAULT_ITERATIONS;
    }
    else if (argc == 2)
    {
        num_iterations = atoi(argv[1]);
    }
    else
    {
        printf("Usage: ./gameoflife <num_iterations>\n");
        exit(1);
    }

    MPI_Status stat;

    if (MPI_Init(&argc, &argv) != MPI_SUCCESS)
    {
        printf("MPI_Init error\n");
    }

    MPI_Comm_size(MPI_COMM_WORLD, &num_procs); 
    MPI_Comm_rank(MPI_COMM_WORLD, &ID);

    assert(DIM % num_procs == 0);
  
    int *arr = (int *)malloc(DIM * ((DIM / num_procs) + 2) * sizeof(int));
    for (iters = 0; iters < num_iterations; iters++)
    {
        j = DIM;
        for (int i = ID * (GRID_WIDTH / num_procs); i < (ID + 1) * (GRID_WIDTH / num_procs); i++)
        {
            arr[j] = global_grid[i];
            j++;
        }

        if (num_procs != 1)
        {
            int incoming_1[DIM];
            int incoming_2[DIM];
            int send_1[DIM];
            int send_2[DIM];
            if (ID % 2 == 0)
            {

                for (int i = 0; i < DIM; i++)
                {
                    send_1[i] = arr[i + DIM];
                }
                MPI_Ssend(&send_1, DIM, MPI_INT, mod(ID - 1, num_procs), 1, MPI_COMM_WORLD);

                for (int i = 0; i < DIM; i++)
                {
                    send_2[i] = arr[(DIM * (DIM / num_procs)) + i];
                }
                MPI_Ssend(&send_2, DIM, MPI_INT, mod(ID + 1, num_procs), 1, MPI_COMM_WORLD);
            }
            else
            {
                MPI_Recv(&incoming_2, DIM, MPI_INT, mod(ID + 1, num_procs), 1, MPI_COMM_WORLD, &stat);
                MPI_Recv(&incoming_1, DIM, MPI_INT, mod(ID - 1, num_procs), 1, MPI_COMM_WORLD, &stat);
            }
            if (ID % 2 == 0)
            {
                MPI_Recv(&incoming_2, DIM, MPI_INT, mod(ID + 1, num_procs), 1, MPI_COMM_WORLD, &stat);
                MPI_Recv(&incoming_1, DIM, MPI_INT, mod(ID - 1, num_procs), 1, MPI_COMM_WORLD, &stat);
            }
            else
            {
                for (int i = 0; i < DIM; i++)
                {
                    send_1[i] = arr[i + DIM];
                }
                MPI_Ssend(&send_1, DIM, MPI_INT, mod(ID - 1, num_procs), 1, MPI_COMM_WORLD);

                for (int i = 0; i < DIM; i++)
                {
                    send_2[i] = arr[(DIM * (DIM / num_procs)) + i];
                }
                MPI_Ssend(&send_2, DIM, MPI_INT, mod(ID + 1, num_procs), 1, MPI_COMM_WORLD);
            }
            for (int i = 0; i < DIM; i++)
            {
                arr[i] = incoming_1[i];
                arr[(DIM * ((DIM / num_procs) + 1)) + i] = incoming_2[i];
            }
        }
        else
        {
            for (int i = 0; i < DIM; i++)
            {
                arr[i + GRID_WIDTH + DIM] = global_grid[i];
            }
            for (int i = GRID_WIDTH; i < GRID_WIDTH + DIM; i++)
            {
                arr[i - GRID_WIDTH] = global_grid[i - DIM];
            }
        }
        int * final = (int *)malloc(DIM * ((DIM / num_procs)) * sizeof(int));

        for (int k = DIM; k < DIM * ((DIM / num_procs) + 1); k++)
        {
            int total_rows = DIM * (DIM / num_procs) + 2;
            int r = k / DIM;
            int c = k % DIM;
            int prev_r = mod(r - 1, total_rows);
            int prev_c = mod(c - 1, DIM);
            int next_r = mod(r + 1, total_rows);
            int next_c = mod(c + 1, DIM);

            int count = arr[prev_r * DIM + prev_c] + arr[prev_r * DIM + c] + arr[prev_r * DIM + next_c] + arr[r * DIM + prev_c] + arr[r * DIM + next_c] + arr[next_r * DIM + prev_c] + arr[next_r * DIM + c] + arr[next_r * DIM + next_c];
            if (arr[k] == 1)
            {
                if (count < 2)
                    final[k - DIM] = 0;
                else if (count > 3)
                    final[k - DIM] = 0;
                else
                    final[k - DIM] = 1;
            }
            else
            {
                if (count == 3)
                    final[k - DIM] = 1;
                else
                    final[k - DIM] = 0;
            }
        }

        j = 0;
        for (int i = ID * (GRID_WIDTH / num_procs); i < (ID + 1) * (GRID_WIDTH / num_procs); i++)
        {
            global_grid[i] = final[j];
            j++;
        }
        MPI_Gather(final, DIM * (DIM / num_procs), MPI_INT, &global_grid, DIM * (DIM / num_procs), MPI_INT, 0, MPI_COMM_WORLD); 
      
        if (ID == 0)
        {
            printf("\nIteration %d: final grid:\n", iters);
            for (j = 0; j < GRID_WIDTH; j++)
            {
                if (j % DIM == 0)
                {
                    printf("\n");
                }
                printf("%d  ", global_grid[j]);
            }
            printf("\n");
        }
    }

    free(arr);
    MPI_Finalize();
}
