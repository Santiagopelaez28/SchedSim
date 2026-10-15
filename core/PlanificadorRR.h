#ifndef PLANIFICADOR_RR_H
#define PLANIFICADOR_RR_H

#include <cstddef>
#include <stdexcept>
#include <string>

#include "../struct/ColaCircularEnlazada.h"
#include "Planificador.h"

/**
 * PlanificadorRR: Round Robin.
 *
 * La cola de listos es una cola circular: cada proceso usa la CPU como máximo
 * "quantum" unidades de tiempo y, si no terminó, vuelve al final de la cola.
 */
class PlanificadorRR : public Planificador {
public:
    explicit PlanificadorRR(int quantum) : quantum_(quantum) {
        if (quantum <= 0) {
            throw std::invalid_argument("PlanificadorRR: el quantum debe ser mayor que 0");
        }
    }

    std::string nombre() const override {
        return "Round Robin (quantum = " + std::to_string(quantum_) + ")";
    }

    void agregar(std::size_t indiceProceso) override { listos_.insertarPrimero(indiceProceso); }

    bool hayListos() const override { return !listos_.estaVacia(); }

    std::size_t siguiente() override { return listos_.borrarPrimero(); }

    int quantum(const Proceso&) const override { return quantum_; }

    // Contenido actual de la cola de listos (índices), de solo lectura.
    const ColaCircularEnlazada<std::size_t>& listos() const { return listos_; }

private:
    int quantum_;
    ColaCircularEnlazada<std::size_t> listos_;
};

#endif // PLANIFICADOR_RR_H
