#ifndef GENERADOR_PROCESOS_H
#define GENERADOR_PROCESOS_H

#include <cstddef>
#include <optional>
#include <random>
#include <stdexcept>
#include <string>

#include "../struct/ArregloDinamico.h"
#include "Proceso.h"

/**
 * Parámetros del generador. Los valores por defecto producen cargas pequeñas
 * y fáciles de seguir a mano.
 *
 * Convención de prioridad: MENOR número = MAYOR prioridad (1 es la más urgente).
 * Las prioridades se sortean de forma uniforme y pueden repetirse; los empates
 * deben resolverse en el algoritmo (por ejemplo, por orden de llegada).
 */
struct ConfigGenerador {
    int cantidad = 5;                // cuántos procesos generar (>= 1)
    int llegadaMin = 0;              // instante de llegada (>= 0)
    int llegadaMax = 40;
    int rafagaMin = 1;               // tiempo de CPU requerido (>= 1)
    int rafagaMax = 8;
    int prioridadMin = 1;            // la más urgente (>= 1)
    int prioridadMax = 5;            // la menos urgente
    std::optional<unsigned> semilla; // si no se indica, se elige una al azar
};

/**
 * GeneradorProcesos
 *
 * Genera un conjunto de procesos aleatorio pero REPRODUCIBLE: la semilla se
 * fija al construir el generador y generar() siempre devuelve el mismo
 * conjunto para esa semilla. Para obtener un conjunto distinto, se crea otro
 * generador. La semilla usada se puede consultar con semilla() para mostrarla
 * en pantalla y volver a usarla.
 *
 * Nota: std::mt19937 es idéntico en todos los compiladores, pero las
 * distribuciones de <random> pueden variar entre implementaciones, así que la
 * misma semilla garantiza el mismo conjunto solo con el mismo compilador.
 */
class GeneradorProcesos {
public:
    explicit GeneradorProcesos(const ConfigGenerador& config) : config_(config), semilla_(0) {
        validar();
        semilla_ = config_.semilla.has_value() ? *config_.semilla : std::random_device{}();
    }

    unsigned semilla() const { return semilla_; }
    const ConfigGenerador& config() const { return config_; }

    ArregloDinamico<Proceso> generar() const {
        std::mt19937 motor(semilla_);
        std::uniform_int_distribution<int> llegada(config_.llegadaMin, config_.llegadaMax);
        std::uniform_int_distribution<int> rafaga(config_.rafagaMin, config_.rafagaMax);
        std::uniform_int_distribution<int> prioridad(config_.prioridadMin, config_.prioridadMax);

        ArregloDinamico<Proceso> tabla(static_cast<std::size_t>(config_.cantidad));
        for (int pid = 1; pid <= config_.cantidad; ++pid) {
            // Los sorteos van en sentencias separadas, en orden fijo: C++ no
            // define en qué orden se evalúan los argumentos de una llamada.
            const int l = llegada(motor);
            const int r = rafaga(motor);
            const int p = prioridad(motor);
            tabla.agregar(Proceso(pid, "P" + std::to_string(pid), l, r, p));
        }
        return tabla;
    }

private:
    ConfigGenerador config_;
    unsigned semilla_;

    void validar() const {
        if (config_.cantidad <= 0) {
            throw std::invalid_argument("GeneradorProcesos: la cantidad debe ser mayor que 0");
        }
        if (config_.llegadaMin < 0 || config_.llegadaMin > config_.llegadaMax) {
            throw std::invalid_argument(
                "GeneradorProcesos: rango de llegada invalido (0 <= min <= max)");
        }
        if (config_.rafagaMin < 1 || config_.rafagaMin > config_.rafagaMax) {
            throw std::invalid_argument(
                "GeneradorProcesos: rango de rafaga invalido (1 <= min <= max)");
        }
        if (config_.prioridadMin < 1 || config_.prioridadMin > config_.prioridadMax) {
            throw std::invalid_argument(
                "GeneradorProcesos: rango de prioridad invalido (1 <= min <= max)");
        }
    }
};

#endif // GENERADOR_PROCESOS_H
