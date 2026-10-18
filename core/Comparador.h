#ifndef COMPARADOR_H
#define COMPARADOR_H

#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>

#include "../struct/ArregloDinamico.h"
#include "FabricaPlanificadores.h"
#include "Planificador.h"
#include "Proceso.h"
#include "Simulador.h"

/**
 * El modo simulación toma el resultado de cada algoritmo y lo devuelve en un arreglo,
 * esto pemite comparar las metricas de cada uno como tiempo de retorno, uso de CPU y demás sin multiplicar el uso de memoria.
 */
struct ResultadoAlgoritmo {
    Algoritmo algoritmo = Algoritmo::FCFS;
    std::string nombre;                 // nombre completo, p. ej. "Round Robin (quantum = 3)"
    int tiempoTotal = 0;
    int tiempoOcupado = 0;
    double utilizacionCPU = 0.0;
    double esperaPromedio = 0.0;
    double retornoPromedio = 0.0;
    double respuestaPromedio = 0.0;
    int esperaMaxima = 0;
};

/**
 * Ejecuta cada algoritmo sobre una copia del mismo conjunto de procesos y devuelve
 * un resultado por algoritmo, en el orden pedido.
 *
 * El quantum es único: lo usan los algoritmos que lo necesitan (Round Robin) y
 * los demás lo ignoran. Sirve para comparar cualquier subconjunto de algoritmos,
 * dos o los que haya.
 *
 * Se ejecuta de uno en uno y no se conserva el Simulador, así que la memoria
 * máxima es la de una sola simulación. El Gantt no se registra.
 *
 * Lanza std::invalid_argument si no se elige ningún algoritmo, si hay uno repetido,
 * o si el quantum falta o no es válido cuando Round Robin participa. Esas
 * comprobaciones ocurren ANTES de simular nada.
 */
inline ArregloDinamico<ResultadoAlgoritmo> compararAlgoritmos(
    const ArregloDinamico<Proceso>& procesos, const ArregloDinamico<Algoritmo>& algoritmos,
    std::optional<int> quantum = std::nullopt) {
    if (algoritmos.estaVacio()) {
        throw std::invalid_argument("compararAlgoritmos: hay que elegir al menos un algoritmo");
    }
    for (std::size_t i = 0; i < algoritmos.tamano(); ++i) {
        for (std::size_t j = i + 1; j < algoritmos.tamano(); ++j) {
            if (algoritmos[i] == algoritmos[j]) {
                throw std::invalid_argument("compararAlgoritmos: algoritmo repetido: " +
                                            nombre(algoritmos[i]));
            }
        }
    }

    // Se crean todos los planificadores primero: si el quantum no sirve, falla antes de gastar tiempo simulando.
    ArregloDinamico<std::unique_ptr<Planificador>> planificadores(algoritmos.tamano());
    for (const Algoritmo algoritmo : algoritmos) {
        planificadores.agregar(crearPlanificador(algoritmo, quantum));
    }

    ArregloDinamico<ResultadoAlgoritmo> resultados(algoritmos.tamano());
    for (std::size_t i = 0; i < algoritmos.tamano(); ++i) {
        Simulador simulador(procesos);          // copia: cada algoritmo parte de cero
        simulador.registrarGantt(false);
        simulador.ejecutar(*planificadores[i]);

        ResultadoAlgoritmo r;
        r.algoritmo = algoritmos[i];
        r.nombre = planificadores[i]->nombre();
        r.tiempoTotal = simulador.tiempoTotal();
        r.tiempoOcupado = simulador.tiempoOcupado();
        r.utilizacionCPU = simulador.utilizacionCPU();
        r.esperaPromedio = simulador.esperaPromedio();
        r.retornoPromedio = simulador.retornoPromedio();
        r.respuestaPromedio = simulador.respuestaPromedio();
        r.esperaMaxima = simulador.esperaMaxima();
        resultados.agregar(std::move(r));
    }
    return resultados;
}

#endif 
