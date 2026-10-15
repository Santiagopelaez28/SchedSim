#ifndef PILA_SIMPLE
#define PILA_SIMPLE

#include <stdexcept>
#include <cstddef>
#include <utility>

template <typename T>
class Pila {
private:
    struct Nodo {
        T dato;
        Nodo* sig;

        Nodo(T d) : dato(std::move(d)), sig(nullptr) {}
    };

    Nodo* cabeza;   // Nodo superior
    Nodo* ultimo;   // Nodo inferior
    std::size_t tamano_;

    void insertarUltimo(T valor) {
            Nodo* nuevo = new Nodo(std::move(valor));
            if (vacia()) {
                cabeza = nuevo;
            } else {
                ultimo->sig = nuevo;
            }
            ultimo = nuevo;
            tamano_++;
        }
    

public:
    Pila() : cabeza(nullptr), ultimo(nullptr), tamano_(0) {}

    ~Pila() {borrarPila(); }

    // Constructor de copia: copia profunda conservando el orden.
    Pila(const Pila& otro) : cabeza(nullptr), ultimo(nullptr), tamano_(0) {
        try {
            for (const Nodo* actual = otro.cabeza; actual != nullptr; actual = actual->sig) {
                insertarUltimo(actual->dato);
            }
        } catch (...) {
            borrarPila();
            throw;
        }
    }

    // Asignación por copia (copiar y intercambiar).
    Pila& operator=(const Pila& otro) {
        if (this != &otro) {
            Pila copia(otro);
            intercambiar(copia);
        }
        return *this;
    }

    // Constructor de movimiento: roba los nodos de "otro".
    Pila(Pila&& otro) noexcept
        : cabeza(otro.cabeza), ultimo(otro.ultimo), tamano_(otro.tamano_) {
        otro.cabeza = nullptr;
        otro.ultimo = nullptr;
        otro.tamano_ = 0;
    }

    // Asignación por movimiento.
    Pila& operator=(Pila&& otro) noexcept {
        if (this != &otro) {
            borrarPila();
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
            throw std::underflow_error("La pila esta vacia");
        }
        return cabeza->dato;
    }

    const T& primero() const {
        if (vacia()) {
            throw std::underflow_error("La pila esta vacia");
        }
        return cabeza->dato;
    }

    const T& operator[](std::size_t posicion) const {
        if (posicion >= tamano_) {
            throw std::out_of_range("Pila: posicion fuera de rango");
        }
        const Nodo* actual = cabeza;
        for (std::size_t i = 0; i < posicion; ++i) {
            actual = actual->sig;
        }
        return actual->dato;
    }

    void insertarPrimero(T proceso) {
        Nodo* nuevo = new Nodo(std::move(proceso));
        if (vacia()) {
            ultimo = nuevo;
            ultimo->sig = nullptr;
        } else {
            nuevo->sig = cabeza;
        }
        cabeza = nuevo;
        tamano_++;
    }

    bool vacia() const {
        return cabeza == nullptr;
    }

    T borrarPrimero() {
        if (vacia()) {
            throw std::underflow_error("La pila esta vacia");
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

    void borrarPila() noexcept {
        while (!vacia()) {
            Nodo* siguiente = cabeza->sig;
            delete cabeza;
            cabeza = siguiente;
        }
        ultimo = nullptr;
        tamano_ = 0;
    }

    void intercambiar(Pila& otro) noexcept {
        std::swap(cabeza, otro.cabeza);
        std::swap(ultimo, otro.ultimo);
        std::swap(tamano_, otro.tamano_);
    }
};

#endif