#ifndef LISTA_FIFO_SIMPLE
#define LISTA_FIFO_SIMPLE

#include <stdexcept>
#include <cstddef>
#include <utility>


/**
 * Esta cola esta definida de manera FIFO (First Imput, First Output).
 * Es una cola sencilla para simular el algoritmo FCFS (First-Come, First-Served), 
 * al igual que el resto de estructuras de datos utilizadas en el programa, se almacenan
 * objetos de tipo Proceso
 */
template <typename T>
class ListaFifo {
private:
    // Definición del nodo
    struct Nodo {
        T dato;
        Nodo* sig;

        Nodo(T d) : dato(std::move(d)), sig(nullptr) {}
    };

    // Se utiliza un nodo que representa la cabeza y uno que representa el ultimo de la lista.
    Nodo* cabeza;
    Nodo* ultimo;
    std::size_t tamano_;

public:
    // Constructor con datos NULL
    ListaFifo() : cabeza(nullptr), ultimo(nullptr), tamano_(0) {}

    // Destructor
    ~ListaFifo() {borrarLista(); }

    // Constructor de copia
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

    // Asignación por copia
    ListaFifo& operator=(const ListaFifo& otro) {
        if (this != &otro) {
            ListaFifo copia(otro);
            intercambiar(copia);
        }
        return *this;
    }

    // Constructor de movimiento
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

    // Método getter para tamaño
    std::size_t tamano() const {return tamano_;}

    /**
     * Valida si la lista esta vacía, si no, toma el dato almacenado en la cabeza y lo devuelve
     * @returns El dato (Proceso) almacenado en el primer nodo (Cabeza) 
     */
    T& primero() {
        if (vacia()) {
            throw std::underflow_error("La cola esta vacia");
        }
        return cabeza->dato;
    }

    // Evita manipular el dato de la cabeza al acceder al método
    const T& primero() const {
        if (vacia()) {
            throw std::underflow_error("La cola esta vacia");
        }
        return cabeza->dato;
    }

    /**
     * Busca el dato almacenado en una posición especifica
     * @param posicion El nodo donde se busca el dato
     * @returns El dato almacenado en el nodo
     */
    const T& operator[](std::size_t posicion) const {
        if (posicion >= tamano_) {
            throw std::out_of_range("Cola: posicion fuera de rango");   // El nodo no se encuentra dentro de la lista
        }
        const Nodo* actual = cabeza;    // Nodo auxiliar 
        // Ciclo for para recorrer la lista nodo por nodo hasta la posición dada
        for (std::size_t i = 0; i < posicion; ++i) {
            actual = actual->sig;
        }
        return actual->dato;
    }

    /**
     * Genera un nuevo nodo con un valor dado para insertarlo en la ultima posición de la lista
     * @param proceso El dato (Proceso) a insertar al final de la lista
     */
    void insertarUltimo(T proceso) {
        Nodo* nuevo = new Nodo(std::move(proceso));     // Creación del nuevo nodo a insertar
        if (vacia()) {
            // Ambos apuntadores de cabeza y ultimo apuntan al unico nodo insertado
            cabeza = nuevo;
            ultimo = nuevo;
            ultimo->sig = nullptr;      // El ultimo nodo siempre apunta a NULL
        } else {
            ultimo->sig = nuevo;        // Ultimo apunta al nuevo nodo
            nuevo->sig = nullptr;        // El nuevo nodo apunta a NULL
        }
        // El nuevo nodo se convierte en el ultimo y se aumenta el tamaño de la lista
        ultimo = nuevo;    
        tamano_++;
    }

    /**
     * Valida si la cabeza tiene valor NULL, al ser constante no se altera el valor del retorno
     * @return True si la cabeza no tiene ningún dato o si es NULL
     */
    bool vacia() const {
        return cabeza == nullptr;
    }

    /**
     * Se utiliza un nodo auxiliar para borrar el primer nodo de la lista
     * @returns El dato (Proceso) almacenado en el nodo borrado
     */
    T borrarPrimero() {
        if (vacia()) {
            throw std::underflow_error("La cola esta vacia");   // No se borra nada
        }
        // Se crea el nodo auxiliar y se almacena el dato a borrar
        Nodo* primero = cabeza;
        T proceso = std::move(primero->dato);
        cabeza = primero->sig;      // Se hace la cabeza al nodo siguiente del nodo a borrar (Antes cabeza)
        // Si la lista quedará vacia al eliminar el primer nodo
        if (cabeza == nullptr) {
            ultimo = nullptr;
        }
        // Borra el nodo y disminuye el tamaño de la lista
        delete primero;
        tamano_--;
        return proceso;
    }

    /**
     * Mediante un nodo auxiliar, se elimina la cabeza hasta que la lista quede vacía.
     * A diferencia de borrarPrimero(), este método no devuelve el proceso eliminado
     */
    void borrarLista() noexcept {
        while (!vacia()) {
            Nodo* siguiente = cabeza->sig;  // Definición del nodo auxiliar
            delete cabeza;
            cabeza = siguiente;
        }
        ultimo = nullptr;   // Al llegar al ultimo nodo, se valida que el ultimo nodo también tenga dato vacío
        tamano_ = 0;
    }

    // Se utiliza swap para intercambiar los nodos en la memoria entre dos listas
    void intercambiar(ListaFifo& otro) noexcept {
        std::swap(cabeza, otro.cabeza);
        std::swap(ultimo, otro.ultimo);
        std::swap(tamano_, otro.tamano_);
    }
};

#endif