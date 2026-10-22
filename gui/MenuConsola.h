#ifndef MENU_CONSOLA_H
#define MENU_CONSOLA_H

#include <cstddef>
#include <exception>
#include <istream>
#include <memory>
#include <optional>
#include <ostream>
#include <string>

#include "../struct/ArregloDinamico.h"
#include "../core/Comparador.h"
#include "../core/FabricaPlanificadores.h"
#include "../core/GeneradorProcesos.h"
#include "../core/Simulador.h"
#include "Comparacion.h"
#include "Entrada.h"
#include "Salida.h"

constexpr int CANTIDAD_POR_DEFECTO = 30;

/**
 * Menú de consola de SchedSim.
 *
 * Diseño:
 *   - Solo conoce el catálogo de algoritmos (FabricaPlanificadores.h): no nombra clases
 *     concretas ni sabe qué algoritmos usan quantum. Añadir un algoritmo no lo cambia.
 *   - Recibe el flujo de entrada y el de salida (no usa std::cin ni std::cout), así se
 *     prueba con texto simulado. main() solo lo crea y lo ejecuta.
 *   - Toda entrada del usuario pasa por las funciones validadas de Entrada.h: un texto
 *     inválido repite la pregunta, nunca rompe el programa.
 *   - Recuerda el último quantum y la última cantidad: Enter los conserva.
 *   - Un error al simular no cierra el programa: se informa y se vuelve al menú. Solo
 *     el fin de la entrada (EntradaCerrada) termina el programa.
 */
class MenuConsola {
public:
    // "base" fija los rangos de llegada, ráfaga y prioridad de los procesos aleatorios;
    // el menú solo cambia la cantidad y la semilla.
    MenuConsola(std::istream& entrada, std::ostream& salida, ConfigGenerador base = ConfigGenerador(),
                OpcionesSalida opcionesSalida = OpcionesSalida())
        : entrada_(entrada), salida_(salida), base_(base), opcionesSalida_(opcionesSalida) {}

    // Ejecuta el menú hasta que el usuario elige salir o se cierra la entrada.
    int ejecutar() {
        const ArregloDinamico<Algoritmo> algoritmos = todosLosAlgoritmos();
        const int opcionComparar = static_cast<int>(algoritmos.tamano()) + 1;
        try {
            while (true) {
                mostrarMenu(algoritmos, opcionComparar);
                const int opcion = leerEntero(entrada_, salida_, "Opcion", 0, opcionComparar);
                if (opcion == 0) {
                    salida_ << "Saliendo de SchedSim.\n";
                    return 0;
                }
                try {
                    if (opcion == opcionComparar) {
                        compararVarios();
                    } else {
                        ejecutarUno(algoritmos[static_cast<std::size_t>(opcion - 1)]);
                    }
                } catch (const EntradaCerrada&) {
                    throw;                                   // se maneja más abajo: termina el programa
                } catch (const std::exception& e) {
                    salida_ << "\nError: " << e.what() << "\n" << SEPARADOR;
                }
            }
        } catch (const EntradaCerrada&) {
            salida_ << "\nEntrada cerrada. Saliendo de SchedSim.\n";
            return 0;
        }
    }

private:
    static constexpr const char* SEPARADOR = "----------------------------------------------------------\n";

    std::istream& entrada_;
    std::ostream& salida_;
    ConfigGenerador base_;
    OpcionesSalida opcionesSalida_;
    int quantum_ = QUANTUM_POR_DEFECTO;      // último quantum elegido
    int cantidad_ = CANTIDAD_POR_DEFECTO;    // última cantidad de procesos elegida

    void mostrarMenu(const ArregloDinamico<Algoritmo>& algoritmos, int opcionComparar) const {
        salida_ << "\n=== SchedSim ===\nSeleccione una opcion:\n";
        for (std::size_t i = 0; i < algoritmos.tamano(); ++i) {
            salida_ << "  " << (i + 1) << ". " << nombre(algoritmos[i]) << " - "
                    << descripcion(algoritmos[i]);
            if (usaQuantum(algoritmos[i])) salida_ << " [quantum actual = " << quantum_ << "]";
            salida_ << "\n";
        }
        salida_ << "  " << opcionComparar << ". Comparar algoritmos\n";
        salida_ << "  0. Salir de SchedSim\n";
    }

    // Pide la cantidad de procesos y la semilla, y devuelve el generador ya configurado.
    // Es el ÚNICO lugar donde el menú toca los parámetros de ConfigGenerador: si tu clase
    // los expone de otra forma, solo hay que cambiar estas dos líneas.
    GeneradorProcesos pedirEscenario() {
        cantidad_ = leerEntero(entrada_, salida_, "Cantidad de procesos", 1,
                               ConfigGenerador::MAX_PROCESOS, cantidad_);
        const std::optional<unsigned> semilla = leerSemilla(entrada_, salida_);

        ConfigGenerador config = base_;
        config.setCantidad(cantidad_);
        config.setSemilla(semilla);
        return GeneradorProcesos(config);        // valida los parámetros; lanza si no sirven
    }

    static int rafagaMaxima(const ArregloDinamico<Proceso>& tabla) {
        int maxima = 0;
        for (const Proceso& p : tabla) {
            if (p.rafaga > maxima) maxima = p.rafaga;
        }
        return maxima;
    }

    void mostrarCabecera(unsigned semilla) const {
        salida_ << "\nSchedSim - semilla: " << semilla << " (escribela para repetir este escenario)\n\n";
    }

    void ejecutarUno(Algoritmo algoritmo) {
        salida_ << "\n--- " << nombre(algoritmo) << " ---\n";

        std::optional<int> quantum;
        if (usaQuantum(algoritmo)) {
            quantum_ = leerQuantum(entrada_, salida_, quantum_);
            quantum = quantum_;
        }
        const GeneradorProcesos generador = pedirEscenario();
        const ArregloDinamico<Proceso> tabla = generador.generar();

        std::unique_ptr<Planificador> planificador = crearPlanificador(algoritmo, quantum);
        Simulador simulador(tabla);
        prepararSimulador(simulador, opcionesSalida_);   // con muchos procesos desactiva el Gantt
        simulador.ejecutar(*planificador);

        mostrarCabecera(generador.semilla());
        mostrarProcesos(salida_, simulador, opcionesSalida_);
        salida_ << "\n";
        if (quantum.has_value()) {
            const std::string aviso = advertenciaQuantum(*quantum, rafagaMaxima(tabla));
            if (!aviso.empty()) salida_ << aviso << "\n\n";
        }
        mostrarResultados(salida_, simulador, planificador->nombre(), opcionesSalida_);
        salida_ << SEPARADOR;
    }

    void compararVarios() {
        salida_ << "\n--- Comparar algoritmos ---\n";
        const ArregloDinamico<Algoritmo> elegidos = leerAlgoritmos(entrada_, salida_, 2);

        // Un solo quantum para todos, y solo si alguno de los elegidos lo usa.
        const std::optional<int> quantum = pedirQuantumSiHaceFalta(entrada_, salida_, elegidos, quantum_);
        if (quantum.has_value()) quantum_ = *quantum;

        const GeneradorProcesos generador = pedirEscenario();
        const ArregloDinamico<Proceso> tabla = generador.generar();

        mostrarCabecera(generador.semilla());
        {
            Simulador vista(tabla);              // solo para listar los procesos; se libera enseguida
            mostrarProcesos(salida_, vista, opcionesSalida_);
        }
        salida_ << "\nComparacion de algoritmos";
        if (quantum.has_value()) salida_ << " - quantum = " << *quantum << " (solo lo usa Round Robin)";
        salida_ << "\n";
        if (quantum.has_value()) {
            const std::string aviso = advertenciaQuantum(*quantum, rafagaMaxima(tabla));
            if (!aviso.empty()) salida_ << aviso << "\n";
        }
        salida_ << "\n";

        const ArregloDinamico<ResultadoAlgoritmo> resultados = compararAlgoritmos(tabla, elegidos, quantum);
        imprimirComparacion(salida_, resultados);
        salida_ << SEPARADOR;
    }
};

#endif 