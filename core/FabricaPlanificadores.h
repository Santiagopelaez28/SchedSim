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
 * El menú de consola trabaja solo con este archivo: no necesita conocer las
 * clases concretas ni qué algoritmos usan quantum. Para añadir un algoritmo
 * nuevo hay que tocar solo aquí: un valor en el enum y un caso en cada switch
 * (el compilador avisa con -Wswitch si falta alguno).
 */
enum class Algoritmo
{
    FCFS,
    RoundRobin,
    Prioridad
};

// Nombre corto para mostrar en menús y tablas.
inline std::string nombreCorto(Algoritmo algoritmo)
{
    switch (algoritmo)
    {
    case Algoritmo::FCFS:
        return "FCFS";
    case Algoritmo::RoundRobin:
        return "Round Robin";
    case Algoritmo::Prioridad:
        return "Prioridad";
    }
    throw std::logic_error("nombreCorto: algoritmo desconocido");
}

// ¿Necesita el usuario elegir un quantum para este algoritmo?
inline bool usaQuantum(Algoritmo algoritmo)
{
    switch (algoritmo)
    {
    case Algoritmo::FCFS:
        return false;
    case Algoritmo::RoundRobin:
        return true;
    case Algoritmo::Prioridad:
        return false;
    }
    throw std::logic_error("usaQuantum: algoritmo desconocido");
}

//* Todos los algoritmos, en el orden en que se muestran en los menús.
inline ArregloDinamico<Algoritmo> todosLosAlgoritmos()
{
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
 *
 * Lanza std::invalid_argument si Round Robin no recibe quantum o si no es > 0.
 */
inline std::unique_ptr<Planificador> crearPlanificador(Algoritmo algoritmo,std::optional<int> quantum = std::nullopt)
{
    switch (algoritmo)
    {
    case Algoritmo::FCFS:
        return std::make_unique<PlanificadorFCFS>();
    case Algoritmo::RoundRobin:
        if (!quantum.has_value())
        {
            throw std::invalid_argument("crearPlanificador: Round Robin necesita un quantum");
        }
        return std::make_unique<PlanificadorRR>(*quantum); // valida que sea > 0
    case Algoritmo::Prioridad:
        return std::make_unique<PlanificadorPrioridad>();
    }
    throw std::logic_error("crearPlanificador: algoritmo desconocido");
}

#endif // FABRICA_PLANIFICADORES_H
