// No funciona
#include <iostream>
#include <cassert>
#include <thread>
#include <mutex>
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include "Semaphore.h"
#include "HoareMonitor.h"

using namespace std;
using namespace HM;

const int num_fumadores = 6; // Número de fumadores
const int num_estanqueros = 2; // Número de estanqueros
bool cerrar = false;
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

int producirIngrediente (int num_estanquero){
    this_thread::sleep_for(chrono::milliseconds(aleatorio<20,200>()));

    int ingrediente = aleatorio<0, 3>();

    mtx.lock();
    cout << "El estanquero " << num_estanquero << " ha producido el ingrediente: " << ingrediente << endl;
    mtx.unlock();

    return ingrediente;
}

class Estanco : public HoareMonitor{
private:
    int mostrador; // Indica qué ingrediente hay en el mostrador
    CondVar estanquero[num_estanqueros], fumador[num_fumadores];
    int siguiente_estanquero;
public:
    Estanco ();
    void obtenerIngrediente (int num_fumador);
    void ponerIngrediente (int ingrediente, int num_estanquero);
    void esperarRecogidaIngrediente (int num_estanquero);
};

Estanco::Estanco (){
    mostrador = -1; // No hay ningún ingrediente en el mostrador
    siguiente_estanquero = -1;

    for (int i = 0; i < num_estanqueros; i++)
        estanquero[i] = newCondVar();

    for (int i = 0; i < num_fumadores; i++)
        fumador[i] = newCondVar();
}

void Estanco::obtenerIngrediente (int num_fumador){
    if (cerrar)
        return;

    if ((num_fumador%3) != mostrador)
        fumador[num_fumador].wait(); // Espera a que esté su ingrediente en el mostrador

    mtx.lock();
    cout << "El fumador " << num_fumador << " ha obtenido el ingrediente " << (num_fumador%3) << endl;
    mtx.unlock();

    mostrador = -1; // Ahora no hay ingredientes en el mostrador

    estanquero[siguiente_estanquero].signal(); // Avisa al estanquero para que produzca otro ingrediente
}

void Estanco::esperarRecogidaIngrediente (int num_estanquero){
    if (mostrador != -1)
        estanquero[num_estanquero].wait(); // Espera a que recojan el ingrediente
}

void Estanco::ponerIngrediente (int ingrediente, int num_estanquero){
    mtx.lock();
    cout << "Estanquero " << num_estanquero << " pone el ingrediente " << ingrediente << endl;
    mtx.unlock();

    siguiente_estanquero = num_estanquero;
    mostrador = ingrediente; // Se pone el ingrediente en el mostrador

    // Se avisa al fumador
    if (fumador[ingrediente].get_nwt() != 0)
        fumador[ingrediente].signal(); 
    else if (fumador[ingrediente+3].get_nwt() != 0)
        fumador[ingrediente+3].signal();
}

// función que ejecuta la hebra del fumador
void funcion_hebra_fumador (MRef<Estanco> estanco, int num_fumador){
    while (true){
        if (cerrar)
            return;

        estanco->obtenerIngrediente (num_fumador);
       
        if (cerrar)
            return;
       
        fumar (num_fumador);  // empieza a fumar
   }
}

// función que ejecuta la hebra del estanquero
void funcion_hebra_estanquero (MRef<Estanco> estanco, int num_estanquero){
    int ingrediente;

    while (true){
        if (cerrar)
            return;

        ingrediente = producirIngrediente (num_estanquero);
        estanco->ponerIngrediente (ingrediente, num_estanquero);
        
        if (ingrediente == 3){
            mtx.lock();
            cout << "HORA DE CERRAR EL ESTANCO\n";
            mtx.unlock();

            cerrar = true;

            return;
        }
        
        estanco->esperarRecogidaIngrediente (num_estanquero); // Espera a que recojan el ingrediente
    }
}

int main (){
    cout << "-------------------------------------------------------------------------------" << endl
          << "Problema de los fumadores (Monitor SU)." << endl
          << "-------------------------------------------------------------------------------" << endl
          << flush;

    MRef<Estanco> estanco = Create<Estanco>();
    thread h_estanquero[num_estanqueros], h_fumadores[num_fumadores];

    for (int i = 0; i < num_estanqueros; i++)
        h_estanquero[i] = thread(funcion_hebra_estanquero, estanco, i);

    for (int i = 0; i < num_fumadores; i++)
        h_fumadores[i] = thread(funcion_hebra_fumador, estanco, i);

    for (int i = 0; i < num_fumadores; i++)
        h_fumadores[i].join();

    for (int i = 0; i < num_estanqueros; i++)
        h_estanquero[i].join();
}
