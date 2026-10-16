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
 * Catálogo de algoritmos disponibles y creación de los mismos.
 *
 * A partir de una clase enum, se puede generar un algoritmo de planificación especifico para la simulación. 
 * El programa general de simulacion utiliza esta clase para utilizar los algoritmos sin acceder directamente a sus clases concretas.
 */
enum class Algoritmo
{
    FCFS,
    RoundRobin,
    Prioridad
};

/** Nombres de los algoritmos (Se muestran en pantalla al usuario).
 * @param algoritmo El algoritmo para el cual crear el planificador.
 */
inline std::string nombre(Algoritmo algoritmo)
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
    throw std::logic_error("nombre: algoritmo desconocido");
}

/** Valida si un algoritmo depende o no de un quantum ingresado por el usuario.
 * @param algoritmo El algoritmo para el cual crear el planificador. 
 * @return Verdadero si el algoritmo depende de un quantum.
 */
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

// Orden de los algorimos mostrados en la UI
inline ArregloDinamico<Algoritmo> todosLosAlgoritmos()
{
    ArregloDinamico<Algoritmo> lista;
    lista.agregar(Algoritmo::FCFS);
    lista.agregar(Algoritmo::RoundRobin);
    lista.agregar(Algoritmo::Prioridad);
    return lista;
}

/**
 * Creación del planificador para el algoritmo pedido.
 *
 * El quantum solo se usa si usaQuantum(algoritmo) retorna verdadero; los demás lo ignoran,
 * así en el modo de comparación se puede pasar el mismo quantum a todos.
 *
 * @param algoritmo El algoritmo para el cual crear el planificador. 
 * @param quantum El quantum a usar (Si aplica) y se ignora si el algoritmo no lo requiere.
 * @return Un puntero único al planificador creado.
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
            throw std::invalid_argument("crearPlanificador: Round Robin necesita un quantum");  // Valida que el quantum sea proporcionado explicitamente por el usuario.
        }
        return std::make_unique<PlanificadorRR>(*quantum); // Valida que el quantum sea mayor a 0 en el constructor de PlanificadorRR.
    case Algoritmo::Prioridad:
        return std::make_unique<PlanificadorPrioridad>();
    }
    throw std::logic_error("crearPlanificador: algoritmo desconocido");
}

#endif 
