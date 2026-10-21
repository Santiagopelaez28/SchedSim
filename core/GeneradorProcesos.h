#ifndef GENERADOR_PROCESOS_H
#define GENERADOR_PROCESOS_H

#include <cstddef>
#include <limits>
#include <optional>
#include <random>
#include <stdexcept>
#include <string>

#include "../struct/ArregloDinamico.h"
#include "Proceso.h"

/**
 * Parámetros del generador, se utilizan valores enteros para cada uno y para el tiempo de CPU
 */
struct ConfigGenerador {
    static constexpr int MAX_PROCESOS = 1000000;    // Maximo de procesos posibles, evita que el programa tarde demasiado en responder
    int cantidad = 30;                // Cantidad de procesos a generar (maximo 1000000)
    int llegadaMin = 0;              // instante de llegada (>= 0)
    int llegadaMax = 10;
    int rafagaMin = 5;               // tiempo de CPU requerido (>= 1)
    int rafagaMax = 50;
    int prioridadMin = 1;            // la más urgente (>= 1)
    int prioridadMax = 10;            // la menos urgente
    std::optional<unsigned> semilla; // si no se indica, se elige una al azar

    // Métodos Setters para cantidad y semilla, reciben el valor ingresado por el usuario
    void setCantidad(int nuevaCantidad) { cantidad = nuevaCantidad; }
    void setSemilla(std::optional<unsigned> nuevaSemilla) { semilla = nuevaSemilla; }   // La semilla recibe un valor opcional, el usuario puede elegir no ingresar nada
};

/**
 * GeneradorProcesos
 *
 * Genera un conjunto de procesos aleatorio a partir de la semilla de la configuración, en diferentes compiladores la misma semilla puede generar diferentes valores.
 */
class GeneradorProcesos {
public:
    explicit GeneradorProcesos(const ConfigGenerador& config) : config_(config), semilla_(0) {
        validar();
        semilla_ = config_.semilla.has_value() ? *config_.semilla : std::random_device{}();     // Valida que la semilla tenga un valor para usar
    }

    unsigned semilla() const { return semilla_; }
    const ConfigGenerador& config() const { return config_; }

    // Utiliza valores aleatorios para insertar cada parametro correspondiente por proceso
    ArregloDinamico<Proceso> generar() const {
        std::mt19937 motor(semilla_);
        std::uniform_int_distribution<int> llegada(config_.llegadaMin, config_.llegadaMax);
        std::uniform_int_distribution<int> rafaga(config_.rafagaMin, config_.rafagaMax);
        std::uniform_int_distribution<int> prioridad(config_.prioridadMin, config_.prioridadMax);

        // Crea la tabla donde se mostrarán los procesos creados
        ArregloDinamico<Proceso> tabla(static_cast<std::size_t>(config_.cantidad));
        for (int pid = 1; pid <= config_.cantidad; ++pid) {
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
        if (config_.cantidad > ConfigGenerador::MAX_PROCESOS) {
            throw std::invalid_argument("GeneradorProcesos: la cantidad maxima es " +
                                        std::to_string(ConfigGenerador::MAX_PROCESOS));
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
        // Peor caso del reloj: última llegada + suma de todas las ráfagas.
        const long long peorTiempo =
            static_cast<long long>(config_.cantidad) * config_.rafagaMax + config_.llegadaMax;
        if (peorTiempo > std::numeric_limits<int>::max()) {
            throw std::invalid_argument(
                "GeneradorProcesos: cantidad * rafagaMax + llegadaMax excede el rango de int");
        }
    }
};

#endif