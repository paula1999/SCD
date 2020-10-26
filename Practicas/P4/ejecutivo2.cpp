// -----------------------------------------------------------------------------
//
// Sistemas concurrentes y Distribuidos.
// Práctica 4. Implementación de sistemas de tiempo real.
//
// Archivo: ejecutivo2.cpp
// Implementación del primer ejemplo de ejecutivo cíclico:
//
//   Datos de las tareas:
//   ------------
//   Ta.  T    C
//   ------------
//   A  500   100
//   B  500   150
//   C  1000  200
//   D  2000  240
//  -------------
//
// T_M = mcm (500, 500, 1000, 2000) = 2000
// T_s <= min (500, 500, 1000, 2000) = 500
// T_s >= min (100, 150, 200, 240) = 100
// Luego voy a elegir T_s = 500
//
//  Planificación (con Ts == 500 ms)
//  *---------*----------*---------*--------*
//  | A B C   | A B D    | A B C   | A B    |
//  *---------*----------*---------*--------*
//  0         500       1000      1500     2000
//
//
// - ¿Cuál es el mínimo tiempo de espera que queda al final
// de las iteraciones del ciclo secundario con tu solución?
// El mínimo tiempo de espera es 10 ms. Esto se da en el segundo
// ciclo secundario: 100 + 150 + 240 = 490 ms, luego 500 - 490 = 10 ms
//
// - ¿Sería planificable si la tarea D tuviese un tiempo de
// cómputo de 250 ms?
// Según la planificación que hemos hecho, en el segundo ciclo secundario
// tendríamos 100 + 150 + 250 = 500 ms, o sea que teóricamente estaría bien.
// Aunque cuando ejecutamos el código, vemos que se produce
// algún retraso. Esto no ocasionaría ningún problema ya que el siguiente 
// ciclo tiene holgura suficiente. Luego sí sería planificable
//
// Historial:
// Creado en diciembre de 2019
// Paula Villanueva Nuñez
// -----------------------------------------------------------------------------

#include <string>
#include <iostream> // cout, cerr
#include <thread>
#include <chrono>   // utilidades de tiempo
#include <ratio>    // std::ratio_divide

using namespace std;
using namespace std::chrono;
using namespace std::this_thread;

// tipo para duraciones en segundos y milisegundos, en coma flotante:
typedef duration<float,ratio<1,1>>    seconds_f;
typedef duration<float,ratio<1,1000>> milliseconds_f;

// -----------------------------------------------------------------------------
// tarea genérica: duerme durante un intervalo de tiempo (de determinada duración)
void Tarea (const std::string & nombre, milliseconds tcomputo){
   cout << "   Comienza tarea " << nombre << " (C == " << tcomputo.count() << " ms.) ... ";

   sleep_for(tcomputo);
   
   cout << "fin." << endl;
}

// -----------------------------------------------------------------------------
// tareas concretas del problema:
void TareaA() { Tarea ("A", milliseconds(100));}
void TareaB() { Tarea ("B", milliseconds(150));}
void TareaC() { Tarea ("C", milliseconds(200));}
void TareaD() { Tarea ("D", milliseconds(240));}

// -----------------------------------------------------------------------------
// implementación del ejecutivo cíclico:

int main(int argc, char *argv[]){
   // Ts = duración del ciclo secundario
   const milliseconds Ts(500);

   // ini_sec = instante de inicio de la iteración actual del ciclo secundario
   time_point<steady_clock> ini_sec = steady_clock::now();

   while (true){ // ciclo principal
      cout << endl
           << "---------------------------------------" << endl
           << "Comienza iteración del ciclo principal." << endl;

      for(int i = 1; i <= 4; i++){ // ciclo secundario (4 iteraciones)
         cout << endl << "Comienza iteración " << i << " del ciclo secundario." << endl;

         switch (i){
            case 1: TareaA(); TareaB(); TareaC();           break;
            case 2: TareaA(); TareaB(); TareaD();           break;
            case 3: TareaA(); TareaB(); TareaC();           break;
            case 4: TareaA(); TareaB();                     break;
         }

         // calcular el siguiente instante de inicio del ciclo secundario
         ini_sec += Ts;

         // esperar hasta el inicio de la siguiente iteración del ciclo secundario
         sleep_until(ini_sec);

         time_point<steady_clock> tiempo = steady_clock::now();

         cout << "\nHay un retardo de " << milliseconds_f(tiempo - ini_sec).count() << " milisegundos\n";

         if (tiempo - ini_sec > milliseconds(20)){
            cout << "\nEl retardo es mayor a 20 milisegundos, abortando...\n";

            exit(-1);
         }
      }
   }
}
