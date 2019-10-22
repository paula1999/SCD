//Paula Villanueva Nuñez 49314567Z
#include <iostream>
#include <cassert>
#include <thread>
#include <mutex>
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include "Semaphore.h"

using namespace std ;
using namespace SEM ;

//**********************************************************************
// plantilla de función para generar un entero aleatorio uniformemente
// distribuido entre dos valores enteros, ambos incluidos
// (ambos tienen que ser dos constantes, conocidas en tiempo de compilación)
//----------------------------------------------------------------------

const int num_fumadores = 3; // Número de fumadores
Semaphore m_libre = 1, // 1 si está libre, 0 si está ocupado
            mostrador[3] = {0, 0, 0}, // 0 si no hay ingrediente, 1 si sí hay
            pale_vacio = 3, // Controla cuantos lotes hay en el pale
            pale[3] = {0, 0, 0}, // Se almacenan los lotes en el pale, 1 si hay lote, 0 si no
            escribir = 1;
int contador_lotes = 0; // Cuenta cuantos lotes hay disponibles
mutex mtx;

template< int min, int max > int aleatorio (){
  static default_random_engine generador((random_device())());
  static uniform_int_distribution<int> distribucion_uniforme(min, max);
  return distribucion_uniforme(generador);
}

//----------------------------------------------------------------------
// función que ejecuta la hebra del estanquero
void funcion_hebra_estanquero (){
    while (true){
        sem_wait(pale[contador_lotes]); // Coge el ultimo lote si hay, si no espera

        mtx.lock();
        cout << "He cogido un lote\n";
        mtx.unlock();

        sem_signal(pale_vacio); // Incrementa el numero de lotes que puede haber en el pale
        sem_wait(escribir);
        contador_lotes--;
        sem_signal(escribir);

        int ingrediente = aleatorio<0, num_fumadores-1>();

        sem_wait(m_libre);  // Espera a que el mostrador esté libre (m_libre == 1)

        mtx.lock();
        cout << "Pongo el ingrediente numero: " << ingrediente << endl;
        mtx.unlock();

        sem_signal(mostrador[ingrediente]); // pone en el mostrador el ingrediente
    }
}

void funcion_hebra_suministradora (){
    while (true){
        sem_wait(pale_vacio); //Espera a que el pale este vacio (>=1)

        mtx.lock();
        cout << "Ha llegado un nuevo lote\n";
        mtx.unlock();

        sem_signal(pale[contador_lotes]); // Pone un nuevo lote

        sem_wait(escribir);
        contador_lotes++;
        sem_signal(escribir);
    }
}

//-------------------------------------------------------------------------
// Función que simula la acción de fumar, como un retardo aleatoria de la hebra

void fumar (int num_fumador){
   // calcular milisegundos aleatorios de duración de la acción de fumar)
   chrono::milliseconds duracion_fumar(aleatorio<20,200>());

   // informa de que comienza a fumar
    cout << "Fumador " << num_fumador << "  :"
          << " empieza a fumar (" << duracion_fumar.count() << " milisegundos)" << endl;

   // espera bloqueada un tiempo igual a ''duracion_fumar' milisegundos
   this_thread::sleep_for(duracion_fumar);

   // informa de que ha terminado de fumar
    cout << "Fumador " << num_fumador << "  : termina de fumar, comienza espera de ingrediente." << endl;

}

//----------------------------------------------------------------------
// función que ejecuta la hebra del fumador
void  funcion_hebra_fumador (int num_fumador){
   while (true){
       sem_wait(mostrador[num_fumador]); // espera a que haya algo en el mostrador

       cout << "Fumador " << num_fumador << "  : ha cogido el ingrediente\n";

       sem_signal(m_libre); // ahora el mostrador está libre
       fumar(num_fumador);  // empieza a fumar
   }
}

//----------------------------------------------------------------------

int main (){
   thread h_estanquero, h_fumadores[num_fumadores], suministradora;

   suministradora = thread(funcion_hebra_suministradora);
   h_estanquero = thread(funcion_hebra_estanquero);

   for (int i = 0; i < num_fumadores; i++)
        h_fumadores[i] = thread(funcion_hebra_fumador, i);

   for (int i = 0; i < num_fumadores; i++)
        h_fumadores[i].join();

    h_estanquero.join();
    suministradora.join();
}
