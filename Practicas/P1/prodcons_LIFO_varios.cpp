#include <iostream>
#include <cassert>
#include <thread>
#include <mutex>
#include <random>
#include "Semaphore.h"

using namespace std;
using namespace SEM;

//**********************************************************************
// variables compartidas

const int num_items = 50 ,   // número de items
	       tam_vec  = 10 ,   // tamaño del buffer
		   nprod = 4 ,
		   ncons = 5 ;
unsigned  cont_prod[num_items] = {0}, // contadores de verificación: producidos
          cont_cons[num_items] = {0}; // contadores de verificación: consumidos
Semaphore libres = num_items, ocupadas = 0, asignar = 1;
int vec[tam_vec], contador = 0;
mutex mtx, mtx2, mtx3;

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

//**********************************************************************
// funciones comunes a las dos soluciones (fifo y lifo)
//----------------------------------------------------------------------

int producir_dato (){
   static int contador = 0;

   this_thread::sleep_for( chrono::milliseconds(aleatorio<20,100>()));

   mtx.lock();

   cout << "producido: " << contador << endl << flush;

   mtx.unlock();

   cont_prod[contador]++;
   return contador++;
}

//----------------------------------------------------------------------

void consumir_dato (unsigned dato){
   assert(dato < num_items);

   cont_cons[dato]++;

   this_thread::sleep_for(chrono::milliseconds(aleatorio<20,100>()));

   mtx.lock();

   cout << "                  consumido: " << dato << endl;

   mtx.unlock();
}

//----------------------------------------------------------------------

void test_contadores(){
   bool ok = true;

   cout << "comprobando contadores ....";

   for (unsigned i = 0 ; i < num_items; i++){
	   	if (cont_prod[i] != 1){
		  	cout << "error: valor " << i << " producido " << cont_prod[i] << " veces." << endl;
        	ok = false;
		}
      	if (cont_cons[i] != 1){
			cout << "error: valor " << i << " consumido " << cont_cons[i] << " veces" << endl;
         	ok = false;
      }
   }

   if (ok)
      cout << endl << flush << "solución (aparentemente) correcta." << endl << flush;
}

//----------------------------------------------------------------------

void  funcion_hebra_productora (int ih){
	int dato;

   	for (unsigned i = ih; i < num_items; i += nprod){
      	dato = producir_dato();

      	sem_wait(libres);
		sem_wait(asignar);

		mtx2.lock();
		vec[contador] = dato;
		contador++;
		mtx2.unlock();

		sem_signal(asignar);
		sem_signal(ocupadas);
   	}
}

//----------------------------------------------------------------------

void funcion_hebra_consumidora (int ih){
	int dato;

   	for (unsigned i = ih; i < num_items; i += ncons){;
		sem_wait(ocupadas);
		sem_wait(asignar);

		mtx3.lock();
		contador--;
		dato = vec[contador];
		mtx3.unlock();

		sem_signal(asignar);
		sem_signal(libres);

      	consumir_dato(dato);
    }
}
//----------------------------------------------------------------------

int main(){
   cout << "--------------------------------------------------------" << endl
        << "Problema de los productores-consumidores (solución LIFO)." << endl
        << "--------------------------------------------------------" << endl
        << flush;

   thread hebra_productora[nprod],
          hebra_consumidora[ncons];

   for (int i = 0; i < nprod; i++)
	   hebra_productora[i] = thread(funcion_hebra_productora, i);

   for (int i = 0; i < ncons; i++)
	   hebra_consumidora[i] = thread(funcion_hebra_consumidora, i);

   for (int i = 0; i < nprod; i++)
       hebra_productora[i].join();

   for (int i = 0; i < ncons; i++)
   	   hebra_consumidora[i].join();

   test_contadores();
}
