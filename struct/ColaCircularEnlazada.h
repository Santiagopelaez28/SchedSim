#ifndef COLA_CIRCULAR_ENLAZADA_H
#define COLA_CIRCULAR_ENLAZADA_H

#include <stdexcept>
#include <cstddef>

template <typename T>
class ColaCircularEnlazada {
private:
    struct Nodo {
        T dato;
        Nodo* sig;

        Nodo(const T& valor) : dato(valor), sig(nullptr) {}
    };

    Nodo* ultimo;
    std::size_t tamano_;

public:
    ColaCircularEnlazada() : ultimo(nullptr), tamano_(0) {}

    ColaCircularEnlazada(const ColaCircularEnlazada&) = delete;
    ColaCircularEnlazada& operator=(const ColaCircularEnlazada&) = delete;

    ~ColaCircularEnlazada() {
        limpiar();
    }


    bool estaVacia() const {
        return ultimo == nullptr;
    }

    std::size_t tamano() const {
        return tamano_;
    }

    T& primero() {
        if (estaVacia()) {
            throw std::underflow_error("La cola esta vacia");
        }

        return ultimo->sig->dato;
    }

    const T& primero() const {
        if (estaVacia()) {
            throw std::underflow_error("La cola esta vacia");
        }

        return ultimo->sig->dato;
    }

    void insertarPrimero(const T& proceso) {
        Nodo* nuevo = new Nodo(proceso);

        if (estaVacia()) {
            nuevo->sig = nuevo;
        } else {
            nuevo->sig = ultimo->sig;
            ultimo->sig = nuevo;
        }

        ultimo = nuevo;
        tamano_++;
    }

    T borrarPrimero() {
        if (estaVacia()) {
            throw std::underflow_error("La cola esta vacia");
        }

        Nodo* primero = ultimo->sig;
        T proceso = primero->dato;

        if (primero == ultimo) {
            ultimo = nullptr;
        } else {
            ultimo->sig = primero->sig;
        }

        delete primero;
        tamano_--;

        return proceso;
    }

    void limpiar() {
        while (!estaVacia()) {
            borrarPrimero();
        }
    }
};

#endif
