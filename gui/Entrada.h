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
 * Ayuda en las entradas y salidas para la consola.
 *
 * Reciben el flujo de entrada y el de salida como parámetros sin utilizar cin/cout directamente
 */

constexpr int QUANTUM_POR_DEFECTO = 3;

/**
 *  Convierte un texto en un entero de forma ESTRICTA. Acepta espacios alrededor y un
 * signo opcional; rechaza texto vacío, caracteres extra ("3x", "2.5", "3 4") y
 * valores fuera del rango de int. 
 * @param texto Recibe una cadena de texto por referencia constante que representa el número a convertir.
 * @return Devuelve un valor de tipo entero si la conversión es exitosa o nullopt de lo contrario.
*/
inline std::optional<int> convertirEntero(const std::string& texto) {
    std::size_t i = 0;
    std::size_t j = texto.size();
    while (i < j && std::isspace(static_cast<unsigned char>(texto[i]))) ++i;    // Omitir espacios en blanco al inicio
    while (j > i && std::isspace(static_cast<unsigned char>(texto[j - 1]))) --j;    // Omitir espacios al final
    if (i == j) return std::nullopt;

    bool negativo = false;
    if (texto[i] == '+' || texto[i] == '-') {
        negativo = (texto[i] == '-');
        i++;
    }
    if (i == j) return std::nullopt;

    long long valor = 0;
    for (; i < j; i++) {
        const char c = texto[i];
        if (c < '0' || c > '9') return std::nullopt;
        valor = valor * 10 + (c - '0');
        if (valor > 2147483648LL) return std::nullopt;   // Fuera de rango de int, evita desbordar long long
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
        if (!std::isspace(static_cast<unsigned char>(c))) return false;     // Texto con caracteres diferentes a espacios.
    }
    return true;
}

inline std::string describirRango(int minimo, int maximo) {
    if (maximo == std::numeric_limits<int>::max()) {
        return "entero mayor o igual que " + std::to_string(minimo);
    }
    return "entero entre " + std::to_string(minimo) + " y " + std::to_string(maximo);
}

} 

/**
 * Recibe un entero del usuario, con un rango mínimo y máximo. Se repite hasta que el valor ingresado sea valido.
 *
 * @param entrada Flujo de entrada desde el cual se leerá el valor.
 * @param salida Flujo de salida donde se mostrará la pregunta y los mensajes de error.
 * @param pregunta Texto que se mostrará al usuario para solicitar el valor.
 * @param minimo Valor mínimo permitido para la entrada.
 * @param maximo Valor máximo permitido para la entrada.
 * @param porDefecto Valor opcional que se utilizará si el usuario presiona Enter sin registrar un valor.
 * @return El valor entero ingresado por el usuario, o el valor por defecto si se proporcionó y el usuario presionó Enter.
 * @throws std::invalid_argument Si el mínimo es mayor que el máximo o si el valor por defecto está fuera del rango permitido.
 * @throws std::runtime_error Si la entrada se c
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

        const std::optional<int> valor = convertirEntero(linea);
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

// Avisar al usuario si el quantum es mayor o igual que la ráfaga máxima, de ser así Round Robin se comportará igual que FCFS.
inline std::string advertenciaQuantum(int quantum, int rafagaMaxima) {
    if (quantum < rafagaMaxima) return "";
    return "Aviso: el quantum (" + std::to_string(quantum) +
           ") es mayor o igual que la rafaga maxima (" + std::to_string(rafagaMaxima) +
           "); Round Robin se comportara igual que FCFS.";
}

#endif 