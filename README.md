# SchedSim

**Simulador didáctico de planificación de procesos en C++.**

SchedSim modela cómo un sistema operativo reparte la CPU entre varios procesos y permite **comparar algoritmos de planificación** a partir de métricas de rendimiento. Las estructuras de datos lineales se implementan en el propio proyecto, como plantillas de C++.

> **Estado:** en desarrollo, versión 1 (interfaz de consola).

---

## Tabla de contenido

- [Objetivo](#objetivo)
- [Características de la versión 1](#características-de-la-versión-1)
- [Estructuras de datos](#estructuras-de-datos)
- [Algoritmos de planificación](#algoritmos-de-planificación)
- [Estructura del proyecto](#estructura-del-proyecto)
- [Requisitos](#requisitos)
- [Compilación y ejecución](#compilación-y-ejecución)
- [Uso](#uso)
- [Métricas](#métricas)
- [Hoja de ruta](#hoja-de-ruta)
- [Control de versiones](#control-de-versiones)
- [Licencia](#licencia)
- [Autoría](#autoría)

---

## Objetivo

Ofrecer una herramienta para **aprender y experimentar** con la gestión de la CPU:

- Entender cómo funcionan FCFS, Round Robin y la planificación por prioridad.
- Probar distintos escenarios: muchos procesos cortos, un proceso muy largo, prioridades desbalanceadas.
- Evaluar con datos qué algoritmo rinde mejor para una carga de trabajo determinada.

## Características de la versión 1

- [ ] Modelo de proceso (PID, nombre, llegada, ráfaga de CPU, prioridad, tiempo restante y estado).
- [ ] Reloj de simulación por unidades de tiempo (*ticks*).
- [ ] Algoritmos: **FCFS**, **Prioridad** y **Round Robin** (quantum configurable).
- [ ] Carga de procesos: manual, desde archivo y aleatoria.
- [ ] Ejecución completa o paso a paso.
- [ ] Diagrama de Gantt en texto.
- [ ] Métricas: espera, retorno, respuesta y uso de CPU.
- [ ] Modo de comparación: los tres algoritmos sobre el mismo conjunto de procesos.

## Estructuras de datos

| Estructura | Papel en SchedSim |
|---|---|
| **Cola con prioridad** | Cola de listos del algoritmo por prioridad (heap binario). |
| **Cola circular** | Cola de listos de Round Robin: el proceso que agota su quantum vuelve al final. |
| **Arreglo dinámico** | Tabla de procesos, registro de resultados y almacenamiento interno del heap. |
| Cola simple *(apoyo)* | FCFS y cola de procesos que aún no han llegado. |
| Pila *(apoyo)* | Historial de eventos y deshacer en el modo paso a paso. |

## Algoritmos de planificación

| Algoritmo | Estructura principal |
|---|---|
| FCFS (primero en llegar, primero en ser atendido) | Cola |
| Prioridad | Cola con prioridad |
| Round Robin | Cola circular |

## Estructura del proyecto

```
SchedSim/
├── CMakeLists.txt
├── README.md
├── .gitignore
├── main.cpp
├── struct/
│   ├── ArregloDinamico.h
│   ├── Cola.h
│   ├── ColaCircular.h
│   ├── ColaPrioridad.h
│   └── Pila.h
├── core/
│   ├── Proceso.h
│   ├── Planificador.h          # clase abstracta
│   ├── PlanificadorFCFS.h
│   ├── PlanificadorPrioridad.h
│   ├── PlanificadorRR.h
│   └── Simulador.h
├── ui/
│   └── MenuConsola.cpp
├── pruebas/                    # pruebas de cada estructura y algoritmo
├── datos/                      # archivos de ejemplo con procesos
└── docs/                       # documentación y diagramas
```

La separación en tres capas (`struct`, `core`, `ui`) permite cambiar la interfaz o añadir estructuras y algoritmos sin reescribir el resto.

## Requisitos

- Compilador con soporte de **C++17** o superior (GCC, Clang o MSVC).
- **CMake** 3.16 o superior.
- **Git**.

## Compilación y ejecución

```bash
git clone https://github.com/SantiagoPelaez28/SchedSim.git
cd SchedSim

cmake -S . -B build
cmake --build build

./build/schedsim        # en Windows: build\Debug\schedsim.exe
```

## Uso

Al iniciar, SchedSim muestra un menú de consola (ejemplo ilustrativo):

```
=== SchedSim ===
1. Cargar procesos
2. Elegir algoritmo
3. Ejecutar simulación
4. Ejecutar paso a paso
5. Ver diagrama de Gantt y métricas
6. Comparar algoritmos
0. Salir
```

### Formato de archivo de procesos (propuesto)

```csv
pid,nombre,llegada,rafaga,prioridad
1,P1,0,5,2
2,P2,1,3,1
3,P3,2,8,3
```

### Ejemplo ilustrativo de Gantt

```
Tiempo: 0    1    2    3    4    5
        | P1 | P1 | P2 | P2 | P3 |
```

## Métricas

- **Tiempo de espera:** tiempo que un proceso pasa en la cola de listos.
- **Tiempo de retorno:** desde que llega hasta que termina.
- **Tiempo de respuesta:** desde que llega hasta que obtiene la CPU por primera vez.
- **Uso de CPU:** porcentaje del tiempo total en que la CPU estuvo ocupada.

## Hoja de ruta

| Versión | Enfoque |
|---|---|
| **V1** | Interfaz de consola con estructuras lineales: arreglo dinámico, cola, cola circular, cola con prioridad y pila. |
| **V2 en adelante** | Nuevas estructuras de datos y nuevas funcionalidades *(por definir)*. |
| **Futuro** | Interfaz gráfica de usuario. |

## Autoría

- **Santiago Peláez**: desarrollo y diseño.
