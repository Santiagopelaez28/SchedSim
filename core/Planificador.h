#ifndef PLANIFICADOR_H
#define PLANIFICADOR_H

#include <cstddef>
#include <string>

#include "Proceso.h"

/**
 * Planificador (clase abstracta)
 *
 * Define lo que el Simulador necesita de cualquier algoritmo. Los
 * planificadores guardan ÍNDICES a la tabla de procesos (no copias), así el
 * estado de cada proceso vive en un solo lugar.
 *
 * Para añadir un algoritmo nuevo (FCFS, Prioridad...) basta con heredar de
 * esta clase; el Simulador no cambia.
 */
class Planificador {
public:
    virtual ~Planificador() = default;

    // Nombre para mostrar en pantalla, por ejemplo "Round Robin (quantum = 3)".
    virtual std::string nombre() const = 0;

    // Un proceso queda listo: acaba de llegar o fue expulsado de la CPU.
    virtual void agregar(std::size_t indiceProceso) = 0;

    virtual bool hayListos() const = 0;

    // Saca de la cola de listos el proceso que usará la CPU ahora.
    virtual std::size_t siguiente() = 0;

    // Máximo tiempo seguido que el proceso puede usar la CPU antes de ser
    // expulsado. Los algoritmos sin expulsión devuelven p.restante.
    virtual int quantum(const Proceso& p) const = 0;
};

#endif // PLANIFICADOR_H
