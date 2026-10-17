#ifndef ENTRADA_H
#define ENTRADA_H

#include <cctype>
#include <istream>
#include <limits>
#include <optional>
#include <ostream>
#include <stdexcept>
#include <string>

#include "../struct/ArregloDinamico.h"
#include "../core/FabricaPlanificadores.h"

/**
 * Ayudantes de entrada para la consola.
 *
 * Reciben el flujo de entrada y el de salida como parámetros (en vez de usar
 * std::cin y std::cout directamente) para poder probarlos con texto simulado.
 */

constexpr int QUANTUM_POR_DEFECTO = 3;

// Convierte un texto en un entero de forma ESTRICTA. Acepta espacios alrededor y un
// signo opcional; rechaza texto vacío, caracteres extra ("3x", "2.5", "3 4") y
// valores fuera del rango de int. A diferencia de atoi, un error no se convierte en 0.
inline std::optional<int> parsearEntero(const std::string& texto) {
    std::size_t i = 0;
    std::size_t j = texto.size();
    while (i < j && std::isspace(static_cast<unsigned char>(texto[i]))) ++i;
    while (j > i && std::isspace(static_cast<unsigned char>(texto[j - 1]))) --j;
    if (i == j) return std::nullopt;

    bool negativo = false;
    if (texto[i] == '+' || texto[i] == '-') {
        negativo = (texto[i] == '-');
        ++i;
    }
    if (i == j) return std::nullopt;

    long long valor = 0;
    for (; i < j; ++i) {
        const char c = texto[i];
        if (c < '0' || c > '9') return std::nullopt;
        valor = valor * 10 + (c - '0');
        if (valor > 2147483648LL) return std::nullopt;   // ya no cabe en int; evita desbordar long long
    }
    if (negativo) valor = -valor;
    if (valor < std::numeric_limits<int>::min() || valor > std::numeric_limits<int>::max()) {
        return std::nullopt;
    }
    return static_cast<int>(valor);
}

namespace detalle {

inline bool soloEspacios(const std::string& texto) {
    for (const char c : texto) {
        if (!std::isspace(static_cast<unsigned char>(c))) return false;
    }
    return true;
}

inline std::string describirRango(int minimo, int maximo) {
    if (maximo == std::numeric_limits<int>::max()) {
        return "entero mayor o igual que " + std::to_string(minimo);
    }
    return "entero entre " + std::to_string(minimo) + " y " + std::to_string(maximo);
}

}  // namespace detalle

/**
 * Pide un entero en [minimo, maximo] y repite la pregunta hasta recibir uno válido.
 * Si se indica porDefecto, pulsar solo Enter lo elige.
 *
 * Lanza std::runtime_error si la entrada se cierra (fin de archivo) antes de recibir
 * un valor válido: sin esto, una entrada redirigida o cerrada giraría para siempre.
 * Lanza std::invalid_argument si los parámetros no tienen sentido.
 */
inline int leerEntero(std::istream& entrada, std::ostream& salida, const std::string& pregunta,
                      int minimo, int maximo, std::optional<int> porDefecto = std::nullopt) {
    if (minimo > maximo) {
        throw std::invalid_argument("leerEntero: el minimo no puede superar al maximo");
    }
    if (porDefecto.has_value() && (*porDefecto < minimo || *porDefecto > maximo)) {
        throw std::invalid_argument("leerEntero: el valor por defecto esta fuera del rango");
    }

    const std::string rango = detalle::describirRango(minimo, maximo);
    while (true) {
        salida << pregunta << " (" << rango << ")";
        if (porDefecto.has_value()) salida << " [Enter = " << *porDefecto << "]";
        salida << ": ";
        salida.flush();

        std::string linea;
        if (!std::getline(entrada, linea)) {
            throw std::runtime_error("Entrada cerrada antes de recibir un valor valido");
        }
        if (porDefecto.has_value() && detalle::soloEspacios(linea)) return *porDefecto;

        const std::optional<int> valor = parsearEntero(linea);
        if (valor.has_value() && *valor >= minimo && *valor <= maximo) return *valor;

        salida << "  Valor invalido: escribe un " << rango << ".\n";
    }
}

// Pide el quantum (entero >= 1, con valor por defecto).
inline int leerQuantum(std::istream& entrada, std::ostream& salida,
                       int porDefecto = QUANTUM_POR_DEFECTO) {
    return leerEntero(entrada, salida, "Quantum de Round Robin", 1,
                      std::numeric_limits<int>::max(), porDefecto);
}

/**
 * Pide el quantum SOLO si alguno de los algoritmos elegidos lo usa; si ninguno lo
 * usa, no pregunta nada y devuelve std::nullopt. Se pregunta una sola vez: en el
 * modo de comparación todos los algoritmos que lo usan comparten el mismo quantum.
 */
inline std::optional<int> pedirQuantumSiHaceFalta(std::istream& entrada, std::ostream& salida,
                                                  const ArregloDinamico<Algoritmo>& elegidos,
                                                  int porDefecto = QUANTUM_POR_DEFECTO) {
    bool algunoLoUsa = false;
    for (const Algoritmo algoritmo : elegidos) {
        if (usaQuantum(algoritmo)) {
            algunoLoUsa = true;
            break;
        }
    }
    if (!algunoLoUsa) return std::nullopt;
    return leerQuantum(entrada, salida, porDefecto);
}

// Aviso (no error) cuando el quantum es tan grande que Round Robin equivale a FCFS.
// Devuelve un texto vacío si no hay nada que advertir.
inline std::string advertenciaQuantum(int quantum, int rafagaMaxima) {
    if (quantum < rafagaMaxima) return "";
    return "Aviso: el quantum (" + std::to_string(quantum) +
           ") es mayor o igual que la rafaga maxima (" + std::to_string(rafagaMaxima) +
           "); Round Robin se comportara igual que FCFS.";
}

#endif // ENTRADA_H
