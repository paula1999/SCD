// -----------------------------------------------------------------------------
// Sistemas concurrentes y Distribuidos.
// Seminario 1. Programación Multihebra y Semáforos.
//
// Ejemplo 9 (ejemplo9.cpp)
// Calculo concurrente de una integral.
//
// Historial:
// Creado en Abril de 2017
// Completado en septiembre de 2019 por Paula Villanueva Núñez
// -----------------------------------------------------------------------------

#include <iostream>
#include <iomanip>
#include <chrono>  // incluye now, time\_point, duration
#include <future>
#include <vector>
#include <cmath>

using namespace std ;
using namespace std::chrono;

const long m  = 1024l*1024l*1024l,
           n  = 4  ;


// -----------------------------------------------------------------------------
// evalua la función $f$ a integrar ($f(x)=4/(1+x^2)$)
double f (double x){
  return 4.0/(1.0+x*x);
}
// -----------------------------------------------------------------------------
// calcula la integral de forma secuencial, devuelve resultado:
double calcular_integral_secuencial(){
   double suma = 0.0;                      // inicializar suma

   for (long i = 0; i < m; i++)            // para cada $i$ entre $0$ y $m-1$:
      suma += f((i+double(0.5))/m);        //   $~$ añadir $f(x_i)$ a la suma actual
   return suma/m;                          // devolver valor promedio de $f$
}

// -----------------------------------------------------------------------------
// función que ejecuta cada hebra: recibe $i$ ==índice de la hebra, ($0\leq i<n$)
double funcion_hebra_contigua (long i){
    double suma = 0.0;                          // inicializar suma

    for (long j = i*m/n; j < i*m/n + m/n; j++)  // para cada $j$ entre $i*m/n$ y $i*m/n + m/n - 1$:
        suma += f((j+double(0.5))/m);           //  añadir $f(x_j)$ a la suma actual

    return suma;                                // devolver valor de la suma de $f$
}

// -----------------------------------------------------------------------------
// calculo de la integral de forma concurrente contigua
double calcular_integral_concurrente_contigua(){
  future<double> futuros[n];        // declaración del vector de future
  double suma = 0.0;                // inicializar suma

  for (long i = 0; i < n; i++)      // para cada $i$ entre $0$ y $n-1$:
      futuros[i] = async(launch::async, funcion_hebra_contigua, i); // inicializar el vector con la suma correspondiente

  for (long i = 0; i < n; i++)      // para cada $i$ entre $0$ y $n-1$:
      suma += futuros[i].get();     // añadir la suma de cada valor del vector

  return suma/m;                    // devolver valor promedio
}

// -----------------------------------------------------------------------------
// función que ejecuta cada hebra: recibe $i$ ==índice de la hebra, ($0\leq i<n$)
double funcion_hebra_entrelazada (long i){
    double suma = 0.0;                  // inicializar suma

    for (long j = i; j < m; j = j + n)  // para cada $j$ entre $i$ y $m - 1$:
        suma += f((j+double(0.5))/m);   //   añadir $f(x_j)$ a la suma actual

    return suma;                        // devolver valor de la suma de $f$
}

// -----------------------------------------------------------------------------
// calculo de la integral de forma concurrente entrelazada
double calcular_integral_concurrente_entrelazada(){
    future<double> futuros[n];      // declaración del vector de future
    double suma = 0.0;              // inicializar suma

    for (long i = 0; i < n; i++)    // para cada $i$ entre $0$ y $n-1$:
        futuros[i] = async(launch::async, funcion_hebra_entrelazada, i);    // inicializar el vector con la suma correspondiente

    for (long i = 0; i < n; i++)    // para cada $i$ entre $0$ y $n-1$:
        suma += futuros[i].get();   // añadir la suma de cada valor del vector

    return suma/m;                  // devolver valor promedio
}

// -----------------------------------------------------------------------------

int main(){
  time_point<steady_clock> inicio_sec  = steady_clock::now();
  const double             result_sec  = calcular_integral_secuencial();
  time_point<steady_clock> fin_sec     = steady_clock::now();
  double x = sin(0.4567);
  time_point<steady_clock> inicio_conc_contigua = steady_clock::now();
  const double             result_conc_contigua = calcular_integral_concurrente_contigua();
  time_point<steady_clock> fin_conc_contigua    = steady_clock::now();
  time_point<steady_clock> inicio_conc_entrelazada = steady_clock::now();
  const double             result_conc_entrelazada = calcular_integral_concurrente_entrelazada();
  time_point<steady_clock> fin_conc_entrelazada   = steady_clock::now();
  duration<float,milli>    tiempo_sec  = fin_sec  - inicio_sec,
                           tiempo_conc_contigua = fin_conc_contigua - inicio_conc_contigua,
                           tiempo_conc_entrelazada = fin_conc_entrelazada - inicio_conc_entrelazada;
  const float              porc_contiguo        = 100.0*tiempo_conc_contigua.count()/tiempo_sec.count(),
                           porc_entrelazado = 100.0*tiempo_conc_entrelazada.count()/tiempo_sec.count();

  constexpr double pi = 3.14159265358979323846l;

  cout << "Número de muestras (m)              : " << m << endl
       << "Número de hebras (n)                : " << n << endl
       << setprecision(18)
       << "Valor de PI                         : " << pi << endl
       << "Resultado secuencial                : " << result_sec  << endl
       << "Resultado concurrente contiguo      : " << result_conc_contigua << endl
       << "Resultado concurrente entrelazado   : " << result_conc_entrelazada << endl
       << setprecision(5)
       << "Tiempo secuencial                   : " << tiempo_sec.count()  << " milisegundos. " << endl
       << "Tiempo concurrente contiguo         : " << tiempo_conc_contigua.count() << " milisegundos. " << endl
       << "Tiempo concurrente entrelazado      : " << tiempo_conc_entrelazada.count() << " milisegundos. " << endl
       << setprecision(4)
       << "Porcentaje t.conc_contiguo/t.sec.   : " << porc_contiguo << "%" << endl
       << "Porcentaje t.conc_entrelazado/t.sec : " << porc_entrelazado << "%" << endl;
}
