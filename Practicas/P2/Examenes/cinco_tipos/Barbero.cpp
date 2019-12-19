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
const int n = 3;

template<int min, int max> int aleatorio (){
  static default_random_engine generador((random_device())());
  static uniform_int_distribution<int> distribucion_uniforme(min, max);
  return distribucion_uniforme(generador);
}

void esperarFueraBarberia (int num_cliente, int tipo){
    // calcular milisegundos aleatorios de duración de la acción de esperar fuera)
    chrono::milliseconds duracion_esperar_fuera(aleatorio<20,200>());

    // informa de que comienza a esperar fuera
    mtx.lock();
    cout << "Cliente " << num_cliente << " tipo " << tipo << ": empieza a esperar fuera de la barbería (" << duracion_esperar_fuera.count() << " milisegundos)" << endl;
    mtx.unlock();

    // espera bloqueada un tiempo igual a 'duracion_esperar_fuera' milisegundos
    this_thread::sleep_for(duracion_esperar_fuera);

    // informa de que ha terminado de esperar fuera
    mtx.lock();
    cout << "Cliente " << num_cliente << " tipo " << tipo << ": termina de esperar fuera." << endl;
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

class Barberia : public HoareMonitor{
private:
    CondVar sala_espera0, sala_espera1, silla, barbero;
    int siguiente_tipo;

public:
    Barberia ();
    void cortarPelo (int num_cliente, int tipo);
    void siguienteCliente ();
    void finCliente ();
};

Barberia::Barberia (){
    siguiente_tipo = 0;
    sala_espera0 = newCondVar ();
    sala_espera1 = newCondVar();
    silla = newCondVar ();
    barbero = newCondVar ();
}

void Barberia::cortarPelo (int num_cliente, int tipo){
    mtx.lock();
    cout << "El cliente " << num_cliente << " tipo " << tipo << " ha entrado en la barberia\n";
    mtx.unlock();

    if (!silla.empty() || siguiente_tipo != tipo){
        mtx.lock();
        cout << "El cliente " << num_cliente <<  " tipo " << tipo << " se sienta en la sala de espera\n";
        mtx.unlock();

        if (tipo == 0)
            sala_espera0.wait();
        else
            sala_espera1.wait();

        mtx.lock();
        cout << "El cliente " << num_cliente <<  " tipo " << tipo << " ha acabado de esperar\n";
        mtx.unlock();
    }

    if (!barbero.empty()){
        mtx.lock();
        cout << "El cliente " << num_cliente << " tipo " << tipo << " encuentra la barberia vacia\n";
        mtx.unlock();

        if (siguiente_tipo == tipo){
            mtx.lock();
            cout << "Soy el siguiente tipo " << tipo << " y despierto al barbero\n";
            mtx.unlock();

            barbero.signal();
        }
    }

    mtx.lock();
    cout << "El cliente " << num_cliente <<  " tipo " << tipo << " se ha sentado en la silla\n";
    mtx.unlock();

    silla.wait();

    mtx.lock();
    cout << "El cliente " << num_cliente <<  " tipo " << tipo << " ha terminado de cortarse el pelo\n";
    mtx.unlock();
}

void Barberia::siguienteCliente (){
    if ((siguiente_tipo == 0 && sala_espera0.empty() && silla.empty()) || (siguiente_tipo == 1 && sala_espera1.empty() && silla.empty())){ // no hay nadie en la barberia
        mtx.lock();
        cout << "No hay nadie en la barberia, el barbero se duerme\n";
        mtx.unlock();

        barbero.wait();

        mtx.lock();
        cout << "El barbero se despierta\n";
        mtx.unlock();
    }
    else if (silla.empty()){ // hay alguien en la sala de espera y no en la silla
        mtx.lock();
        cout << "Silla vacia, que entre el siguiente cliente\n";
        mtx.unlock();

        if (siguiente_tipo == 0)
            sala_espera0.signal();
        else 
            sala_espera1.signal();
    }
}

void Barberia::finCliente (){
    mtx.lock();
    cout << "El cliente ha terminado. Silla libre\n";
    mtx.unlock();

    if (siguiente_tipo == 0)
        siguiente_tipo = 1;
    else 
        siguiente_tipo = 0;

    silla.signal();
}

void funcion_hebra_barbero (MRef<Barberia> barberia){
    while (true){
        barberia->siguienteCliente();
        cortarPeloACliente();
        barberia->finCliente();
    }
}

void funcion_hebra_cliente (MRef<Barberia> barberia, int num_cliente, int tipo){
    while (true){
        barberia->cortarPelo(num_cliente, tipo);
        esperarFueraBarberia(num_cliente, tipo);
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
        h_clientes[i] = thread(funcion_hebra_cliente, barberia, i, i%2);

    for (int i = 0; i < n; i++)
        h_clientes[i].join();

    h_barbero.join();
}
