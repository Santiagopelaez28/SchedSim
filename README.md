# SchedSim

**Simulador didáctico de planificación de procesos de CPU, en C++17 y con interfaz de consola.**

SchedSim reproduce cómo un sistema operativo reparte la CPU entre varios procesos, permite **comparar algoritmos de planificación** sobre exactamente los mismos procesos y mide cada resultado con métricas de rendimiento. Todas las estructuras de datos del proyecto (arreglo dinámico, colas, cola circular, cola con prioridad y pila) están implementadas desde cero, como plantillas de C++, sin usar los contenedores de la biblioteca estándar.

> **Versión:** 1.0 (interfaz de consola).

---

## Contenido

- [Introducción](#introducción)
- [Estructuras de datos y dónde se usan](#estructuras-de-datos-y-dónde-se-usan)
- [Estructura del proyecto](#estructura-del-proyecto)
- [Cómo compilar](#cómo-compilar)
- [Manual de usuario](#manual-de-usuario)
- [Pruebas](#pruebas)
- [Limitaciones de la versión 1](#limitaciones-de-la-versión-1)
- [Hoja de ruta](#hoja-de-ruta)
- [Control de versiones](#control-de-versiones)
- [Licencia y autoría](#licencia-y-autoría)

---

## Introducción

### ¿Qué problema resuelve?

En un sistema operativo hay casi siempre más procesos que núcleos de CPU. El **planificador** decide cuál usa la CPU en cada momento, y esa decisión cambia cuánto esperan los procesos, qué tan rápido responde el sistema y qué tan justo es el reparto. Entender esas diferencias solo con teoría es difícil.

SchedSim convierte esas decisiones en algo que se puede ver y medir: el usuario elige un algoritmo (o varios para compararlos), la cantidad de procesos y, si quiere, una semilla para repetir el escenario; el programa genera los procesos, simula la ejecución y muestra el diagrama de Gantt y las métricas.

### Qué hace

- Genera **procesos aleatorios** reproducibles (semilla) con llegada, ráfaga de CPU y prioridad.
- Simula tres algoritmos de planificación:

  | Algoritmo | Idea |
  |---|---|
  | **FCFS** (primero en llegar, primero en ser atendido) | Se atiende por orden de llegada y cada proceso conserva la CPU hasta terminar. |
  | **Round Robin** | Cada proceso usa la CPU como máximo un *quantum*; si no termina, vuelve al final de la cola. El quantum lo elige el usuario. |
  | **Prioridad** (sin expulsión) | Cuando la CPU se libera, entra el proceso más urgente entre los que ya llegaron (**1 es la prioridad más urgente**). Los empates se resuelven por orden de llegada. |

- Muestra el **diagrama de Gantt** y, por proceso, los tiempos de **espera**, **retorno** y **respuesta**, además del **uso de CPU** y la **espera máxima**.
- **Compara** dos o más algoritmos sobre el mismo conjunto de procesos, con un único quantum, y marca el mejor valor de cada métrica.
- Funciona con **cantidades grandes** (hasta 1 000 000 de procesos): con muchos procesos muestra un resumen en lugar de miles de filas.

### Diseño

El código se divide en tres capas, todas de solo cabeceras:

- `struct/`: estructuras de datos propias, sin conocer nada de procesos.
- `core/`: el modelo (procesos, planificadores) y el simulador, sin entrada ni salida de usuario.
- `gui/`: la interfaz de consola (menú, lectura validada de datos y formato de resultados).

Todos los algoritmos heredan de una interfaz común (`Planificador`) y el menú solo conoce un catálogo (`FabricaPlanificadores.h`). Por eso el proyecto es **incremental**: agregar un algoritmo, una estructura o una interfaz nueva no obliga a reescribir el resto.

---

## Estructuras de datos y dónde se usan

| Estructura | Archivo | Uso en SchedSim |
|---|---|---|
| **Arreglo dinámico** | `struct/ArregloDinamico.h` | Tabla de procesos, tramos del Gantt, resultados de la comparación y almacenamiento interno del heap. |
| **Cola simple** | `struct/Cola.h` | Cola de procesos listos de **FCFS**. |
| **Cola circular** (con nodos enlazados) | `struct/ColaCircularEnlazada.h` | Cola de procesos listos de **Round Robin**: el proceso que agota su quantum vuelve al final. |
| **Cola con prioridad** (heap binario sobre el arreglo dinámico) | `struct/ColaPrioridad.h` | Cola de listos del algoritmo de **Prioridad** y cola de procesos que aún no han llegado, ordenada por llegada. |
| Pila | `struct/Pilas.h` | Implementada y probada, **todavía no se usa** en el simulador (está prevista para el modo paso a paso). |
| Cola circular (con arreglo) | `struct/ColaCircular.h` | Implementada y probada como alternativa a la versión enlazada, **no se usa** en la versión 1. |

---

## Estructura del proyecto

```
SchedSim/
├── CMakeLists.txt
├── README.md
├── .gitignore
├── main.cpp                       punto de entrada: crea y ejecuta el menú
├── struct/                        estructuras de datos propias (plantillas)
│   ├── ArregloDinamico.h
│   ├── Cola.h
│   ├── ColaCircular.h
│   ├── ColaCircularEnlazada.h
│   ├── ColaPrioridad.h
│   └── Pilas.h
├── core/                          modelo y simulación
│   ├── Proceso.h
│   ├── GeneradorProcesos.h        procesos aleatorios con semilla
│   ├── Planificador.h             interfaz común de los algoritmos
│   ├── PlanificadorFCFS.h
│   ├── PlanificadorRR.h
│   ├── PlanificadorPrioridad.h
│   ├── FabricaPlanificadores.h    catálogo de algoritmos y fábrica
│   ├── Simulador.h                reloj, ejecución, Gantt y métricas
│   └── Comparador.h               ejecuta varios algoritmos sobre los mismos procesos
├── gui/                           interfaz de consola
│   ├── MenuConsola.h
│   ├── Entrada.h                  lectura validada de datos
│   ├── Salida.h                   qué mostrar según la cantidad de procesos
│   └── Comparacion.h              tabla comparativa
└── pruebas/                       16 archivos prueba_*.cpp
```

### Cómo agregar un algoritmo nuevo

1. Crear `core/PlanificadorXxx.h` con una clase que herede de `Planificador` (`agregar`, `hayListos`, `siguiente`, `quantum`, `nombre`).
2. Añadir un valor al `enum Algoritmo` de `core/FabricaPlanificadores.h` y un caso en cada `switch` (el compilador avisa si falta alguno).
3. Agregar su archivo de pruebas en `pruebas/`.

El menú y la comparación lo reconocen sin más cambios.

---

## Cómo compilar

### Requisitos

- Un compilador con soporte de **C++17**. El proyecto se probó con **GCC 13** en Linux.
- **CMake 3.16 o superior** (opcional, pero recomendado). Se probó con CMake 4.4.
- Git, para clonar el repositorio.

### Con CMake (recomendado)

```bash
git clone https://github.com/<usuario>/SchedSim.git
cd SchedSim

cmake -S . -B build
cmake --build build
```

El ejecutable queda en `build/schedsim` (en Windows con MinGW o Ninja, `build\schedsim.exe`; con Visual Studio, compila con `cmake --build build --config Release` y queda en `build\Release\schedsim.exe`).

```bash
./build/schedsim
```

Si no indicas otra cosa, CMake compila en modo `Release` (optimizado).

### Con un solo comando, sin CMake

Todo el código es de solo cabeceras, así que basta con compilar `main.cpp`:

```bash
g++ -std=c++17 -O2 main.cpp -o schedsim
./schedsim
```

### Opciones de CMake

| Opción | Efecto |
|---|---|
| `-DSCHEDSIM_TESTS=OFF` | No compila las pruebas (por defecto están activadas). |
| `-DSCHEDSIM_SANITIZERS=ON` | Compila con AddressSanitizer y UBSan para detectar errores de memoria. Solo con GCC o Clang en Linux o macOS. Conviene combinarla con `-DCMAKE_BUILD_TYPE=Debug`. |

Ejemplo con sanitizadores:

```bash
cmake -S . -B build-san -DSCHEDSIM_SANITIZERS=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build build-san
```

> Solo he podido verificar la compilación con GCC en Linux. En Windows (MinGW o Visual Studio) las instrucciones anteriores deberían funcionar, pero conviene comprobarlas en tu equipo.

---

## Manual de usuario

Los ejemplos de esta sección son salidas reales del programa, generadas con la semilla `42`. **Con la misma semilla, los procesos pueden variar entre compiladores** (la biblioteca estándar no fija el algoritmo de las distribuciones aleatorias), así que en tu equipo las tablas pueden no coincidir exactamente con las de este manual.

### Iniciar el programa

```bash
./build/schedsim
```

Aparece el menú principal:

```
=== SchedSim ===
Seleccione una opcion:
  1. FCFS - primero en llegar, primero en ser atendido
  2. Round Robin - turnos de duracion fija (quantum) [quantum actual = 3]
  3. Prioridad - sin expulsion; 1 = la mas urgente
  4. Comparar algoritmos
  0. Salir de SchedSim
Opcion (entero entre 0 y 4):
```

Escribe el número de la opción y pulsa Enter. Al terminar cada acción, el programa vuelve al menú hasta que elijas **0**.

### Datos que pide el programa

Según la opción, el programa pregunta, en este orden:

| Dato | Cuándo | Reglas |
|---|---|---|
| **Quantum** | Solo si se usa Round Robin | Entero mayor o igual que 1. Enter conserva el último valor (3 la primera vez). |
| **Cantidad de procesos** | Siempre | Entero entre 1 y 1 000 000. Enter conserva el último valor (30 la primera vez). |
| **Semilla** | Siempre | Entero entre 0 y 4 294 967 295. Enter elige una semilla **aleatoria**. |

El valor por defecto aparece entre corchetes: `[Enter = 3]`. Si escribes algo que no sirve, el programa lo explica y **repite la pregunta**; nunca se cierra por un dato mal escrito.

### Ejecutar un algoritmo (opciones 1 a 3)

Ejemplo: FCFS con 5 procesos y semilla 42.

```
Opcion (entero entre 0 y 4): 1

--- FCFS ---
Cantidad de procesos (entero entre 1 y 1000000) [Enter = 30]: 5
Semilla (entero sin signo; Enter = aleatoria): 42

SchedSim - semilla: 42 (escribela para repetir este escenario)

Procesos generados (5; prioridad: 1 = la mas urgente)
PID  Nombre      Llegada  Rafaga  Prioridad
1    P1                4       7          5
2    P2                2       6          4
3    P3                6       5          1
4    P4                4       2          1
5    P5                0       4          5

=== FCFS (primero en llegar, primero en ser atendido) ===

Diagrama de Gantt
| P5 | P2 | P1 | P4 | P3 |
0    4    10   17   19   24

Resultados por proceso
PID  Nombre      Llegada  Rafaga  Inicio   Fin  Retorno  Espera  Respuesta
1    P1                4       7      10    17       13       6          6
2    P2                2       6       4    10        8       2          2
3    P3                6       5      19    24       18      13         13
4    P4                4       2      17    19       15      13         13
5    P5                0       4       0     4        4       0          0

Procesos:                     5
Tiempo total:                24
CPU ocupada:                 24
Uso de CPU:              100.00 %
Espera promedio:           6.80
Retorno promedio:         11.60
Respuesta promedio:        6.80
Espera maxima:               13
```

**La semilla se muestra siempre** al final de la ejecución: escríbela la próxima vez para repetir el mismo escenario.

#### Cómo leer los resultados

- **Procesos generados:** los datos de entrada. La *llegada* es el instante en que el proceso entra al sistema, la *ráfaga* es el tiempo de CPU que necesita y la *prioridad* solo la usa el algoritmo de Prioridad (1 es la más urgente).
- **Diagrama de Gantt:** una línea con los procesos en el orden en que usaron la CPU y otra con los instantes en que empieza cada tramo; el último número es el instante final. Un tramo `--` indica que la CPU estuvo ociosa porque aún no había llegado ningún proceso. Si el diagrama no cabe en 80 columnas, se parte en varios bloques.
- **Resultados por proceso:**

  | Métrica | Significado | Cálculo |
  |---|---|---|
  | Inicio | Primer instante en que el proceso usa la CPU | |
  | Fin | Instante en que termina | |
  | **Retorno** | Tiempo total desde que llega hasta que termina | Fin − Llegada |
  | **Espera** | Tiempo que pasa esperando en la cola de listos | Retorno − Ráfaga |
  | **Respuesta** | Tiempo hasta que obtiene la CPU por primera vez | Inicio − Llegada |

- **Resumen:** tiempo total de la simulación, tiempo de CPU ocupada, **uso de CPU** (porcentaje del tiempo total, contado desde 0, en que la CPU estuvo ocupada), promedios de espera, retorno y respuesta, y la **espera máxima** (el peor caso, que el promedio puede ocultar).

#### Round Robin y el quantum

Con la opción 2 el programa pregunta el quantum primero. Si escribes algo inválido, vuelve a preguntar:

```
--- Round Robin ---
Quantum de Round Robin (entero mayor o igual que 1) [Enter = 3]: abc
  Valor invalido: escribe un entero mayor o igual que 1.
Quantum de Round Robin (entero mayor o igual que 1) [Enter = 3]: 4
```

Con los mismos 5 procesos y quantum 4, el diagrama muestra cómo los procesos se turnan:

```
| P5 | P2 | P1 | P4 | P3 | P2 | P1 | P3 |
0    4    8    12   14   18   20   23   24
```

Si el quantum es mayor o igual que la ráfaga más larga, Round Robin se comporta exactamente como FCFS, y el programa lo avisa.

### Comparar algoritmos (opción 4)

La comparación ejecuta cada algoritmo sobre **el mismo conjunto de procesos**, así las diferencias se deben solo al algoritmo.

1. Elige los algoritmos escribiendo sus números separados por espacios o comas (por ejemplo `1 3`). Con solo Enter se eligen todos. Hay que elegir al menos dos.
2. Si entre ellos está Round Robin, el programa pide el quantum **una sola vez**; todos los algoritmos que lo usan comparten ese valor.
3. Indica la cantidad de procesos y la semilla.

Ejemplo: comparar FCFS y Prioridad con 5 procesos y semilla 42.

```
--- Comparar algoritmos ---
Algoritmos disponibles:
  1. FCFS
  2. Round Robin
  3. Prioridad
Cuales elegir (numeros separados por espacios o comas; Enter = todos): 1 3
Cantidad de procesos (entero entre 1 y 1000000) [Enter = 30]: 5
Semilla (entero sin signo; Enter = aleatoria): 42

Comparacion de algoritmos

Metrica                 FCFS   Prioridad
Tiempo total              24          24
Uso de CPU (%)        100.00      100.00
Espera promedio         6.80        4.40*
Retorno promedio       11.60        9.20*
Respuesta promedio      6.80        4.40*
Espera maxima             13          13

* = mejor valor de la fila (menor es mejor)
```

- El **`*`** marca el mejor valor (el menor) de cada métrica de espera, retorno, respuesta y espera máxima. Si hay empate, se marcan todos; si todos los valores son iguales, no se marca ninguno.
- **El tiempo total y el uso de CPU son iguales para todos los algoritmos.** Es normal: esta versión no modela el costo de cambiar de un proceso a otro, así que el algoritmo solo cambia el orden en que se atiende a los procesos, no cuánto trabajo hace la CPU.
- Ningún algoritmo gana en todo: por ejemplo, Round Robin suele tener la mejor respuesta pero no la mejor espera promedio, y Prioridad puede hacer esperar mucho a los procesos poco urgentes (inanición).

### Muchos procesos

Hasta **30 procesos** se muestra todo el detalle. Con más, el programa muestra un **resumen** (totales y promedios) y solo las primeras 10 filas; el diagrama de Gantt no se registra, lo que además ahorra memoria. Así se ve con 100 000 procesos (extracto):

```
Procesos generados (100000; prioridad: 1 = la mas urgente)
PID  Nombre      Llegada  Rafaga  Prioridad
1    P1                0       2          4
...
10   P10               7       3          4
... se omiten 99990 procesos mas (se muestran 10 de 100000)
=== FCFS (primero en llegar, primero en ser atendido) ===
Resumen (con mas de 30 procesos se omiten el diagrama de Gantt y el detalle completo)
...
Procesos:                100000
Tiempo total:            450375
Uso de CPU:              100.00 %
Espera promedio:      225393.81
```

En las pruebas con 1 000 000 de procesos, cada algoritmo tardó menos de un segundo y el pico de memoria fue de unos 200 MB (compilado con optimización). Con tantos procesos llegando casi a la vez, las esperas son enormes: es lo esperable, porque todos compiten por una sola CPU.

### Errores y mensajes

| Situación | Qué hace el programa |
|---|---|
| Opción o dato inválido (texto, fuera de rango, vacío cuando no hay valor por defecto) | Explica el error y repite la pregunta. |
| Selección de algoritmos con menos de dos distintos | Avisa y vuelve a preguntar. |
| Un error durante la simulación | Muestra `Error: ...` y vuelve al menú. |
| La entrada se cierra (por ejemplo, Ctrl+D o el fin de un archivo redirigido) | Termina de forma ordenada con `Entrada cerrada. Saliendo de SchedSim.` |

### Usar el programa con un guion

Como el programa lee de la entrada estándar, se puede automatizar. Por ejemplo, comparar los tres algoritmos con quantum 3, 100 000 procesos y semilla 7, y salir:

```bash
printf '4\n\n3\n100000\n7\n0\n' | ./build/schedsim
```

---

## Pruebas

Hay 16 archivos de pruebas en `pruebas/`: uno por cada estructura de datos y por cada componente (generador, algoritmos, simulador con cantidades grandes, salida, comparador, entrada y menú). Los algoritmos se comprueban con resultados **calculados a mano**, y el menú con sesiones de teclado simuladas.

Con CMake:

```bash
cd build
ctest --output-on-failure
```

(Con Visual Studio u otro generador de varias configuraciones: `ctest -C Release --output-on-failure`.)

Una prueba suelta, sin CMake:

```bash
g++ -std=c++17 -O2 pruebas/prueba_fcfs.cpp -o prueba_fcfs
./prueba_fcfs
```

Para detectar errores de memoria, compila con `-DSCHEDSIM_SANITIZERS=ON` (ver [Opciones de CMake](#opciones-de-cmake)) o a mano con `-fsanitize=address,undefined`.

---

## Limitaciones de la versión 1

- Una sola CPU, sin entrada/salida de los procesos y **sin costo de cambio de contexto**.
- Los procesos son **solo aleatorios** (sin carga manual ni desde archivo).
- La prioridad no tiene expulsión ni envejecimiento: un proceso poco urgente puede esperar mucho (inanición).
- Los tiempos son enteros y deben caber en `int` (el programa lo valida).
- La reproducibilidad por semilla es exacta con el mismo compilador, pero puede variar entre compiladores.

## Hoja de ruta

| Versión | Enfoque |
|---|---|
| **1.0** | Interfaz de consola con arreglo dinámico, cola, cola circular y cola con prioridad; algoritmos FCFS, Round Robin y Prioridad sin expulsión; modo de comparación. |
| **Próximo** | Modo **paso a paso** con la pila (avanzar y deshacer). |
| **Siguientes** | Costo de cambio de contexto, envejecimiento de prioridades, nuevas estructuras de datos (listas enlazadas, tablas hash, árboles, grafos), carga de procesos desde archivo e interfaz gráfica. |

## Control de versiones

- Rama `main`: versión estable.
- Ramas de trabajo: `feature/<nombre>` para cada funcionalidad nueva.
- Cada versión se marca con una etiqueta: `v1.0`, `v2.0`, etc.

## Licencia y autoría

- **Licencia:** por definir (MIT es una opción sencilla para proyectos académicos y abiertos).
- **Autoría:** [Tu nombre].
