#ifndef FABRICA_PLANIFICADORES_H
#define FABRICA_PLANIFICADORES_H

#include <memory>
#include <optional>
#include <stdexcept>
#include <string>

#include "../struct/ArregloDinamico.h"
#include "Planificador.h"
#include "PlanificadorFCFS.h"
#include "PlanificadorPrioridad.h"
#include "PlanificadorRR.h"

/**
 * Catálogo de algoritmos disponibles y fábrica para crearlos.
 *
 * Este archivo es utilizado por el menú de consola para acceder a los diferentes algorimos
 * sin necesidad de conocer todos sus métodos y atributos
 */
enum class Algoritmo { FCFS, RoundRobin, Prioridad };

/** Nombre corto para mostrar en menús y tablas. 
 * 
 * @param algoritmo El algoritmo ingresado por el usuario
 * @returns El nombre especifico de dicho algoritmo
 */
inline std::string nombre(Algoritmo algoritmo) {
    switch (algoritmo) {
        case Algoritmo::FCFS: return "FCFS";
        case Algoritmo::RoundRobin: return "Round Robin";
        case Algoritmo::Prioridad: return "Prioridad";
    }
    throw std::logic_error("nombre: algoritmo desconocido");
}

/** Descripción de cada algoritmo para mostrar en el menú
 * 
 * @param algoritmo El algoritmo ingresado por el usuario
 * @returns Una descripción corta del algoritmo
 */
inline std::string descripcion(Algoritmo algoritmo) {
    switch (algoritmo) {
        case Algoritmo::FCFS: return "primero en llegar, primero en ser atendido";
        case Algoritmo::RoundRobin: return "turnos de duracion fija (quantum)";
        case Algoritmo::Prioridad: return "sin expulsion; 1 = la mas urgente";
    }
    throw std::logic_error("descripcion: algoritmo desconocido");
}

/** Revisa si el algoritmo necesita o no un quantum
 * @param algoritmo El algoritmo ingresado por el usuario
 * @returns True si el algoritmo requiere de quantum (Round Robin)
 */
inline bool usaQuantum(Algoritmo algoritmo) {
    switch (algoritmo) {
        case Algoritmo::FCFS: return false;
        case Algoritmo::RoundRobin: return true;
        case Algoritmo::Prioridad: return false;
    }
    throw std::logic_error("usaQuantum: algoritmo desconocido");
}

// Todos los algoritmos, en el orden en que se muestran en los menús.
inline ArregloDinamico<Algoritmo> todosLosAlgoritmos() {
    ArregloDinamico<Algoritmo> lista;
    lista.agregar(Algoritmo::FCFS);
    lista.agregar(Algoritmo::RoundRobin);
    lista.agregar(Algoritmo::Prioridad);
    return lista;
}

/**
 * Crea el planificador del algoritmo pedido.
 *
 * El quantum solo se usa si usaQuantum(algoritmo) es true; los demás lo ignoran,
 * así en el modo de comparación se puede pasar el mismo quantum a todos.
 * @param algoritmo Cualquier algoritmo pedido por el usuario
 * @param quantum Opcional, si no lo requiere el valor por defecto es NULL
 * @returns El objeto especifico del algoritmo requerido
 */
inline std::unique_ptr<Planificador> crearPlanificador(Algoritmo algoritmo,
                                                       std::optional<int> quantum = std::nullopt) {
    switch (algoritmo) {
        case Algoritmo::FCFS:
            return std::make_unique<PlanificadorFCFS>();
        case Algoritmo::RoundRobin:
            if (!quantum.has_value()) {
                throw std::invalid_argument("crearPlanificador: Round Robin necesita un quantum");
            }
            return std::make_unique<PlanificadorRR>(*quantum);   // valida que sea > 0
        case Algoritmo::Prioridad:
            return std::make_unique<PlanificadorPrioridad>();
    }
    throw std::logic_error("crearPlanificador: algoritmo desconocido");
}

#endif