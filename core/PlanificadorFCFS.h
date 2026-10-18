#ifndef PLANIFICADOR_FCFS_H
#define PLANIFICADOR_FCFS_H

#include <cstddef>
#include <string>

#include "../struct/ListaFifo.h"
#include "Planificador.h"

/**
 * PlanificadorFCFS: First-Come First-Served.
 *
 * No hay expulsión: cuando un proceso toma la CPU la conserva hasta terminar. El Simulador entrega los procesos al
 * planificador en orden de llegada, y la cola simple los atiende en ese orden.
 */
class PlanificadorFCFS : public Planificador {
public:
    std::string nombre() const override { return "FCFS (primero en llegar, primero en ser atendido)"; }

    void agregar(std::size_t indiceProceso, const Proceso&) override { listos_.insertarUltimo(indiceProceso); }

    bool hayListos() const override { return !listos_.vacia(); }

    std::size_t siguiente() override { return listos_.borrarPrimero(); }

    // Sin expulsión: usa la CPU todo lo que le falta.
    int quantum(const Proceso& p) const override { return p.restante; }

    // Contenido actual de la cola de listos (índices), de solo lectura.
    const ListaFifo<std::size_t>& listos() const { return listos_; }

private:
    ListaFifo<std::size_t> listos_;
};

#endif
