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

mutex mtx;
const int n = 4;

/*

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
    if (mostrador != -1)
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

*/


class Barberia : public HoareMonitor{
private:
    CondVar sala_espera, silla, barbero;

public:
    Barberia ();
    void cortarPelo (int num_cliente);
    void siguienteCliente ();
    void finCliente ();
};

Barberia::Barberia (){
    sala_espera = newCondVar ();
    silla = newCondVar ();
    barbero = newCondVar ();
}

void Barberia::cortarPelo (int num_cliente){
    cout << "El cliente " << num_cliente << " ha entrado en la barberia\n";

    if (!barbero.empty()){
        mtx.lock();
        cout << "El cliente " << num_cliente << " encuentra la barberia vacia\n";
        mtx.unlock();

        barbero.signal();
    }
    else{
        mtx.lock();
        cout << "El cliente " << num_cliente << " se sienta en la sala de espera\n";
        mtx.unlock();

        sala_espera.wait();

        mtx.lock();
        cout << "El cliente " << num_cliente << " ha acabado de esperar\n";
        mtx.unlock();
    }

    cout << "El cliente " << num_cliente << " se ha sentado en la silla\n";

    silla.wait();

    cout << "El cliente " << num_cliente << " ha terminado de cortarse el pelo\n";
}

void Barberia::siguienteCliente (){
    if (sala_espera.empty() and silla.empty()){ // no hay nadie en la barberia
        mtx.lock();
        cout << "No hay nadie en la barberia, el barbero se duerme\n";
        mtx.unlock();

        barbero.wait();
    }
    else if (silla.empty()){ // hay alguien en la sala de espera y no en la silla
        mtx.lock();
        cout << "Silla vacia, que entre el siguiente cliente\n";
        mtx.unlock();

        sala_espera.signal();
    }
}

void Barberia::finCliente (){
    mtx.lock();
    cout << "El cliente ha terminado. Silla libre\n";
    mtx.unlock();

    silla.signal();
}

template<int min, int max> int aleatorio (){
  static default_random_engine generador((random_device())());
  static uniform_int_distribution<int> distribucion_uniforme(min, max);
  return distribucion_uniforme(generador);
}

void esperarFueraBarberia (int num_cliente){
    // calcular milisegundos aleatorios de duración de la acción de esperar fuera)
    chrono::milliseconds duracion_esperar_fuera(aleatorio<20,200>());

    // informa de que comienza a esperar fuera
    mtx.lock();
    cout << "Cliente " << num_cliente << "  :" << " empieza a esperar fuera de la barbería (" << duracion_esperar_fuera.count() << " milisegundos)" << endl;
    mtx.unlock();

    // espera bloqueada un tiempo igual a 'duracion_esperar_fuera' milisegundos
    this_thread::sleep_for(duracion_esperar_fuera);

    // informa de que ha terminado de esperar fuera
    mtx.lock();
    cout << "Cliente " << num_cliente << "  : termina de esperar fuera." << endl;
    mtx.unlock();
}

void cortarPeloACliente (){
    // calcular milisegundos aleatorios de duración de la acción de cortar)
    chrono::milliseconds duracion_cortar(aleatorio<20,200>());

    // informa de que comienza a cortar
    mtx.lock();
    cout << "Cortando pelo al cliente durante " << duracion_cortar.count() << " milisegundos" << endl;
    mtx.unlock();

    // espera bloqueada un tiempo igual a 'duracion_cortar' milisegundos
    this_thread::sleep_for(duracion_cortar);

    // informa de que ha terminado de cortar
    mtx.lock();
    cout << "He terminado de cortar el pelo, comienza espera de nuevo cliente" << endl;
    mtx.unlock();
}

void funcion_hebra_barbero (MRef<Barberia> barberia){
    while (true){
        barberia->siguienteCliente();
        cortarPeloACliente();
        barberia->finCliente();
    }
}

void funcion_hebra_cliente (MRef<Barberia> barberia, int num_cliente){
    while (true){
        barberia->cortarPelo(num_cliente);
        esperarFueraBarberia(num_cliente);
    }
}

int main (){
   cout << "-------------------------------------------------------------------------------" << endl
          << "Problema del barbero durmiente." << endl
          << "-------------------------------------------------------------------------------" << endl
          << flush;

   MRef<Barberia> barberia = Create<Barberia>();
   thread h_barbero, h_clientes[n];

   h_barbero = thread(funcion_hebra_barbero, barberia);

   for (int i = 0; i < n; i++)
        h_clientes[i] = thread(funcion_hebra_cliente, barberia, i);

   for (int i = 0; i < n; i++)
        h_clientes[i].join();

    h_barbero.join();
}
