#ifndef COLA_PRIORIDAD_H
#define COLA_PRIORIDAD_H

#include <cstddef>
#include <functional>
#include <stdexcept>
#include <utility>

#include "ArregloDinamico.h"

/**
 * ColaPrioridad<T, Anterior>
 *
 * Cola con prioridad implementada como HEAP BINARIO sobre un ArregloDinamico.
 * El heap es un árbol binario casi completo guardado por niveles en el arreglo:
 *
 *        posición:   0   1   2   3   4   5   6
 *        árbol:          [0]
 *                       /   \
 *                    [1]     [2]
 *                   /  \     /  \
 *                 [3]  [4] [5]  [6]
 *
 *   padre de i ........ (i - 1) / 2
 *   hijo izquierdo .... 2 * i + 1
 *   hijo derecho ...... 2 * i + 2
 *
 * Invariante: ningún hijo anterior a su padre. Por eso la raíz (posición 0) es
 * siempre el elemento que debe salir primero.
 *
 * "Anterior" es un objeto función: Anterior()(a, b) devuelve true si a debe salir
 * ANTES que b. Con el valor por defecto, std::less<T>, sale primero el menor
 * (heap de mínimo). Con std::greater<T> sale primero el mayor.
 *
 * Importante: si dos elementos no se pueden distinguir con Anterior (empate), el
 * orden en que salen no está definido. Para desempatar de forma determinista
 * hay que incluir el criterio de desempate dentro de Anterior.
 *
 * Complejidad: insertarUltimo y borrarPrimero -> O(log n)
 *              primero, tamano, estaVacia -> O(1)
 *
 * Requisito: T debe tener constructor por defecto (lo exige ArregloDinamico) y
 * ser copiable/movible. Copia y movimiento los resuelve ArregloDinamico.
 */
template <typename T, typename Anterior = std::less<T>>
class ColaPrioridad {
public:
    ColaPrioridad() = default;
    explicit ColaPrioridad(Anterior anterior) : anterior_(std::move(anterior)) {}

    // ---------- Consulta ----------

    std::size_t tamano() const { return elementos_.tamano(); }
    bool estaVacia() const { return elementos_.estaVacio(); }

    // Elemento que sería el próximo en salir. Solo lectura: modificarlo desde
    // fuera podría romper el invariante del heap.
    const T& primero() const {
        verificarNoVacia();
        return elementos_[0];
    }

    // ---------- Modificación ----------

    // Agrega al final del arreglo y lo "sube" hasta su lugar.
    void insertarUltimo(T valor) {
        elementos_.agregar(std::move(valor));
        subirNodo(elementos_.tamano() - 1);
    }

    // Quita y devuelve el elemento que debe salir primero (la raíz).
    // Lanza std::underflow_error si la cola está vacía.
    T borrarPrimero() {
        verificarNoVacia();
        if (elementos_.tamano() == 1) {
            return elementos_.quitarUltimo();
        }
        T raiz = std::move(elementos_[0]);
        T ultimo = elementos_.quitarUltimo();   // el último ocupa la raíz...
        elementos_[0] = std::move(ultimo);
        bajarNodo(0);                              // ...y se "hunde" hasta su lugar
        return raiz;
    }

    void limpiar() { elementos_.limpiar(); }

private:
    ArregloDinamico<T> elementos_;
    Anterior anterior_;

    void verificarNoVacia() const {
        if (elementos_.estaVacio()) {
            throw std::underflow_error("ColaPrioridad: la cola esta vacia");
        }
    }

    // Mientras el elemento preceda a su padre, intercambian lugares.
    void subirNodo(std::size_t i) {
        while (i > 0) {
            const std::size_t padre = (i - 1) / 2;
            if (!anterior_(elementos_[i], elementos_[padre])) break;
            std::swap(elementos_[i], elementos_[padre]);
            i = padre;
        }
    }

    // Mientras algún hijo preceda al elemento, intercambia con el mejor hijo.
    void bajarNodo(std::size_t i) {
        const std::size_t n = elementos_.tamano();
        while (true) {
            const std::size_t izquierdo = 2 * i + 1;
            const std::size_t derecho = 2 * i + 2;
            std::size_t mejor = i;
            if (izquierdo < n && anterior_(elementos_[izquierdo], elementos_[mejor])) {
                mejor = izquierdo;
            }
            if (derecho < n && anterior_(elementos_[derecho], elementos_[mejor])) {
                mejor = derecho;
            }
            if (mejor == i) break;
            std::swap(elementos_[i], elementos_[mejor]);
            i = mejor;
        }
    }
};

#endif // COLA_PRIORIDAD_H
