#ifndef PLANIFICADOR_FCFS_H
#define PLANIFICADOR_FCFS_H

#include <cstddef>
#include <string>

#include "../struct/ListaFifo.h"
#include "Planificador.h"

/**
 * PlanificadorFCFS: First-Come First-Served.
 *
 * Hereda de Planificador.h, sobreescribe sus métodos para simular el algoritmo FCFS utilizando metódos propios de las listas FIFO.
 */
class PlanificadorFCFS : public Planificador {
public:
    std::string nombre() const override { return "FCFS (primero en llegar, primero en ser atendido)"; }

    void agregar(std::size_t indiceProceso, const Proceso&) override { listos_.insertarUltimo(indiceProceso); }

    bool hayListos() const override { return !listos_.vacia(); }

    std::size_t siguiente() override { return listos_.borrarPrimero(); }

    // Sin expulsión: usa la CPU todo lo que le falta.
    int quantum(const Proceso& p) const override { return p.restante; }

    // Contenido actual de la cola de listos (índices).
    const ListaFifo<std::size_t>& listos() const { return listos_; }

private:
    ListaFifo<std::size_t> listos_;
};

#endif
