#include <cstdlib>
#include <iostream>
#include <fstream>

#include "mpi.h"

bool comprobarNumeroPotenciaDeDos(int);
int obtenerExponenteDeDosCorrespondiente(int);
void mergesort(int *, int);
void mergesortR(int *, int *, int);
void ordenarSubArreglos(int *, int, int *, int, int *);
void ordenarSubArreglosEnDestino(int *, int, int *, int, int *);


int main(int argc, char ** argv){
    // identificador de cada proceso;
    int idProceso;
    // numero total de procesos;
    int numProcesos;
    // Iniciliza MPI
    MPI_Init(&argc,&argv);
    MPI_Status status;
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
    int tamArreglo;
    
    int exponenteProcesos;
    
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
            exponenteProcesos = obtenerExponenteDeDosCorrespondiente(numProcesos);
        }
        else {
            std::cout << "ejecucion terminada: parametros invalidos" << std::endl;
            parametrosCorrectos = false;
        }
        
        
    }
    //Espera todos los procesos
    MPI_Bcast(&parametrosCorrectos, 1, MPIR_CXX_BOOL, 0, MPI_COMM_WORLD);
    
    if( parametrosCorrectos ){
        // pasa a todos los procesos la longitud que tendra cada arreglo
        MPI_Bcast(&tamArreglo, 1, MPI_INT, 0, MPI_COMM_WORLD);
        // el numero de iteraciones que se harán
        MPI_Bcast(&exponenteProcesos, 1, MPI_INT, 0, MPI_COMM_WORLD);
        //crea el arreglo local donde cada proceso guaradará su parte del arreglo
        arregloLocal = new int[tamArreglo];
        //reparte el arreglo principal entre todos los arreglos
        MPI_Scatter(arregloTotal,tamArreglo,MPI_INT,arregloLocal,tamArreglo,MPI_INT,0,MPI_COMM_WORLD);
        //cada proceso ordena su arreglo
        mergesort(arregloLocal,tamArreglo);
        
        //Variables para la reduccion
        
        // se utiliza para conocer que proceso envía y cual recibe
        int modulo = 2;
        int selector = 1;
        
        // apuntara al vector que se va a recibir
        int * arregloTemporalParaRecibir;
        // apuntara al vector en el cual se mezclaran ambos vectores
        int * arregloTemporalDestino;
        
        // hace el proceso una cantidad de veces que corresponde al logaritmo en base dos de la cantidad de procesos
        while( exponenteProcesos ){
            
            if(idProceso % modulo == 0){
                arregloTemporalParaRecibir = new int[tamArreglo];
                MPI_Recv(arregloTemporalParaRecibir,tamArreglo,MPI_INT,idProceso+selector,19, MPI_COMM_WORLD,&status);
                tamArreglo = tamArreglo << 1;
                arregloTemporalDestino = new int[tamArreglo];
                ordenarSubArreglosEnDestino(arregloTemporalParaRecibir, tamArreglo/2, arregloLocal, tamArreglo/2, arregloTemporalDestino);
                delete[] arregloTemporalParaRecibir;
                delete[] arregloLocal;
                arregloLocal = arregloTemporalDestino;
            }
            
            else if(idProceso % modulo == selector){
                MPI_Send(arregloLocal,tamArreglo,MPI_INT,idProceso-selector,19, MPI_COMM_WORLD);
                break;
            }
            
            --exponenteProcesos;
            modulo = modulo << 1;
            selector = selector << 1;    
        }
        
        if(idProceso==0){
            
            // abre el archivo ListaF
            std::ofstream archivoListaFinal("ListaF.txt");
            // verifica si puedo abrise
            if( archivoListaFinal.is_open() )
            {
                // copia los elementos del arreglo en el archivo
                for(int i = 0; i < tamArreglo;++i){
                    archivoListaFinal << arregloLocal[i] << std::endl;
                }
            }
            else{
                std::cerr << "No se pudo crear el archivo ListaI.txt" << std::endl;
                //Se advierte pero se continua con la ejecución normal
            }
            
            // variable para la respuesta del usuario
            char respuesta = 0;
            std::cout << "Desea que se muestre en pantalla el vector ordenado? ( S | [N] )";
            std::cin >> respuesta;
            // si el usuario selecciono ver el resultado en pantalla lo umprime
            if(respuesta == 'S' | respuesta == 's')
            {
                for(int i=0; i<tamArreglo;++i){
                    std::cout << arregloLocal[i] << std::endl;
                }
            }
        }
        
        
        delete[] arregloLocal;
    }
    else{
        std::cout << "parametros incorrectos" << std::endl;
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


void mergesort(int * arreglo, int tamano)
{
    // se hace un vector temporal de tamano n
    int * vectorTemporal = new int[tamano];
    mergesortR(arreglo,vectorTemporal, tamano);
    delete [] vectorTemporal;
};

/* =========== METODOS MERGESORT ========== */

/**
 * Es el llamado mergesort que se invoca recursivamente
 */
void mergesortR(int * arreglo, int * temporal, int tamano)
{
    if(tamano == 1)
    {
        return;
    }
    
    mergesortR(arreglo,temporal, tamano/2);
    mergesortR(arreglo+(tamano/2),temporal, tamano-tamano/2);
    ordenarSubArreglos(arreglo,tamano/2, arreglo+(tamano/2),tamano-tamano/2, temporal);
}

/**
 * Metodo del mergesort que se encarga de ordenar los subarreglos
 */
void ordenarSubArreglos(int * arreglo1, int tamano1, int * arreglo2, int tamano2, int * temporal)
{
    int iteradorArreglo1 = 0;
    int iteradorArreglo2 = 0;
    int iteradorArregloTemporal = 0;
    
    //va llenando el vector temporal, comparando el primer elemento de cada subarreglo
    //se sale cuando termina de copiar todos, o ya se copiaron todos los valores de un vector
    while (iteradorArregloTemporal < tamano1+tamano2 && iteradorArreglo1 < tamano1 && iteradorArreglo2 < tamano2)
    {
        if(arreglo1[iteradorArreglo1] < arreglo2[iteradorArreglo2])
        {
            temporal[iteradorArregloTemporal] = arreglo1[iteradorArreglo1];
            ++iteradorArreglo1;
        }
        else
        {
            temporal[iteradorArregloTemporal] = arreglo2[iteradorArreglo2];
            ++iteradorArreglo2;
        }
        ++iteradorArregloTemporal;
        
    }
    
    //entra cuando ya se copiaron todos los valores de un vector, se copian todos los del otro vector que queden
    if (iteradorArregloTemporal < tamano1+tamano2)
    {
        if(iteradorArreglo1 < tamano1)
        {
            for (; iteradorArreglo1 < tamano1; ++iteradorArreglo1, ++iteradorArregloTemporal)
            {
                temporal[iteradorArregloTemporal] = arreglo1[iteradorArreglo1];
            }
        }
        else
        {
            for (; iteradorArreglo2 < tamano2; ++iteradorArreglo2, ++iteradorArregloTemporal)
            {
                temporal[iteradorArregloTemporal] = arreglo2[iteradorArreglo2];
            }
        }
        
    }
    
    //copia el arreglo temporal en los subarreglos para que el vector original sea el que termine acomodado
    iteradorArregloTemporal = 0;
    for (int i = 0; i < tamano1; ++i, ++iteradorArregloTemporal)
    {
        arreglo1[i] = temporal[iteradorArregloTemporal];
    }
    for (int i = 0; i < tamano2; ++i, ++iteradorArregloTemporal)
    {
        arreglo2[i] = temporal[iteradorArregloTemporal];
    }
}

/**
  * Metodo auxiliar basado en el de ordenar los subarreglos,
  * con la diferencia de dejar el arreglo ordenado en el
  * arreglo pasdo por parametro llamado destino
  **/
void ordenarSubArreglosEnDestino(int * arreglo1, int tamano1, int * arreglo2, int tamano2, int * destino)
{
    int iteradorArreglo1 = 0;
    int iteradorArreglo2 = 0;
    int iteradorArregloDestino = 0;
    
    //va llenando el vector destino, comparando el primer elemento de cada subarreglo
    //se sale cuando termina de copiar todos, o ya se copiaron todos los valores de un vector
    while (iteradorArregloDestino < tamano1+tamano2 && iteradorArreglo1 < tamano1 && iteradorArreglo2 < tamano2)
    {
        if(arreglo1[iteradorArreglo1] < arreglo2[iteradorArreglo2])
        {
            destino[iteradorArregloDestino] = arreglo1[iteradorArreglo1];
            ++iteradorArreglo1;
        }
        else
        {
            destino[iteradorArregloDestino] = arreglo2[iteradorArreglo2];
            ++iteradorArreglo2;
        }
        ++iteradorArregloDestino;
        
    }
    
    //entra cuando ya se copiaron todos los valores de un vector, se copian todos los del otro vector que queden
    if (iteradorArregloDestino < tamano1+tamano2)
    {
        if(iteradorArreglo1 < tamano1)
        {
            for (; iteradorArreglo1 < tamano1; ++iteradorArreglo1, ++iteradorArregloDestino)
            {
                destino[iteradorArregloDestino] = arreglo1[iteradorArreglo1];
            }
        }
        else
        {
            for (; iteradorArreglo2 < tamano2; ++iteradorArreglo2, ++iteradorArregloDestino)
            {
                destino[iteradorArregloDestino] = arreglo2[iteradorArreglo2];
            }
        }
        
    }
}
