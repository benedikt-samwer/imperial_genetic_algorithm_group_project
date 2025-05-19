#include <cstdlib>
#include <iostream>

#include <mpi.h>

/* Example program using rand from cstdlib
   to generate random numbers

   Advantages:
    - Conceptually simple to use
    - Works ok with MPI

   Disadvantages:
    - Not thread safe (on Windows)
    - Not very good random numbers (on Windows)
    - Blocking call to rand() (on mac/linux)
    - Inconsistent behaviour across platforms


*/


int main() {

    int count = 0;
    int n_trials = 10000000;

    int rank, size;

    MPI_Init(NULL, NULL);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    srand(time(NULL)+10*rank);

    int n_local_trials;
    
    if (rank == 0)
    {
        n_local_trials = n_trials/size + n_trials%size;
    }
    else
    {
        n_local_trials = n_trials/size;
    }

    #pragma omp parallel for reduction(+:count)
    for (int i = 0; i < n_local_trials; i++) {
        int random_number = rand() % 6 + 1;
        if (random_number % 3 == 0)
            count++;
    }

    MPI_Allreduce(MPI_IN_PLACE, &count, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);

    if (rank == 0)
        std::cout << "Fraction of multiples of 3: " << count / (1.0 * n_trials) << std::endl;

    MPI_Finalize();

    return 0;
}