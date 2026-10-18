#ifndef PLANIFICADOR_H
#define PLANIFICADOR_H

#include <cstddef>
#include <string>

#include "Proceso.h"

/**
 * Planificador (clase abstracta)
 *
 * Define lo que el Simulador necesita de cualquier algoritmo. Los
 * planificadores guardan índices a la tabla de procesos, así el
 * estado de cada proceso vive en un solo lugar.
 *
 * cada algoritmo nuevo hereda de esta clase.
 */
class Planificador {
public:
    virtual ~Planificador() = default;

    // Nombre para mostrar en pantalla.
    virtual std::string nombre() const = 0;

    // Un proceso queda listo: acaba de llegar o fue expulsado de la CPU.
    virtual void agregar(std::size_t indiceProceso, const Proceso& proceso) = 0;

    virtual bool hayListos() const = 0;

    // Saca de la cola de listos el proceso que usará la CPU ahora.
    virtual std::size_t siguiente() = 0;

    // Máximo tiempo seguido que el proceso puede usar la CPU antes de ser expulsado. 
    virtual int quantum(const Proceso& p) const = 0;
};

#endif 
