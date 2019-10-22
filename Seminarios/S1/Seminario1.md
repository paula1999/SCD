# Seminario 1. Programación multihebra y semáforos
## Paula Villanueva Núñez 3º DGIIM SCD

En este seminario he implementado un programa multihebra para aproximar el número pi mediante integración numérica. Este cálculo se realiza de tres formas distintas: secuencial, concurrente de forma contigua y concurrente de forma entrelazada. Además, he medido el tiempo que tarda en cada caso.

### Programa concurrente de forma contigua

En este caso, siendo `m` el número de puntos uniformemente espaciados y `n` el número de hebras, a cada hebra le corresponden `m/n` puntos consecutivos y evalúa la función en cada uno de ellos. He asumido que `m` es múltiplo de `n`, si no lo fuera habría que redondear la operación `m/n` al siguiente número natural. Las funciones correspondientes son:

```c++
// función que ejecuta cada hebra: recibe $i$ ==índice de la hebra, ($0\leq i<n$)
double funcion_hebra_contigua (long i){
    double suma = 0.0;                     // inicializar suma

    for (long j = i*m/n; j < i*m/n + m/n; j++)
        suma += f((j+double(0.5))/m);      //  añadir $f(x_j)$ a la suma actual

    return suma;                           // devolver valor de la suma de $f$
}

// calculo de la integral de forma concurrente contigua
double calcular_integral_concurrente_contigua(){
  future<double> futuros[n];        // declaración del vector de future
  double suma = 0.0;                // inicializar suma

  for (long i = 0; i < n; i++)      // para cada $i$ entre $0$ y $n-1$:
      futuros[i] = async(launch::async, funcion_hebra_contigua, i);

  for (long i = 0; i < n; i++)      // para cada $i$ entre $0$ y $n-1$:
      suma += futuros[i].get();     // añadir la suma de cada valor del vector

  return suma/m;                    // devolver valor promedio
}
```

En la función `calcular_integral_concurrente_contigua`, declaro el vector de `future` donde almacenaré la suma de cada hebra. En el primer bucle `for` utilizo `async` para calcular dicha suma llamando a otra función. En el segundo bucle `for` acumulo la suma de cada hebra en la variable `suma` y devuelvo el valor promedio, ya que divido esta `suma` por el número de puntos.

La función `funcion_hebra_contigua` recibe el parámetro `i` que se corresponde con el índice de la hebra. Hay un bucle `for` en el que a cada `j` le corresponde un valor entre `i*m/n` y `i*m/n + m/n - 1`, avanzando en una unidad. Esto se debe a que a la hebra con índice `i` empieza en el punto `i*m/n` y avanza al siguiente punto de forma contigua hasta el último, `i*m/n + m/n - 1`, ya que `m/n` es el número de puntos consecutivos que tiene que calcularles la suma. Dentro de este bucle calculo la suma llamando a la función `f` y devuelvo la suma total de cada hebra.

### Programa concurrente de forma entrelazada

En este caso, siendo `m` el número de puntos uniformemente espaciados y `n` el número de hebras, a cada hebra le corresponden `m/n` puntos entrelazados, es decir, a cada hebra le toca el siguiente punto evaluado por la hebra anterior. Además, se evalúa la función en cada uno de ellos. Las funciones correspondientes son:

```c++
// función que ejecuta cada hebra: recibe $i$ ==índice de la hebra, ($0\leq i<n$)
double funcion_hebra_entrelazada (long i){
    double suma = 0.0;                  // inicializar suma

    for (long j = i; j < m; j = j + n)  // para cada $j$ entre $i$ y $m - 1$:
        suma += f((j+double(0.5))/m);   //   añadir $f(x_j)$ a la suma actual

    return suma;                        // devolver valor de la suma de $f$
}

// calculo de la integral de forma concurrente entrelazada
double calcular_integral_concurrente_entrelazada(){
    future<double> futuros[n];      // declaración del vector de future
    double suma = 0.0;              // inicializar suma

    for (long i = 0; i < n; i++)    // para cada $i$ entre $0$ y $n-1$:
        futuros[i] = async(launch::async, funcion_hebra_entrelazada, i);    

    for (long i = 0; i < n; i++)    // para cada $i$ entre $0$ y $n-1$:
        suma += futuros[i].get();   // añadir la suma de cada valor del vector

    return suma/m;                  // devolver valor promedio
}
```

En la función `calcular_integral_concurrente_entrelazada`, hago exactamente lo mismo que en la función `calcular_integral_concurrente_contigua` excepto que llamo a la función `funcion_hebra_entrelazada` dentro del primer bucle `for`.

La función `funcion_hebra_entrelazada` recibe el parámetro `i` que se corresponde con el índice de la hebra. Hay un bucle `for` en el que a cada `j` le corresponde un valor entre `i` y `m - 1`, avanzando en `j + n` unidades. Esto se debe a que a la hebra con índice `i` empieza en el punto `i` y el siguiente que le corresponde sería `j + n`, ya que los que hay entremedias les corresponden a las otras hebras. Dentro de este bucle calculo la suma llamando a la función `f` y devuelvo la suma total de cada hebra.

### Ejecución del programa

El programa devuelve los siguientes resultados:

```
Número de muestras (m)              : 1073741824
Número de hebras (n)                : 4
Valor de PI                         : 3.14159265358979312
Resultado secuencial                : 3.14159265358998185
Resultado concurrente contiguo      : 3.14159265358982731
Resultado concurrente entrelazado   : 3.14159265358978601
Tiempo secuencial                   : 15275 milisegundos.
Tiempo concurrente contiguo         : 4072.8 milisegundos.
Tiempo concurrente entrelazado      : 4118.6 milisegundos.
Porcentaje t.conc_contiguo/t.sec.   : 26.66%
Porcentaje t.conc_entrelazado/t.sec : 26.96%
```

Como habíamos visto en el seminario, el cálculo concurrente (tanto de forma contigua como entrelazada) tarda un poco más de la cuarta parte que el secuencial.
