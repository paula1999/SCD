// -----------------------------------------------------------------------------
//
// Sistemas concurrentes y Distribuidos.
// Práctica 3. Implementación de algoritmos distribuidos con MPI
//
// Archivo: filosofos-plantilla.cpp
// Implementación del problema de los filósofos (sin camarero).
// Plantilla para completar.
//
// Historial:
// Actualizado a C++11 en Septiembre de 2017
// -----------------------------------------------------------------------------


#include <mpi.h>
#include <thread> // this_thread::sleep_for
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include <iostream>

using namespace std;
using namespace std::this_thread;
using namespace std::chrono;

const int
   num_filosofos = 5,
   num_procesos  = 2*num_filosofos+1,
   etiq_sentarse = 1,
   etiq_levantarse = 2;


//**********************************************************************
// plantilla de función para generar un entero aleatorio uniformemente
// distribuido entre dos valores enteros, ambos incluidos
// (ambos tienen que ser dos constantes, conocidas en tiempo de compilación)
//----------------------------------------------------------------------
template<int min, int max> int aleatorio(){
   static default_random_engine generador((random_device())());
   static uniform_int_distribution<int> distribucion_uniforme(min, max);
   
   return distribucion_uniforme(generador);
}

// ---------------------------------------------------------------------
void funcion_filosofos (int id){
   int id_ten_izq = (id+1) % (num_procesos-1), //id. tenedor izq.
      id_ten_der = (id+num_procesos-2) % (num_procesos-1), //id. tenedor der.
      peticion,
      id_camarero = num_procesos - 1;

   while (true){
      cout << "Filósofo " << id << " solicita sentarse" << endl;

      // Solicitar sentarse al camarero
      MPI_Ssend(&peticion, 1, MPI_INT, id_camarero, etiq_sentarse, MPI_COMM_WORLD);
      
      cout << "Filósofo " << id << " solicita ten. izq." << id_ten_izq << endl;
      
      // ... solicitar tenedor izquierdo 
      MPI_Ssend(&peticion, 1, MPI_INT, id_ten_izq, 0, MPI_COMM_WORLD);

      cout << "Filósofo " << id << " solicita ten. der." << id_ten_der << endl;
      
      // ... solicitar tenedor derecho
      MPI_Ssend(&peticion, 1, MPI_INT, id_ten_der, 0, MPI_COMM_WORLD);
      
      cout << "Filósofo " << id << " comienza a comer" << endl ;
      
      sleep_for(milliseconds(aleatorio<10,100>()));

      cout << "Filósofo " << id << " suelta ten. izq. " << id_ten_izq << endl;
      
      // ... soltar el tenedor izquierdo
      MPI_Ssend(&peticion, 1, MPI_INT, id_ten_izq, 0, MPI_COMM_WORLD);

      cout << "Filósofo " << id << " suelta ten. der. " << id_ten_der << endl;
      
      // ... soltar el tenedor derecho
      MPI_Ssend(&peticion, 1, MPI_INT, id_ten_der, 0, MPI_COMM_WORLD);

      cout << "Filósofo " << id << " solicita levantarse" << endl;

      // Solicitar levantarse al camarero
      MPI_Ssend(&peticion, 1, MPI_INT, id_camarero, etiq_levantarse, MPI_COMM_WORLD);

      cout << "Filosofo " << id << " comienza a pensar" << endl;
      
      sleep_for(milliseconds(aleatorio<10,100>()));
 }
}

// ---------------------------------------------------------------------
void funcion_tenedores (int id){
   int valor, id_filosofo;  // valor recibido, identificador del filósofo
   MPI_Status estado;       // metadatos de las dos recepciones

   while (true){
      // ...... recibir petición de cualquier filósofo
      MPI_Recv (&valor, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &estado);

      // ...... guardar en 'id_filosofo' el id. del emisor
      id_filosofo = estado.MPI_SOURCE;

      cout << "Ten. " << id << " ha sido cogido por filo. " << id_filosofo << endl;

      // ...... recibir liberación de filósofo 'id_filosofo'
      MPI_Recv (&valor, 1, MPI_INT, id_filosofo, 0, MPI_COMM_WORLD, &estado);

      cout << "Ten. " << id << " ha sido liberado por filo. " << id_filosofo << endl ;
  }
}

// ---------------------------------------------------------------------
void funcion_camarero (){
   int contador = 0, valor, etiq_valida, id_filosofo;
   MPI_Status estado;       // metadatos de las dos recepciones
   bool encontrado = false;

   while (true){
      if (contador == 4)
         etiq_valida = etiq_levantarse;
      else
         etiq_valida = MPI_ANY_TAG;

      while (!encontrado){
         MPI_Iprobe(id_filosofo, etiq_valida, MPI_COMM_WORLD, &encontrado, &estado);

         if (!encontrado){
            cout << "No hay peticion del filósofo " << id_filosofo/2 << endl;

            id_filosofo = (id_filosofo + 2) % (2 * num_filosofos);
         }
         else
            cout << "Se ha encontrado petición del filósofo " << id_filosofo/2 << endl;
      }

      MPI_Recv (&valor, 1, MPI_INT, id_filosofo, etiq_valida, MPI_COMM_WORLD, &estado);

      if (estado.MPI_TAG == etiq_sentarse){
         cout << "El filosofo " << estado.MPI_SOURCE << " se ha sentado en la mesa\n";

         contador++;
      }
      else{
         cout << "El filosofo " << estado.MPI_SOURCE << " se ha levantado de la mesa\n";

         contador--;
      }
   }
}

// ---------------------------------------------------------------------
int main (int argc, char** argv){
   int id_propio, num_procesos_actual;

   MPI_Init (&argc, &argv);
   MPI_Comm_rank (MPI_COMM_WORLD, &id_propio);
   MPI_Comm_size (MPI_COMM_WORLD, &num_procesos_actual);

   if (num_procesos == num_procesos_actual){
      // ejecutar la función correspondiente a 'id_propio'
      if (id_propio == num_procesos - 1)
         funcion_camarero();
      else if (id_propio % 2 == 0)          // si es par
         funcion_filosofos (id_propio); //   es un filósofo
      else                               // si es impar
         funcion_tenedores (id_propio); //   es un tenedor
   }
   else{
      if (id_propio == 0){ // solo el primero escribe error, indep. del rol
         cout << "el número de procesos esperados es:    " << num_procesos << endl
             << "el número de procesos en ejecución es: " << num_procesos_actual << endl
             << "(programa abortado)" << endl;
      }
   }

   MPI_Finalize();

   return 0;
}