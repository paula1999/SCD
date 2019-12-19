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
const int N = 8, M = 2, MAX_ESPERA = 5, MAX_CORTAR = 10;

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

void cortarPeloACliente (int num_barbero){
    // calcular milisegundos aleatorios de duración de la acción de cortar)
    chrono::milliseconds duracion_cortar(aleatorio<20,200>());

    // informa de que comienza a cortar
    mtx.lock();
    cout << "Barbero: " << num_barbero << ", cortando pelo al cliente durante " << duracion_cortar.count() << " milisegundos" << endl;
    mtx.unlock();

    // espera bloqueada un tiempo igual a 'duracion_cortar' milisegundos
    this_thread::sleep_for(duracion_cortar);

    // informa de que ha terminado de cortar
    mtx.lock();
    cout << "Barbero: " << num_barbero << ", he terminado de cortar el pelo, comienza espera de nuevo cliente" << endl;
    mtx.unlock();
}

void descansarBarbero (int num_barbero){
    // calcular milisegundos aleatorios de duración de la acción de descansar)
    chrono::milliseconds duracion_descansar(aleatorio<20,200>());

    // informa de que comienza a descansar
    mtx.lock();
    cout << "Barbero: " << num_barbero << " ha cortado el pelo a " << MAX_CORTAR << " clientes y descansa durante " << duracion_descansar.count() << " milisegundos" << endl;
    mtx.unlock();

    // espera bloqueada un tiempo igual a 'duracion_cortar' milisegundos
    this_thread::sleep_for(duracion_descansar);

    // informa de que ha terminado de descansar
    mtx.lock();
    cout << "Barbero: " << num_barbero << " ha terminado de descansar" << endl;
    mtx.unlock();
}

class Barberia : public HoareMonitor{
private:
    CondVar sala_espera, silla[M], cama;
    unsigned clientes_pelados[M];
    int siguiente_barbero;    // Indica el barbero que va a pelar al siguiente cliente

public:
    Barberia ();
    void cortarPelo (int num_cliente);
    void siguienteCliente (int num_barbero);
    bool finCliente (int num_barbero);
};

Barberia::Barberia (){
    siguiente_barbero = -1;
    sala_espera = newCondVar ();
    cama = newCondVar ();

    for (int i = 0; i < M; i++)
        silla[i] = newCondVar();
    
    for (int i = 0; i < M; i++)
        clientes_pelados[i] = 0;
}

void Barberia::cortarPelo (int num_cliente){
    cout << "El cliente " << num_cliente << " ha entrado en la barberia\n";

    if (!cama.empty()){
        mtx.lock();
        cout << "El cliente " << num_cliente << " encuentra la barberia vacia\n";
        mtx.unlock();

        cama.signal();
    }
    else if (sala_espera.get_nwt() == MAX_ESPERA){
        mtx.lock();
        cout << "Barberia llena, espero fuera\n";
        mtx.unlock();

        return;
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

    cout << "El cliente " << num_cliente << " se ha sentado en la silla con el barbero " << siguiente_barbero << "\n";

    silla[siguiente_barbero].wait();

    cout << "El cliente " << num_cliente << " ha terminado de cortarse el pelo\n";
}

void Barberia::siguienteCliente (int num_barbero){
    if (sala_espera.empty() and silla[num_barbero].empty()){ // no hay nadie en la barberia
        mtx.lock();
        cout << "No hay nadie en la barberia, el barbero " << num_barbero << " se duerme\n";
        mtx.unlock();

        cama.wait();

        mtx.lock();
        cout << "El barbero " << num_barbero << " se despierta\n";
        mtx.unlock();

        siguiente_barbero = num_barbero;
    }
    else if (silla[num_barbero].empty()){ // hay alguien en la sala de espera y no en la silla
        mtx.lock();
        cout << "Silla vacia, que entre el siguiente cliente\n";
        mtx.unlock();

        siguiente_barbero = num_barbero;

        sala_espera.signal();
    }
}

bool Barberia::finCliente (int num_barbero){
    bool descansar = false;

    mtx.lock();
    cout << "Barbero: " << num_barbero << ", el cliente ha terminado. Silla libre\n";
    mtx.unlock();

    clientes_pelados[num_barbero]++;

    silla[num_barbero].signal();

    if (clientes_pelados[num_barbero] == MAX_CORTAR){
        clientes_pelados[num_barbero] = 0;
        descansar = true;
    }

    return descansar;   
}

void funcion_hebra_barbero (MRef<Barberia> barberia, int num_barbero){
    bool descansar;

    while (true){
        barberia->siguienteCliente(num_barbero);
        cortarPeloACliente(num_barbero);
        descansar = barberia->finCliente(num_barbero);

        if (descansar)
            descansarBarbero(num_barbero);
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
   thread h_barbero[M], h_clientes[N];

   for (int i = 0; i < M; i++)
        h_barbero[i] = thread(funcion_hebra_barbero, barberia, i);

   for (int i = 0; i < N; i++)
        h_clientes[i] = thread(funcion_hebra_cliente, barberia, i);

   for (int i = 0; i < N; i++)
        h_clientes[i].join();

    for (int i = 0; i < M; i++)
        h_barbero[i].join();
}
