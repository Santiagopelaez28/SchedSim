#ifndef PLANIFICADOR_PRIORIDAD_H
#define PLANIFICADOR_PRIORIDAD_H

#include <cstddef>
#include <string>

#include "../struct/ColaPrioridad.h"
#include "Planificador.h"
#include "Proceso.h"

/**
 * Lo que el min-heap guarda de cada proceso listo son los datos para ordenarlo y el
 * índice a la tabla de procesos.
 */
struct EntradaListo {
    int prioridad = 0;
    int llegada = 0;
    std::size_t indice = 0;

    EntradaListo() = default;
    EntradaListo(int prioridad_, int llegada_, std::size_t indice_)
        : prioridad(prioridad_), llegada(llegada_), indice(indice_) {}
};

/**
 * Criterio de orden: un proceso sale antes que otro si
 *   - Tiene menor número de prioridad (1 es la más urgente).
 *   - Si dos o más procesos tienen la misma prioridad, se atiende el que llegó ANTES.
 *   - Si ambos criterios son iguales, se toma primero al que está antes en la tabla de procesos.
 */
struct PrecedeEntradaListo {
    bool operator()(const EntradaListo& a, const EntradaListo& b) const {
        if (a.prioridad != b.prioridad) return a.prioridad < b.prioridad;
        if (a.llegada != b.llegada) return a.llegada < b.llegada;
        return a.indice < b.indice;
    }
};

/**
 * PlanificadorPrioridad: prioridad sin expulsión (no apropiativo).
 *
 * Cada vez que la CPU queda libre se elige, entre los procesos ya llegados, el
 * de mayor prioridad. Una vez que un proceso toma la CPU la conserva hasta
 * terminar, aunque llegue otro más urgente: ese
 * esperará a que la CPU se libere.
 *
 * Problema: Un proceso de baja prioridad puede
 * esperar indefinidamente si siguen llegando procesos más urgentes.
 */
class PlanificadorPrioridad : public Planificador {
public:
    std::string nombre() const override { return "Prioridad (sin expulsion; 1 = la mas urgente)"; }

    void agregar(std::size_t indiceProceso, const Proceso& proceso) {
        listos_.insertarUltimo(EntradaListo(proceso.prioridad, proceso.llegada, indiceProceso));
    }

    bool hayListos() const override { return !listos_.estaVacia(); }

    std::size_t siguiente() override { return listos_.borrarPrimero().indice; }

    // Sin expulsión: usa la CPU todo lo que le falta.
    int quantum(const Proceso& p) const override { return p.restante; }

    // Próximo proceso que saldría (solo lectura). Útil para mostrar la cola de listos.
    const EntradaListo& proximo() const { return listos_.primero(); }
    std::size_t cantidadListos() const { return listos_.tamano(); }

private:
    ColaPrioridad<EntradaListo, PrecedeEntradaListo> listos_;
};

#endif 
