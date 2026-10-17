#ifndef SALIDA_H
#define SALIDA_H

#include <cstddef>
#include <ostream>
#include <string>

#include "../core/Simulador.h"

/**
 * Política de salida por consola según la cantidad de procesos.
 *
 *   - Hasta umbralDetalle procesos: tabla completa de procesos, diagrama de Gantt
 *     (partido en bloques que caben en anchoLinea) y tabla de resultados completa.
 *   - Con más procesos: solo un resumen (totales y promedios) y las primeras
 *     filasMuestra filas de ejemplo. El diagrama de Gantt ni siquiera se registra,
 *     lo que además ahorra memoria.
 *
 * Imprimir un millón de filas no es útil ni viable; los promedios y el uso de CPU
 * resumen igual la simulación.
 */
struct OpcionesSalida {
    std::size_t umbralDetalle = 30;   // hasta esta cantidad de procesos se muestra todo
    std::size_t filasMuestra = 10;    // con más procesos, cuántas filas de ejemplo se muestran
    std::size_t anchoLinea = 80;      // ancho máximo del diagrama de Gantt, en caracteres
};

inline bool mostrarDetalle(std::size_t cantidadProcesos,
                           const OpcionesSalida& opciones = OpcionesSalida()) {
    return cantidadProcesos <= opciones.umbralDetalle;
}

// Llamar ANTES de ejecutar: con muchos procesos desactiva el registro del Gantt.
inline void prepararSimulador(Simulador& simulador,
                              const OpcionesSalida& opciones = OpcionesSalida()) {
    simulador.registrarGantt(mostrarDetalle(simulador.procesos().tamano(), opciones));
}

// Tabla de los procesos que se simularon (completa o solo las primeras filas).
inline void mostrarProcesos(std::ostream& os, const Simulador& simulador,
                            const OpcionesSalida& opciones = OpcionesSalida()) {
    const std::size_t n = simulador.procesos().tamano();
    os << "Procesos generados (" << n << "; prioridad: 1 = la mas urgente)\n";
    simulador.imprimirProcesos(os, mostrarDetalle(n, opciones) ? Simulador::SIN_LIMITE
                                                                : opciones.filasMuestra);
}

// Resultados de una simulación ya ejecutada, con el nivel de detalle que corresponda.
inline void mostrarResultados(std::ostream& os, const Simulador& simulador,
                              const std::string& titulo,
                              const OpcionesSalida& opciones = OpcionesSalida()) {
    const std::size_t n = simulador.procesos().tamano();
    os << "=== " << titulo << " ===\n";
    if (mostrarDetalle(n, opciones)) {
        os << "\nDiagrama de Gantt\n";
        simulador.imprimirGantt(os, opciones.anchoLinea);
        os << "\nResultados por proceso\n";
        simulador.imprimirMetricas(os);
    } else {
        os << "\nResumen (con mas de " << opciones.umbralDetalle
           << " procesos se omiten el diagrama de Gantt y el detalle completo)\n";
        simulador.imprimirMetricas(os, opciones.filasMuestra);
    }
}

#endif // SALIDA_H
