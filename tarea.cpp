#include <cstdlib>
#include <iostream>
#include <fstream>

#include "mpi.h"

bool comprobarNumeroPotenciaDeDos(int);

int main(int argc, char ** argv){
    // identificador de cada proceso;
    int idProceso;
    // numero total de procesos;
    int numProcesos;
    // Iniciliza MPI
    MPI_Init(&argc,&argv);
    // arreglos
    int * arregloTotal;
    int * arregloLocal;
    
    // banderas
    bool parametrosCorrectos = true;
    
    //Obtiene el numero total de procesos
    MPI_Comm_size(MPI_COMM_WORLD,&numProcesos); 
   
    //Obtiene el identificador del proceso
    MPI_Comm_rank(MPI_COMM_WORLD,&idProceso);
       
    // cantidad de numeros del vector
    int tamArreglo = 0;
    
    
    
    if( idProceso == 0 ){
        srand(time(0));
        // solicita el numero de elementos que contendra el arreglo
        std::cout << "Inserte el numero !";
        std::cin >> tamArreglo;
        if(! tamArreglo){ // se ingreso un numero invalido
            std::cout << "Error en el numero ingresado" << std::endl;
        }
        
            // verifica que el numero de elementos que contendra el arreglo sea valido
        if( tamArreglo && 
            // verifica que el numero de procesos sea una potencia de dos
            comprobarNumeroPotenciaDeDos(numProcesos) &&
            // verifica que el tamano del arreglo sea mayor o igual al numero de procesos
            tamArreglo >= numProcesos && 
            // verifica que el tamano del arreglo sea divisible por el numero de procesos
            !(tamArreglo%numProcesos)  
         )
        {
            // inicializa el arreglo total
            arregloTotal = new int[tamArreglo];
            // rellena el arreglo con numeros aleatorios
            for(int i = 0; i < tamArreglo;++i){
                arregloTotal[i] = rand()%2000;
            }
            // abre el archivo ListaI
            std::ofstream archivoListaInicial("ListaI.txt");
            // verifica si puedo abrise
            if( archivoListaInicial.is_open() )
            {
                // copia los elementos del arreglo en el archivo
                for(int i = 0; i < tamArreglo;++i){
                    archivoListaInicial << arregloTotal[i] << std::endl;
                }
            }
            else{
                std::cerr << "No se pudo crear el archivo ListaI.txt" << std::endl;
                //Se advierte pero se continua con la ejecución normal
            }
            // ahora se tiene el tamano que tendra cada arreglo local
            tamArreglo /= numProcesos;
        }
        else {
            std::cout << "ejecucion terminada: parametros invalidos" << std::endl;
            parametrosCorrectos = false;
        }
        
        
    }
    //Espera todos los procesos
    MPI_Bcast(&parametrosCorrectos, 1, MPIR_CXX_BOOL, 0, MPI_COMM_WORLD);
    
    if( parametrosCorrectos ){
        MPI_Bcast(&tamArreglo, 1, MPI_INT, 0, MPI_COMM_WORLD);
        arregloLocal = new int[tamArreglo];
        MPI_Scatter(arregloTotal,tamArreglo,MPI_INT,arregloLocal,tamArreglo,MPI_INT,0,MPI_COMM_WORLD);
        // TODO: cada proceso ordena su arreglo
        // Ciclo while donde lo van devolviendo hasta llegar a la raiz
        for(int i = 0; i < tamArreglo; ++i){
            std::cout << "Id proceso: " << idProceso << " arreglo["<<i<<"]: "<<arregloLocal[i]<<'\n';
        }
        delete[] arregloLocal;
    }
    else{
        std::cout << "parametro incorrectos" << std::endl;
    }
    
    //Fin MPI
    MPI_Finalize();
    return 0;
}

bool comprobarNumeroPotenciaDeDos(int numero){
    bool respuesta = true;
    while (numero>1 && respuesta){
        if(numero & 1){
            respuesta = false;
        }
        numero=numero>>1;
    }
    return respuesta;
}

/**
 * Se asume que el numero enviado es una potencia de dos
 **/
int obtenerExponenteDeDosCorrespondiente(int numero){
    int respuesta = 0;
    int comparador = 1;
    while (numero > comparador){
        ++respuesta;
        comparador=comparador<<1;
    }
    return respuesta;
}
