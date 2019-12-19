#include <iostream>
#include <cassert>
#include <thread>
#include <mutex>
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include "Semaphore.h"
#include "HoareMonitor.h"

using namespace std ;
using namespace HM;

const int num_fumadores = 3; // Número de fumadores
mutex mtx;

//**********************************************************************
// plantilla de función para generar un entero aleatorio uniformemente
// distribuido entre dos valores enteros, ambos incluidos
// (ambos tienen que ser dos constantes, conocidas en tiempo de compilación)
//----------------------------------------------------------------------
template<int min, int max> int aleatorio (){
  static default_random_engine generador((random_device())());
  static uniform_int_distribution<int> distribucion_uniforme(min, max);
  return distribucion_uniforme(generador);
}

// Función que simula la acción de fumar, como un retardo aleatoria de la hebra
void fumar (int num_fumador){
    // calcular milisegundos aleatorios de duración de la acción de fumar)
    chrono::milliseconds duracion_fumar(aleatorio<20,200>());

    // informa de que comienza a fumar
    mtx.lock();
    cout << "Fumador " << num_fumador << "  :" << " empieza a fumar (" << duracion_fumar.count() << " milisegundos)" << endl;
    mtx.unlock();

    // espera bloqueada un tiempo igual a ''duracion_fumar' milisegundos
    this_thread::sleep_for(duracion_fumar);

    // informa de que ha terminado de fumar
    mtx.lock();
    cout << "Fumador " << num_fumador << "  : termina de fumar, comienza espera de ingrediente." << endl;
    mtx.unlock();
}

int producirIngrediente (){
    this_thread::sleep_for(chrono::milliseconds(aleatorio<20,200>()));

    int ingrediente = aleatorio<0, num_fumadores-1>();

    mtx.lock();
    cout << "He producido el ingrediente: " << ingrediente << endl;
    mtx.unlock();

    return ingrediente;
}

class Estanco : public HoareMonitor{
private:
    int mostrador; // Indica qué ingrediente hay en el mostrador
    CondVar estanquero, fumador[num_fumadores];

public:
    Estanco ();
    void obtenerIngrediente (int num_fumador);
    void ponerIngrediente (int ingrediente);
    void esperarRecogidaIngrediente ();
};

Estanco::Estanco (){
    mostrador = -1; // No hay ningún ingrediente en el mostrador
    estanquero = newCondVar ();

    for (int i = 0; i < num_fumadores; i++)
        fumador[i] = newCondVar ();
}

void Estanco::obtenerIngrediente (int num_fumador){
    if (num_fumador != mostrador)
        fumador[num_fumador].wait(); // Espera a que esté su ingrediente en el mostrador

    mtx.lock();
    cout << "He obtenido el ingrediente " << num_fumador << endl;
    mtx.unlock();

    mostrador = -1; // Ahora no hay ingredientes en el mostrador

    estanquero.signal(); // Avisa al estanquero para que produzca otro ingrediente
}

void Estanco::esperarRecogidaIngrediente (){
    if (mostrador != -1) // Si no esta vacio
        estanquero.wait(); // Espera a que recojan el ingrediente
}

void Estanco::ponerIngrediente (int ingrediente){
    mtx.lock();
    cout << "Pongo el ingrediente " << ingrediente << endl;
    mtx.unlock();

    mostrador = ingrediente; // Se pone el ingrediente en el mostrador

    fumador[ingrediente].signal(); // Se avisa al fumador
}

// función que ejecuta la hebra del fumador
void funcion_hebra_fumador (MRef<Estanco> estanco, int num_fumador){
    while (true){
        estanco->obtenerIngrediente (num_fumador);
        fumar (num_fumador);  // empieza a fumar
   }
}

// función que ejecuta la hebra del estanquero
void funcion_hebra_estanquero (MRef<Estanco> estanco){
    int ingrediente;

    while (true){
        ingrediente = producirIngrediente ();
        estanco->ponerIngrediente (ingrediente);
        estanco->esperarRecogidaIngrediente (); // Espera a que recojan el ingrediente
    }
}

int main (){
   cout << "-------------------------------------------------------------------------------" << endl
        << "Problema de los fumadores (Monitor SU)." << endl
        << "-------------------------------------------------------------------------------" << endl
        << flush;

    MRef<Estanco> estanco = Create<Estanco>();
    thread h_estanquero, h_fumadores[num_fumadores];

    h_estanquero = thread(funcion_hebra_estanquero, estanco);

    for (int i = 0; i < num_fumadores; i++)
        h_fumadores[i] = thread(funcion_hebra_fumador, estanco, i);

    for (int i = 0; i < num_fumadores; i++)
        h_fumadores[i].join();

    h_estanquero.join();
}
