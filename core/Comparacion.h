#ifndef COMPARACION_H
#define COMPARACION_H

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <iomanip>
#include <ostream>
#include <sstream>
#include <string>

#include "../struct/ArregloDinamico.h"
#include "../core/Comparador.h"

namespace detalle {

// Una fila de la tabla comparativa: su etiqueta y una celda ya formateada por algoritmo.
struct FilaComparacion {
    std::string etiqueta;
    ArregloDinamico<std::string> celdas;
};

inline std::string formatearNumero(double valor, int decimales) {
    std::ostringstream flujo;
    flujo << std::fixed << std::setprecision(decimales) << valor;
    return flujo.str();
}

/**
 * Construye una fila. Cada celda termina en un carácter extra: '*' si es el mejor
 * valor de la fila y ' ' si no, así los números quedan alineados.
 *
 * Solo se marca cuando tiene sentido: la fila debe poder compararse (menorEsMejor)
 * y los valores no pueden ser todos iguales. Los empates en el mejor valor se
 * marcan todos. La comparación usa los valores redondeados a los decimales que se
 * muestran, para no marcar un "mejor" que se ve igual que los demás.
 */
inline FilaComparacion crearFila(const std::string& etiqueta, const ArregloDinamico<double>& valores,
                                 int decimales, bool menorEsMejor) {
    const double escala = std::pow(10.0, decimales);
    auto redondear = [escala](double v) { return std::round(v * escala) / escala; };

    double minimo = redondear(valores[0]);
    double maximo = minimo;
    for (const double v : valores) {
        minimo = std::min(minimo, redondear(v));
        maximo = std::max(maximo, redondear(v));
    }
    const bool hayDiferencia = (maximo - minimo) > 1e-9;

    FilaComparacion fila;
    fila.etiqueta = etiqueta;
    for (const double v : valores) {
        const bool esMejor = menorEsMejor && hayDiferencia && redondear(v) <= minimo + 1e-9;
        fila.celdas.agregar(formatearNumero(v, decimales) + (esMejor ? "*" : " "));
    }
    return fila;
}

}  // namespace detalle

/**
 * Imprime la tabla comparativa: una columna por algoritmo y una fila por métrica.
 * Los anchos se adaptan al contenido. Las métricas de tiempo de espera, retorno y
 * respuesta tienen marcado el mejor valor (el menor).
 */
inline void imprimirComparacion(std::ostream& os, const ArregloDinamico<ResultadoAlgoritmo>& resultados) {
    if (resultados.estaVacio()) {
        os << "(sin resultados que comparar)\n";
        return;
    }

    auto valores = [&resultados](auto extraer) {
        ArregloDinamico<double> v;
        for (const ResultadoAlgoritmo& r : resultados) v.agregar(static_cast<double>(extraer(r)));
        return v;
    };

    ArregloDinamico<detalle::FilaComparacion> filas;
    filas.agregar(detalle::crearFila(
        "Tiempo total", valores([](const ResultadoAlgoritmo& r) { return r.tiempoTotal; }), 0, false));
    filas.agregar(detalle::crearFila(
        "Uso de CPU (%)", valores([](const ResultadoAlgoritmo& r) { return r.utilizacionCPU; }), 2, false));
    filas.agregar(detalle::crearFila(
        "Espera promedio", valores([](const ResultadoAlgoritmo& r) { return r.esperaPromedio; }), 2, true));
    filas.agregar(detalle::crearFila(
        "Retorno promedio", valores([](const ResultadoAlgoritmo& r) { return r.retornoPromedio; }), 2, true));
    filas.agregar(detalle::crearFila(
        "Respuesta promedio", valores([](const ResultadoAlgoritmo& r) { return r.respuestaPromedio; }), 2, true));
    filas.agregar(detalle::crearFila(
        "Espera maxima", valores([](const ResultadoAlgoritmo& r) { return r.esperaMaxima; }), 0, true));

    // Anchos: la columna de etiquetas y una por algoritmo, con dos espacios de separación.
    std::size_t anchoEtiqueta = std::string("Metrica").size();
    for (const detalle::FilaComparacion& fila : filas) {
        anchoEtiqueta = std::max(anchoEtiqueta, fila.etiqueta.size());
    }
    anchoEtiqueta += 2;

    ArregloDinamico<std::string> encabezados;
    ArregloDinamico<std::size_t> anchos;
    for (std::size_t j = 0; j < resultados.tamano(); ++j) {
        encabezados.agregar(nombre(resultados[j].algoritmo) + " ");   // el espacio alinea con la marca
        std::size_t ancho = encabezados[j].size();
        for (const detalle::FilaComparacion& fila : filas) {
            ancho = std::max(ancho, fila.celdas[j].size());
        }
        anchos.agregar(ancho + 2);
    }

    const std::ios_base::fmtflags banderas = os.flags();
    os << std::left << std::setw(static_cast<int>(anchoEtiqueta)) << "Metrica" << std::right;
    for (std::size_t j = 0; j < encabezados.tamano(); ++j) {
        os << std::setw(static_cast<int>(anchos[j])) << encabezados[j];
    }
    os << "\n";
    for (const detalle::FilaComparacion& fila : filas) {
        os << std::left << std::setw(static_cast<int>(anchoEtiqueta)) << fila.etiqueta << std::right;
        for (std::size_t j = 0; j < fila.celdas.tamano(); ++j) {
            os << std::setw(static_cast<int>(anchos[j])) << fila.celdas[j];
        }
        os << "\n";
    }
    os.flags(banderas);

    os << "\n* = mejor valor de la fila (menor es mejor)\n";

    // Sin costo de cambio de contexto, el algoritmo solo reordena el trabajo: la
    // CPU hace lo mismo en el mismo tiempo. Se avisa para que no parezca un error.
    bool tiemposIguales = resultados.tamano() > 1;
    for (const ResultadoAlgoritmo& r : resultados) {
        if (r.tiempoTotal != resultados[0].tiempoTotal ||
            r.tiempoOcupado != resultados[0].tiempoOcupado) {
            tiemposIguales = false;
        }
    }
    if (tiemposIguales) {
        os << "El tiempo total y el uso de CPU son iguales en todos los algoritmos: sin costo de\n"
              "cambio de contexto, el algoritmo solo cambia el orden en que se atiende a los procesos.\n";
    }
}

#endif // COMPARACION_H
