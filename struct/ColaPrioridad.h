#ifndef COLA_PRIORIDAD_H
#define COLA_PRIORIDAD_H

#include <cstddef>
#include <functional>
#include <stdexcept>
#include <utility>

#include "ArregloDinamico.h"

/**
 * Esta cola con prioridad implementa un min-heap binario sobre el arreglo dinamico.
 * El monticulo es min-heap ya que se utiliza la convención de, el menor número es el más prioritario,
 * además, anterior es el proceso con mayor prioridad (Menor número)
 */
template <typename T, typename Anterior = std::less<T>>
class ColaPrioridad {
public:
    ColaPrioridad() = default;      // Constructor por defecto
    explicit ColaPrioridad(Anterior anterior) : anterior_(std::move(anterior)) {}

    // Metodo getter para tamaño
    std::size_t tamano() const { return elementos_.tamano(); }
    // Valida si el monticulo está vacio, no es override del método del arreglo dinamico, solo toma su valor de retorno.
    bool estaVacia() const { return elementos_.estaVacio(); }

    /** Elemento que sería el próximo en salir, es de solo lectura
     * @returns Dato almacenado en el monticulo en la primera posición (Más prioritario)
    */
    const T& primero() const {
        verificarNoVacia();
        return elementos_[0];
    }

    /** Agrega al final del arreglo y lo sube hasta su lugar.
     * Al igual de un árbol binario, al insertar el valor, en este caso al final del arreglo, 
     * lo compara con su nodo padre hasta subirlo a la posición correspondiente
    */
    void insertarUltimo(T valor) {
        elementos_.agregar(std::move(valor));
        subirNodo(elementos_.tamano() - 1);
    }

    /**  Quita y devuelve el elemento que debe salir primero (la raíz).
     * @returns El proceso más prioritario, es decir, la raíz del árbol teorico
    */
    T borrarPrimero() {
        verificarNoVacia();
        if (elementos_.tamano() == 1) {
            return elementos_.quitarUltimo();
        }
        T raiz = std::move(elementos_[0]);
        T ultimo = elementos_.quitarUltimo();   // El último ocupa la raíz
        elementos_[0] = std::move(ultimo);
        bajarNodo(0);                              // Luego, el ultimo se baja hasta la posición correspondiente
        return raiz;
    }

    // Utiliza el mismo metodo para limpiar que arreglo dinámico
    void limpiar() { elementos_.limpiar(); }

private:
    ArregloDinamico<T> elementos_;  // Monticulo que almacena los procesos de más a menos prioritarios
    Anterior anterior_;

    /**
     * Utilizando el mismo método de arreglo dinámico, valida que este no sea vacio
     * @returns Error si el arreglo está vacio
     */
    void verificarNoVacia() const {
        if (elementos_.estaVacio()) {
            throw std::underflow_error("ColaPrioridad: la cola esta vacia");
        }
    }

    /** Mientras el elemento tenga más prioridad que su padre, intercambian lugares.
     * @param i El indice del dato que debe cambiar posiciones con su padre
     */
    void subirNodo(std::size_t i) {
        while (i > 0) {
            const std::size_t padre = (i - 1) / 2;      // [1,2,3,1] el padre del ultimo 1 es 2, ya que se simula un árbol binario
            if (!anterior_(elementos_[i], elementos_[padre])) break;    // Valida si el nodo actual debe intercambiar con su padre
            std::swap(elementos_[i], elementos_[padre]);    // Intercambia de posición el dato y su padre
            i = padre;      // Cambia el padre, de modo que [1,1,3,2] y se mantiene la propiedad de min-heap
        }
    }

    /**
     * Utiliza como parametro el indice del proceso a comparar con su hijo.
     * @param i El indice de datos que debe intercambiar posiciones con alguno de sus hijos
     */
    void bajarNodo(std::size_t i) {
        const std::size_t n = elementos_.tamano();
        while (true) {
            const std::size_t izquierdo = 2 * i + 1;    // La rama izquierda, indices impares
            const std::size_t derecho = 2 * i + 2;      // La rama derecha, indices pares
            std::size_t mejor = i;
            // Busca el mejor proceso en la rama izquierda
            if (izquierdo < n && anterior_(elementos_[izquierdo], elementos_[mejor])) {
                mejor = izquierdo;
            }
            // Busca el mejor proceso en la rama derecha
            if (derecho < n && anterior_(elementos_[derecho], elementos_[mejor])) {
                mejor = derecho;
            }
            if (mejor == i) break;      // Al llegar a la posición del mejor, termina el bucle
            std::swap(elementos_[i], elementos_[mejor]);
            i = mejor;
        }
    }
};

#endif 
