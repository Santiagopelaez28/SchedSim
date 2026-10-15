#ifndef LISTA_FIFO_SIMPLE
#define LISTA_FIFO_SIMPLE

#include <stdexcept>
#include <cstddef>
#include <utility>

template <typename T>
class ListaFifo {
private:
    struct Nodo {
        T dato;
        Nodo* sig;

        Nodo(T d) : dato(std::move(d)), sig(nullptr) {}
    };

    Nodo* cabeza;
    Nodo* ultimo;
    std::size_t tamano_;

public:
    ListaFifo() : cabeza(nullptr), ultimo(nullptr), tamano_(0) {}

    ~ListaFifo() {borrarLista(); }

    // Constructor de copia: copia profunda conservando el orden.
    ListaFifo(const ListaFifo& otro) : cabeza(nullptr), ultimo(nullptr), tamano_(0) {
        try {
            for (const Nodo* actual = otro.cabeza; actual != nullptr; actual = actual->sig) {
                insertarUltimo(actual->dato);
            }
        } catch (...) {
            borrarLista();
            throw;
        }
    }

    // Asignación por copia (copiar y intercambiar).
    ListaFifo& operator=(const ListaFifo& otro) {
        if (this != &otro) {
            ListaFifo copia(otro);
            intercambiar(copia);
        }
        return *this;
    }

    // Constructor de movimiento: roba los nodos de "otro".
    ListaFifo(ListaFifo&& otro) noexcept
        : cabeza(otro.cabeza), ultimo(otro.ultimo), tamano_(otro.tamano_) {
        otro.cabeza = nullptr;
        otro.ultimo = nullptr;
        otro.tamano_ = 0;
    }

    // Asignación por movimiento.
    ListaFifo& operator=(ListaFifo&& otro) noexcept {
        if (this != &otro) {
            borrarLista();
            cabeza = otro.cabeza;
            ultimo = otro.ultimo;
            tamano_ = otro.tamano_;
            otro.cabeza = nullptr;
            otro.ultimo = nullptr;
            otro.tamano_ = 0;
        }
        return *this;
    }

    std::size_t tamano() const {return tamano_;}

    T& primero() {
        if (vacia()) {
            throw std::underflow_error("La cola esta vacia");
        }
        return cabeza->dato;
    }

    const T& primero() const {
        if (vacia()) {
            throw std::underflow_error("La cola esta vacia");
        }
        return cabeza->dato;
    }

    const T& operator[](std::size_t posicion) const {
        if (posicion >= tamano_) {
            throw std::out_of_range("Cola: posicion fuera de rango");
        }
        const Nodo* actual = cabeza;
        for (std::size_t i = 0; i < posicion; ++i) {
            actual = actual->sig;
        }
        return actual->dato;
    }

    void insertarUltimo(T proceso) {
        Nodo* nuevo = new Nodo(std::move(proceso));
        if (vacia()) {
            cabeza = nuevo;
            ultimo = nuevo;
            ultimo->sig = nullptr;
        } else {
            ultimo->sig = nuevo;
            nuevo->sig = nullptr;
        }
        ultimo = nuevo;
        tamano_++;
    }

    bool vacia() const {
        return cabeza == nullptr;
    }

    T borrarPrimero() {
        if (vacia()) {
            throw std::underflow_error("La cola esta vacia");
        }

        Nodo* primero = cabeza;
        T proceso = std::move(primero->dato);
        cabeza = primero->sig;
        if (cabeza == nullptr) {
            ultimo = nullptr;
        }
        delete primero;
        tamano_--;
        return proceso;
    }

    void borrarLista() noexcept {
        while (!vacia()) {
            Nodo* siguiente = cabeza->sig;
            delete cabeza;
            cabeza = siguiente;
        }
        ultimo = nullptr;
        tamano_ = 0;
    }

    void intercambiar(ListaFifo& otro) noexcept {
        std::swap(cabeza, otro.cabeza);
        std::swap(ultimo, otro.ultimo);
        std::swap(tamano_, otro.tamano_);
    }
};

#endif