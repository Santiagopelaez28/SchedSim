#include <cstdlib>
#include <exception>
#include <iostream>
#include <optional>

#include "struct/ArregloDinamico.h"
#include "core/GeneradorProcesos.h"
#include "core/PlanificadorPrioridad.h"
#include "core/PlanificadorFCFS.h"
#include "core/PlanificadorRR.h"
#include "core/Simulador.h"
#include "gui/Salida.h"

// Demostración de la V1: Round Robin sobre procesos generados al azar.
// Uso: ./schedsim [quantum] [cantidad] [semilla]
//   quantum   por defecto 3
//   cantidad  por defecto 5 (hasta ConfigGenerador::MAX_PROCESOS)
//   semilla   si se omite, se elige una al azar (se muestra en pantalla)
//
// Con pocos procesos se muestra todo el detalle; con muchos, solo un resumen
// (ver ui/Salida.h).
// int main(int argc, char* argv[]) {
//     try {
//         if (argc > 1) quantum = std::atoi(argv[1]);
//         if (argc > 2) config.cantidad = std::atoi(argv[2]);
//         if (argc > 3) config.semilla = static_cast<unsigned>(std::strtoul(argv[3], nullptr, 10));
int main()
{
    try
    {
        int quantum = 3;
        int cantidad = 30;
        ConfigGenerador config;
        int opcion;

        do
        {
            std::cout << "Seleccione el planificador:\n"
                      << "1. Round Robin (quantum = " << quantum << ")\n"
                      << "2. First-Come, First-Served (FCFS)\n"
                      << "3. Prioridad (sin expulsion; 1 = mas urgente)\n"
                      << "0. Salir de SchedSim\n"
                      << "Opcion: ";
            std::cin >> opcion;

            PlanificadorRR planificadorrr(quantum);
            PlanificadorFCFS planificadorfcfs;
            PlanificadorPrioridad planificadorprioridad;
            switch (opcion)
            {
            case 1:
            {
                std::cout << "Planificador Round Robin\n";
                std::cout << "Ingrese el quantum: ";
                std::cin >> quantum;
                planificadorrr.setQuantum(quantum);
                std::cout << "Ingrese la cantidad de procesos a simular (Por defecto 30): ";
                std::cin >> cantidad;
                config.setCantidad(cantidad);
                GeneradorProcesos generador(config);
                ArregloDinamico<Proceso> tabla = generador.generar();
                Simulador simulador(tabla);
                config.setCantidad(cantidad);
                prepararSimulador(simulador);
                simulador.ejecutar(planificadorrr);
                std::cout << "SchedSim - semilla: " << generador.semilla() << "\n\n";
                mostrarProcesos(std::cout, simulador);
                std::cout << "\n";
                mostrarResultados(std::cout, simulador, planificadorrr.nombre());
                std::cout << "----------------------------------------------------------\n";
            }
            break;
            case 2:
            {
                std::cout << "Planificador FCFS:\n";
                std::cout << "Ingrese la cantidad de procesos a simular (Por defecto 30): ";
                std::cin >> cantidad;
                config.setCantidad(cantidad);
                GeneradorProcesos generador(config);
                ArregloDinamico<Proceso> tabla = generador.generar();
                Simulador simulador(tabla);
                prepararSimulador(simulador);
                simulador.ejecutar(planificadorfcfs);
                std::cout << "SchedSim - semilla: " << generador.semilla() << "\n\n";
                mostrarProcesos(std::cout, simulador);
                std::cout << "\n";
                mostrarResultados(std::cout, simulador, planificadorfcfs.nombre());
                std::cout << "----------------------------------------------------------\n";
            }
            break;
            case 3:
            {
                std::cout << "Planificador Prioridad:\n";
                std::cout << "Ingrese la cantidad de procesos a simular (Por defecto 30): ";
                std::cin >> cantidad;
                config.setCantidad(cantidad);
                GeneradorProcesos generador(config);
                ArregloDinamico<Proceso> tabla = generador.generar();
                Simulador simulador(tabla);
                prepararSimulador(simulador);
                simulador.ejecutar(planificadorprioridad);
                std::cout << "SchedSim - semilla: " << generador.semilla() << "\n\n";
                mostrarProcesos(std::cout, simulador);
                std::cout << "\n";
                mostrarResultados(std::cout, simulador, planificadorprioridad.nombre());
                std::cout << "----------------------------------------------------------\n";
            }
            break;
            case 0:
                std::cout << "Saliendo de SchedSim.\n";
                break;
            default:
                std::cout << ("Opcion invalida. Por favor, seleccione una opcion valida.\n");
            }
        } while (opcion != 0);

        return 0;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
}
