#include <cstdlib>
#include <iostream>

#include "mpi.h"

int main(int argc, char ** argv){
    // identificador de cada proceso;
    int idProceso;
    // numero total de procesos;
    int numProcesos;
    // Iniciliza MPI
    MPI_Init(&argc,&argv);
    
    //Obtiene el numero total de procesos
    MPI_Comm_size(MPI_COMM_WORLD,&numProcesos); 
   
    //Obtiene el identificador del proceso
    MPI_Comm_rank(MPI_COMM_WORLD,&idProceso);
    
    std::cout << "Hello World!" << std::endl;
    
    //Fin MPI
    MPI_Finalize();
    return 0;
}
