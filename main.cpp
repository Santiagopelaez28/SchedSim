#include <cstdlib>
#include <exception>
#include <iostream>

#include "struct/ArregloDinamico.h"
#include "core/GeneradorProcesos.h"
#include "core/PlanificadorRR.h"
#include "core/Simulador.h"

// Demostración de la V1: Round Robin sobre procesos generados al azar.
// Uso: ./schedsim [quantum] [cantidad] [semilla]
//   quantum   por defecto 3
//   cantidad  por defecto 5
//   semilla   si se omite, se elige una al azar (se muestra en pantalla)
int main(int argc, char* argv[]) {
    try {
        int quantum = 3; //todo: validar que sea un entero positivo y hacer que sea ingresado por el usuario
        ConfigGenerador config;
        if (argc > 1) quantum = std::atoi(argv[1]); 
        if (argc > 2) config.cantidad = std::atoi(argv[2]);
        if (argc > 3) config.semilla = static_cast<unsigned>(std::strtoul(argv[3], nullptr, 10));

        GeneradorProcesos generador(config);
        ArregloDinamico<Proceso> tabla = generador.generar();

        PlanificadorRR planificador(quantum);
        Simulador simulador(tabla);
        simulador.ejecutar(planificador);

        std::cout << "=== SchedSim: " << planificador.nombre() << " ===\n";
        std::cout << "Procesos aleatorios: " << tabla.tamano() << "  (semilla: "
                  << generador.semilla() << ")\n\n";

        std::cout << "Procesos generados (prioridad: 1 = la mas urgente)\n";
        simulador.imprimirProcesos(std::cout);

        std::cout << "\nDiagrama de Gantt\n";
        simulador.imprimirGantt(std::cout);

        std::cout << "\nMetricas\n";
        simulador.imprimirMetricas(std::cout);
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
}
