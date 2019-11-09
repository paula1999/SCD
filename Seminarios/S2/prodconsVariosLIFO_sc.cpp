// -----------------------------------------------------------------------------
//
// Sistemas concurrentes y Distribuidos.
// Seminario 2. Introducción a los monitores en C++11.
//
// archivo: prodconsVariosLIFO_sc.cpp
// Ejemplo de un monitor en C++11 con semántica SC, para el problema
// del productor/consumidor, con varios productores y varios consumidores.
// Opcion LIFO (stack)
//
// Historial:
// Creado en Julio de 2017
// Actualizado en octubre 2019 por Paula Villanueva Núñez
// -----------------------------------------------------------------------------

#include <iostream>
#include <iomanip>
#include <cassert>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <random>

using namespace std;

const int   np = 5, // Número hebras productoras
            nc = 4; // Número hebras consumidoras

constexpr int num_items = 40;  // número de items a producir/consumir

mutex mtx; // mutex de escritura en pantalla

unsigned    cont_prod[num_items], // contadores de verificación: producidos
            cont_cons[num_items], // contadores de verificación: consumidos
            cont_nprod[np] = {0}; // contadores de verificación: nprod

//**********************************************************************
// plantilla de función para generar un entero aleatorio uniformemente
// distribuido entre dos valores enteros, ambos incluidos
// (ambos tienen que ser dos constantes, conocidas en tiempo de compilación)
//----------------------------------------------------------------------
template<int min, int max> int aleatorio(){
  static default_random_engine generador((random_device())());
  static uniform_int_distribution<int> distribucion_uniforme(min, max) ;
  return distribucion_uniforme(generador);
}

//**********************************************************************
// funciones comunes a las dos soluciones (fifo y lifo)
//----------------------------------------------------------------------
int producir_dato (int ih){
   this_thread::sleep_for(chrono::milliseconds(aleatorio<20,100>()));

   mtx.lock();
   cout << "producido: " << ih*num_items/np + cont_nprod[ih] << endl << flush;
   mtx.unlock();

   cont_prod[ih*num_items/np + cont_nprod[ih]]++;
   return ih*num_items/np + cont_nprod[ih]++;
}

void consumir_dato (unsigned dato){
   if (num_items <= dato){
      cout << " dato === " << dato << ", num_items == " << num_items << endl ;
      assert(dato < num_items);
   }

   cont_cons[dato]++;

   this_thread::sleep_for( chrono::milliseconds( aleatorio<20,100>() ));

   mtx.lock();
   cout << "                  consumido: " << dato << endl;
   mtx.unlock();
}

void ini_contadores (){
   for (unsigned i = 0; i < num_items; i++){
      cont_prod[i] = 0;
      cont_cons[i] = 0;
   }
}

void test_contadores (){
   bool ok = true;

   cout << "comprobando contadores ...." << flush;

   for (unsigned i = 0; i < num_items; i++){
      if (cont_prod[i] != 1){
         cout << "error: valor " << i << " producido " << cont_prod[i] << " veces." << endl ;
         ok = false;
      }
      if (cont_cons[i] != 1){
         cout << "error: valor " << i << " consumido " << cont_cons[i] << " veces" << endl ;
         ok = false;
      }
   }
   if (ok)
      cout << endl << flush << "solución (aparentemente) correcta." << endl << flush ;
}

// *****************************************************************************
// clase para monitor buffer, version LIFO, semántica SC, varios prod. y varios cons.
class ProdConsNSC{
 private:
 static const int               // constantes:
   num_celdas_total = 10;       //  núm. de entradas del buffer
 int                            // variables permanentes
   buffer[num_celdas_total],    //  buffer de tamaño fijo, con los datos
   primera_libre;               //  indice de celda de la próxima inserción
 mutex
   cerrojo_monitor;         // cerrojo del monitor
 condition_variable         // colas condicion:
   ocupadas,                //  cola donde espera el consumidor (n>0)
   libres;                  //  cola donde espera el productor  (n<num_celdas_total)

 public:                    // constructor y métodos públicos
   ProdConsNSC();           // constructor
   int  leer();             // extraer un valor (sentencia L) (consumidor)
   void escribir(int valor); // insertar un valor (sentencia E) (productor)
};

ProdConsNSC::ProdConsNSC(){
   primera_libre = 0;
}

// función llamada por el consumidor para extraer un dato
int ProdConsNSC::leer(){
   // ganar la exclusión mutua del monitor con una guarda
   unique_lock<mutex> guarda(cerrojo_monitor);

   // esperar bloqueado hasta que 0 < num_celdas_ocupadas
   while (primera_libre <= 0)
      ocupadas.wait(guarda);

   // hacer la operación de lectura, actualizando estado del monitor
   assert(0 < primera_libre);

   primera_libre--;

   const int valor = buffer[primera_libre];

   // señalar al productor que hay un hueco libre, por si está esperando
   libres.notify_one();

   return valor;
}

void ProdConsNSC::escribir(int valor){
   // ganar la exclusión mutua del monitor con una guarda
   unique_lock<mutex> guarda(cerrojo_monitor);

   // esperar bloqueado hasta que num_celdas_ocupadas < num_celdas_total
   while (primera_libre == num_celdas_total)
      libres.wait(guarda);

   assert(primera_libre < num_celdas_total);

   // hacer la operación de inserción, actualizando estado del monitor
   buffer[primera_libre] = valor;
   primera_libre ++;

   // señalar al consumidor que ya hay una celda ocupada (por si esta esperando)
   ocupadas.notify_one();
}

// *****************************************************************************
// funciones de hebras
void funcion_hebra_productora (ProdConsNSC * monitor, int ih){
   for (unsigned i = ih; i < (ih + num_items/np); i++){
      int valor = producir_dato(ih);
      monitor->escribir(valor);
   }
}

void funcion_hebra_consumidora (ProdConsNSC * monitor, int ih){
   for (unsigned i = ih; i < (ih + num_items/nc); i++){
      int valor = monitor->leer();
      consumir_dato(valor);
   }
}

int main(){
   cout << "-------------------------------------------------------------------------------" << endl
        << "Problema de los productores-consumidores (5 prod/ 4 cons, Monitor SC, buffer LIFO). " << endl
        << "-------------------------------------------------------------------------------" << endl
        << flush;

   ProdConsNSC monitor;
   thread hebra_productora[np], hebra_consumidora[nc];

   for (int i = 0; i < np; i++)
        hebra_productora[i] = thread(funcion_hebra_productora, &monitor, i);

   for (int i = 0; i < nc; i++)
        hebra_consumidora[i] = thread(funcion_hebra_consumidora, &monitor, i);

   for (int i = 0; i < np; i++)
        hebra_productora[i].join();

   for (int i = 0; i < nc; i++)
        hebra_consumidora[i].join();

   // comprobar que cada item se ha producido y consumido exactamente una vez
   test_contadores();
}
