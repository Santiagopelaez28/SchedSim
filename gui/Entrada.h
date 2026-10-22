#ifndef ENTRADA_H
#define ENTRADA_H

#include <cctype>
#include <istream>
#include <limits>
#include <optional>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>

#include "../struct/ArregloDinamico.h"
#include "../core/FabricaPlanificadores.h"

/**
 * Ayudantes de entrada para la consola.
 *
 * Reciben el flujo de entrada y el de salida como parámetros en lugar de usar
 * std::cin y std::cout directamente, para poder probarlos con texto simulado.
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

// Convierte un texto en una semilla (unsigned de 32 bits, como la que muestra el
// generador). Solo admite dígitos, sin signo, con espacios opcionales alrededor.
// Hace falta aparte de parsearEntero porque una semilla elegida al azar puede
// superar el rango de int.
inline std::optional<unsigned> parsearSemilla(const std::string& texto) {
    std::size_t i = 0;
    std::size_t j = texto.size();
    while (i < j && std::isspace(static_cast<unsigned char>(texto[i]))) ++i;
    while (j > i && std::isspace(static_cast<unsigned char>(texto[j - 1]))) --j;
    if (i == j) return std::nullopt;

    unsigned long long valor = 0;
    for (; i < j; ++i) {
        const char c = texto[i];
        if (c < '0' || c > '9') return std::nullopt;
        valor = valor * 10 + static_cast<unsigned>(c - '0');
        if (valor > std::numeric_limits<unsigned>::max()) return std::nullopt;
    }
    return static_cast<unsigned>(valor);
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

}

// Se lanza cuando la entrada se cierra (fin de archivo) antes de recibir lo que se pedía.
class EntradaCerrada : public std::runtime_error {
public:
    EntradaCerrada() : std::runtime_error("Entrada cerrada antes de recibir un valor valido") {}
};

// Lee una línea completa. Lanza EntradaCerrada si ya no hay más entrada.
inline std::string leerLinea(std::istream& entrada) {
    std::string linea;
    if (!std::getline(entrada, linea)) throw EntradaCerrada();
    return linea;
}

/**
 * Pide un entero en [minimo, maximo] y repite la pregunta hasta recibir uno válido.
 * Si se indica porDefecto, pulsar solo Enter lo elige.
 *
 * Lanza un error si la entrada se cierra (fin de archivo)
 * antes de recibir un valor válido: sin esto, una entrada redirigida o cerrada
 * giraría para siempre.
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

        const std::string linea = leerLinea(entrada);
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

/**
 * Pide la semilla de los procesos aleatorios. Enter = semilla aleatoria (devuelve
 * std::nullopt); un número la fija, y con ella los mismos parámetros generan siempre
 * los mismos procesos. Repite la pregunta si el texto no es una semilla válida.
 */
inline std::optional<unsigned> leerSemilla(std::istream& entrada, std::ostream& salida) {
    while (true) {
        salida << "Semilla (entero sin signo; Enter = aleatoria): ";
        salida.flush();
        const std::string linea = leerLinea(entrada);
        if (detalle::soloEspacios(linea)) return std::nullopt;
        const std::optional<unsigned> semilla = parsearSemilla(linea);
        if (semilla.has_value()) return semilla;
        salida << "  Valor invalido: escribe un entero entre 0 y "
               << std::numeric_limits<unsigned>::max() << ", o Enter para una semilla aleatoria.\n";
    }
}

/**
 * Muestra los algoritmos numerados y pide cuáles elegir: números separados por
 * espacios o comas (Enter = todos). Los repetidos se ignoran y el resultado sale en
 * el orden del catálogo, sea cual sea el orden en que se escribieron. Exige al menos
 * "minimo" algoritmos distintos (para comparar hacen falta dos).
 */
inline ArregloDinamico<Algoritmo> leerAlgoritmos(std::istream& entrada, std::ostream& salida,
                                                 std::size_t minimo = 2) {
    const ArregloDinamico<Algoritmo> catalogo = todosLosAlgoritmos();
    if (minimo > catalogo.tamano()) {
        throw std::invalid_argument("leerAlgoritmos: el minimo supera la cantidad de algoritmos");
    }

    salida << "Algoritmos disponibles:\n";
    for (std::size_t i = 0; i < catalogo.tamano(); ++i) {
        salida << "  " << (i + 1) << ". " << nombre(catalogo[i]) << "\n";
    }

    const int n = static_cast<int>(catalogo.tamano());
    while (true) {
        salida << "Cuales elegir (numeros separados por espacios o comas; Enter = todos): ";
        salida.flush();
        const std::string linea = leerLinea(entrada);

        ArregloDinamico<bool> elegido;
        for (std::size_t i = 0; i < catalogo.tamano(); ++i) elegido.agregar(false);

        bool valida = true;
        if (detalle::soloEspacios(linea)) {
            for (std::size_t i = 0; i < elegido.tamano(); ++i) elegido[i] = true;
        } else {
            std::string texto = linea;
            for (char& c : texto) {
                if (c == ',') c = ' ';
            }
            std::istringstream palabras(texto);
            std::string palabra;
            bool hayNumeros = false;
            while (palabras >> palabra) {
                hayNumeros = true;
                const std::optional<int> numero = parsearEntero(palabra);
                if (!numero.has_value() || *numero < 1 || *numero > n) {
                    valida = false;
                    break;
                }
                elegido[static_cast<std::size_t>(*numero - 1)] = true;
            }
            if (!hayNumeros) valida = false;
        }

        if (!valida) {
            salida << "  Valor invalido: escribe numeros entre 1 y " << n
                   << " separados por espacios, o Enter para elegirlos todos.\n";
            continue;
        }

        ArregloDinamico<Algoritmo> resultado;
        for (std::size_t i = 0; i < catalogo.tamano(); ++i) {
            if (elegido[i]) resultado.agregar(catalogo[i]);
        }
        if (resultado.tamano() < minimo) {
            salida << "  Hay que elegir al menos " << minimo << " algoritmos distintos.\n";
            continue;
        }
        return resultado;
    }
}

// Aviso (no error) cuando el quantum es tan grande que Round Robin equivale a FCFS.
// Devuelve un texto vacío si no hay nada que advertir.
inline std::string advertenciaQuantum(int quantum, int rafagaMaxima) {
    if (quantum < rafagaMaxima) return "";
    return "Aviso: el quantum (" + std::to_string(quantum) +
           ") es mayor o igual que la rafaga maxima (" + std::to_string(rafagaMaxima) +
           "); Round Robin se comportara igual que FCFS.";
}

#endif 