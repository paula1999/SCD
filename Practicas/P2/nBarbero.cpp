#include <iostream>
#include <iomanip>
#include <cassert>
#include <thread>
#include <random>
#include <mutex>
#include "Semaphore.h"
#include "HoareMonitor.h"

using namespace HM;
using namespace std;

const int num_clientes = 4, num_barberos = 2;
 
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
    cout << "Barbero " << num_barbero << " cortando pelo al cliente durante " << duracion_cortar.count() << " milisegundos" << endl;
    mtx.unlock();

    // espera bloqueada un tiempo igual a 'duracion_cortar' milisegundos
    this_thread::sleep_for(duracion_cortar);

    // informa de que ha terminado de cortar
    mtx.lock();
    cout << "Barbero " << num_barbero << ": he terminado de cortar el pelo, comienza espera de nuevo cliente" << endl;
    mtx.unlock();
}

class Barberia : public HoareMonitor{
private:
  	CondVar sala_espera, silla[num_barberos], cama[num_barberos];

public:
	Barberia();
	void cortarPelo(int num_cliente);
	void siguienteCliente(int num_barbero);
	void finCliente(int num_barbero);
};

Barberia::Barberia(){
	sala_espera = newCondVar();
	
	for (int i = 0; i < num_barberos; i++){
		silla[i] = newCondVar();
		cama[i] = newCondVar();
	}
}

void Barberia::cortarPelo(int num_cliente){
	int lugar = -1;
		
	if (sala_espera.empty())
		for (int i = 0; i < num_barberos && lugar == -1; i++)
			if(!cama[i].empty()){ 
				mtx.lock();
				cout << "El cliente " << num_cliente << " despierta al barbero " << i << endl;
				mtx.unlock();
				
				lugar = i;
				cama[lugar].signal();
			}

  while (lugar == -1){
    mtx.lock();
    cout << "Cliente " << num_cliente << " espera en la sala" << endl;
    mtx.unlock();

    sala_espera.wait();
       
    for (int i = 0; i < num_barberos && lugar == -1; i++){
      if (silla[i].empty()){
        lugar = i;

        mtx.lock();
        cout << "Cliente " << num_cliente << " va a ser pelado por el barbero " << lugar << endl;
        mtx.unlock();
      }
	  }
  }

  silla[lugar].wait();

  mtx.lock();
  cout << "El cliente " << num_cliente << " ha sido pelado por el barbero " << lugar << endl;
  mtx.unlock();
}

void Barberia::siguienteCliente (int num_barbero){
  if (sala_espera.empty() && silla[num_barbero].empty()){
    mtx.lock();
    cout << "Barbero " << num_barbero << " se va a dormir" << endl;
    mtx.unlock();
    
    cama[num_barbero].wait();
  }
  else if (silla[num_barbero].empty()){
    mtx.lock();
    cout << "Barbero " << num_barbero << " llama al siguiente cliente" << endl;
    mtx.unlock();
    
    sala_espera.signal();
  }
}

void Barberia::finCliente (int num_barbero){
  mtx.lock();
  cout << "Barbero " << num_barbero << " termina de cortar el pelo" << endl;
  mtx.unlock();
  
  silla[num_barbero].signal();
}

void funcionHebraBarbero (MRef<Barberia> monitor, int num_barbero){
  while (true){
    monitor->siguienteCliente(num_barbero);
    cortarPeloACliente(num_barbero);
    monitor->finCliente(num_barbero);
  }
}

void funcionHebraCliente(MRef<Barberia> monitor, int num_cliente){
  while (true){
    monitor->cortarPelo(num_cliente);
    esperarFueraBarberia(num_cliente);
  }
}

int main(){
  	cout << "-----------------------------------------------------------------------------" << endl
    	<< "Problema del barbero durmiente (Monitor SU)." << endl
        << "----------------------------------------------------------------------------" << endl
        << flush;

  thread hebra_cliente[num_clientes], hebra_barbero[num_barberos];
  MRef<Barberia> monitor = Create<Barberia>();

  for (int j = 0; j < num_barberos; j++)
    hebra_barbero[j] = thread(funcionHebraBarbero, monitor, j);

  for (int i = 0; i < num_clientes; i++)
    hebra_cliente[i] = thread(funcionHebraCliente, monitor, i);

  for (int i = 0; i < num_clientes; i++)
    hebra_cliente[i].join();

  for (int j = 0; j < num_barberos; j++)
    hebra_barbero[j].join();
}
