// -----------------------------------------------------------------------------
//
// Sistemas concurrentes y Distribuidos.
// Práctica 3. Implementación de algoritmos distribuidos con MPI
//
// Archivo: prodcons2.cpp
// Implementación del problema del productor-consumidor con
// un proceso intermedio que gestiona un buffer finito y recibe peticiones
// en orden arbitrario
// (versión con un único productor y un único consumidor)
//
// Historial:
// Actualizado a C++11 en Septiembre de 2017
// -----------------------------------------------------------------------------

#include <iostream>
#include <thread> // this_thread::sleep_for
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include <mpi.h>

using namespace std;
using namespace std::this_thread;
using namespace std::chrono;

const int
   num_items             = 20,
   tam_vector            = 10,
   num_productores       = 4,
   num_consumidores      = 5,
   num_procesos_esperado = num_productores + num_consumidores + 1,
   id_buffer             = num_productores,
   iter_prod             = num_items / num_productores,
   iter_cons             = num_items / num_consumidores;
int
   etiqueta_cons = 1,
   etiqueta_prod = 0;

//**********************************************************************
// plantilla de función para generar un entero aleatorio uniformemente
// distribuido entre dos valores enteros, ambos incluidos
// (ambos tienen que ser dos constantes, conocidas en tiempo de compilación)
//----------------------------------------------------------------------
template< int min, int max > int aleatorio(){
  static default_random_engine generador((random_device())());
  static uniform_int_distribution<int> distribucion_uniforme(min, max);
  
  return distribucion_uniforme(generador);
}

// ---------------------------------------------------------------------
// producir produce los números en secuencia (1,2,3,....) y lleva espera aleatorio
int producir (int num_proceso){
   static int contador = num_proceso * iter_prod;

   sleep_for(milliseconds(aleatorio<10,100>()));

   contador++;

   cout << "Productor " << num_proceso << " ha producido valor " << contador << endl << flush;
   
   return contador;
}

// ---------------------------------------------------------------------
void funcion_productor (int num_proceso){
   for (unsigned int i = 0; i < iter_prod; i++){
      // producir valor
      int valor_prod = producir(num_proceso);
      // enviar valor
      cout << "Productor " << num_proceso << " va a enviar valor " << valor_prod << endl << flush;
      
      MPI_Ssend(&valor_prod, 1, MPI_INT, id_buffer, etiqueta_prod, MPI_COMM_WORLD);
   }
}

// ---------------------------------------------------------------------
void consumir (int valor_cons, int num_proceso){
   // espera bloqueada
   sleep_for(milliseconds(aleatorio<110,200>()));

   cout << "Consumidor " << num_proceso << " ha consumido valor " << valor_cons << endl << flush;
}

// ---------------------------------------------------------------------
void funcion_consumidor (int num_proceso){
   int peticion, valor_rec;
   MPI_Status estado;

   for (unsigned int i = 0 ; i < iter_cons; i++){
      MPI_Ssend (&peticion,  1, MPI_INT, id_buffer, etiqueta_cons, MPI_COMM_WORLD);
      MPI_Recv (&valor_rec, 1, MPI_INT, id_buffer, etiqueta_prod, MPI_COMM_WORLD,&estado);

      cout << "Consumidor " << num_proceso << " ha recibido valor " << valor_rec << endl << flush;
      
      consumir(valor_rec, num_proceso);
   }
}

// ---------------------------------------------------------------------
void funcion_buffer(){
   int        buffer[tam_vector],      // buffer con celdas ocupadas y vacías
              valor,                   // valor recibido o enviado
              primera_libre       = 0, // índice de primera celda libre
              num_celdas_ocupadas = 0, // número de celdas ocupadas
              etiqueta_emisor_aceptable;    // identificador de emisor aceptable
   MPI_Status estado;                 // metadatos del mensaje recibido

   for (unsigned int i = 0; i < num_items*2; i++){
      // 1. determinar si puede enviar solo prod., solo cons, o todos
      if (primera_libre == 0)               // si buffer vacío
         etiqueta_emisor_aceptable = etiqueta_prod;       // $~~~$ solo prod.
      else if (primera_libre == tam_vector) // si buffer lleno
         etiqueta_emisor_aceptable = etiqueta_cons;      // $~~~$ solo cons.
      else                                          // si no vacío ni lleno
         etiqueta_emisor_aceptable = MPI_ANY_TAG;     // $~~~$ cualquiera

      // 2. recibir un mensaje del emisor o emisores aceptables
      MPI_Recv(&valor, 1, MPI_INT, MPI_ANY_SOURCE, etiqueta_emisor_aceptable, MPI_COMM_WORLD, &estado);

      // 3. procesar el mensaje recibido
      if (estado.MPI_TAG == etiqueta_prod){
            buffer[primera_libre++] = valor;
            //num_celdas_ocupadas++;

            cout << "Buffer ha recibido valor " << valor << endl;
       }
       else if (estado.MPI_TAG == etiqueta_cons){ 
            valor = buffer[--primera_libre];
            //num_celdas_ocupadas--;
            
            cout << "Buffer va a enviar valor " << valor << endl;

            MPI_Ssend(&valor, 1, MPI_INT, estado.MPI_SOURCE, etiqueta_prod, MPI_COMM_WORLD);
      }
   }
}

// ---------------------------------------------------------------------
int main (int argc, char *argv[]){
   int id_propio, num_procesos_actual;

   // inicializar MPI, leer identif. de proceso y número de procesos
   MPI_Init(&argc, &argv);
   MPI_Comm_rank(MPI_COMM_WORLD, &id_propio);
   MPI_Comm_size(MPI_COMM_WORLD, &num_procesos_actual);

   if (num_procesos_esperado == num_procesos_actual){
      // ejecutar la operación apropiada a 'id_propio'
      if (id_propio < id_buffer)
         funcion_productor (id_propio);
      else if (id_propio == id_buffer)
         funcion_buffer();
      else
         funcion_consumidor (id_propio);
   }
   else{
      if (id_propio == 0){ // solo el primero escribe error, indep. del rol
         cout << "el número de procesos esperados es:    " << num_procesos_esperado << endl
             << "el número de procesos en ejecución es: " << num_procesos_actual << endl
             << "(programa abortado)" << endl;
      }
   }

   // al terminar el proceso, finalizar MPI
   MPI_Finalize();
   return 0;
}
