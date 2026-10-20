#ifndef COLA_CIRCULAR_ENLAZADA_H
#define COLA_CIRCULAR_ENLAZADA_H

#include <stdexcept>
#include <cstddef>


/**
 * La cola circular permite implementar una simulación del algoritmo de planificación
 * de round robin, esta es la unica lista que utiliza un solo nodo en lugar de dos, 
 * ya que el ultimo nodo en su apuntador sigiente apunta al primer nodo, es decir, 
 * está enlazada. Así mismo, los datos que almacena esta cola son nuevamente de tipo 
 * proceso.
 */
template <typename T>
class ColaCircularEnlazada {
private:
    // Cada nodo generado se crea con un dato de tipo T y un apuntador al siguiente nodo
    struct Nodo {
        T dato;
        Nodo* sig;

        Nodo(const T& valor) : dato(valor), sig(nullptr) {}
    };

    // Creación del ultimo nodo de la cola
    Nodo* ultimo;
    std::size_t tamano_;

public:
    // Constructor de la lista
    ColaCircularEnlazada() : ultimo(nullptr), tamano_(0) {}

    ColaCircularEnlazada(const ColaCircularEnlazada&) = delete;
    ColaCircularEnlazada& operator=(const ColaCircularEnlazada&) = delete;

    // Destructor
    ~ColaCircularEnlazada() {
        limpiar();
    }

    /**
     * Metodo propio de la cola circular, valida si el ultimo nodo tiene apuntador vacio
     */
    bool estaVacia() const {
        return ultimo == nullptr;
    }

    // Método getter para tamaño de lista
    std::size_t tamano() const {
        return tamano_;
    }

    /**
     * Valida si la lista está vacia y devuelve el dato del nodo siguiente al ultimo nodo.
     * @returns El dato correspondiente al nodo siguiente al apuntador del ultimo nodo, es decir, el dato de la "cabeza"
     */
    T& primero() {
        if (estaVacia()) {
            throw std::underflow_error("La cola esta vacia");
        }
        return ultimo->sig->dato;
    }

    // Evita modificar el dato al acceder al método
    const T& primero() const {
        if (estaVacia()) {
            throw std::underflow_error("La cola esta vacia");
        }
        return ultimo->sig->dato;
    }

    /**
     * Este método se encarga de tomar un proceso y añadirlo al principio de la lista, en este caso
     * especifico, el nuevo nodo corresponde también al nodo después del ultimo.
     * @param proceso El dato (Proceso) a insertar al principio de la lista.
     */
    void insertarPrimero(const T& proceso) {
        Nodo* nuevo = new Nodo(proceso);
        if (estaVacia()) {
            nuevo->sig = nuevo;     // Si la lista es vacia, el nuevo nodo apunta a sí mismo
        } else {
            // El nuevo nodo apunta a la cabeza y el apuntador del ultimo apunta a este nuevo nodo
            nuevo->sig = ultimo->sig;  
            ultimo->sig = nuevo;
        }
        ultimo = nuevo;
        tamano_++;
    }

    /**
     * Elimina el nodo correspondiente a la cabeza y devuelve su valor (Proceso correspondiente)
     * @returns El dato almacenado en la cabeza de la lista
     */
    T borrarPrimero() {
        if (estaVacia()) {
            throw std::underflow_error("La cola esta vacia");   // No hay nada para borrar en una lista vacía
        }
        // El nodo "primero" actua como nodo auxiliar para saber que nodo se elimina
        Nodo* primero = ultimo->sig;
        T proceso = primero->dato;      // Se guarda el dato almacenado en el nodo a borrar
        if (primero == ultimo) {
            ultimo = nullptr;       // La lista queda vacía
        } else {
            ultimo->sig = primero->sig;     // Se ajusta el apuntador del ultimo nodo
        }
        // Eliminar nodo y reducir tamaño
        delete primero;     
        tamano_--;
        return proceso;
    }

    /**
     * Se utlizan los métodos para validar que la lista no está vacía y eliminar el primer nodo mediante un ciclo while,
     * cuando solo queda un nodo por borrar se cumple la condición if del metodo borrarPrimero().
     * 
     * Este metodo es utilizado en el destructor de la cola circular
     */
    void limpiar() {
        while (!estaVacia()) {
            borrarPrimero();
        }
    }
};

#endif
