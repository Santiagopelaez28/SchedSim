#ifndef ARREGLO_DINAMICO_H
#define ARREGLO_DINAMICO_H

#include <cstddef>
#include <stdexcept>
#include <utility>

/**
 * ArregloDinamico<T>
 *
 * Arreglo que crece automáticamente cuando se llena (duplica su capacidad).
 *
 * Complejidad:
 *   - acceso por índice ........ O(1)
 *   - agregar al final ......... O(1) amortizado
 *   - insertar / eliminar en el medio ... O(n)
 *   - redimensionar ............ O(n) (ocurre pocas veces)
 *
 * Requisito: T debe tener constructor por defecto y ser copiable/movible.
 */
template <typename T>
class ArregloDinamico {
public:
    // ---------- Construcción y destrucción ----------

    explicit ArregloDinamico(std::size_t capacidadInicial = 4)
        : datos_(nullptr),
          tamano_(0),
          capacidad_(capacidadInicial == 0 ? 1 : capacidadInicial) {
        datos_ = new T[capacidad_];
    }

    ~ArregloDinamico() { delete[] datos_; }

    // Constructor de copia: copia profunda de los elementos.
    ArregloDinamico(const ArregloDinamico& otro)
        : datos_(new T[otro.capacidad_]),
          tamano_(otro.tamano_),
          capacidad_(otro.capacidad_) {
        for (std::size_t i = 0; i < tamano_; ++i) {
            datos_[i] = otro.datos_[i];
        }
    }

    // Asignación por copia (copiar y intercambiar).
    ArregloDinamico& operator=(const ArregloDinamico& otro) {
        if (this != &otro) {
            ArregloDinamico copia(otro);
            intercambiar(copia);
        }
        return *this;
    }

    // Constructor de movimiento: roba el bloque de memoria de "otro".
    ArregloDinamico(ArregloDinamico&& otro) noexcept
        : datos_(otro.datos_), tamano_(otro.tamano_), capacidad_(otro.capacidad_) {
        otro.datos_ = nullptr;
        otro.tamano_ = 0;
        otro.capacidad_ = 0;
    }

    // Asignación por movimiento.
    ArregloDinamico& operator=(ArregloDinamico&& otro) noexcept {
        if (this != &otro) {
            delete[] datos_;
            datos_ = otro.datos_;
            tamano_ = otro.tamano_;
            capacidad_ = otro.capacidad_;
            otro.datos_ = nullptr;
            otro.tamano_ = 0;
            otro.capacidad_ = 0;
        }
        return *this;
    }

    // ---------- Consulta ----------

    std::size_t tamano() const { return tamano_; }
    std::size_t capacidad() const { return capacidad_; }
    bool estaVacio() const { return tamano_ == 0; }

    // Acceso por índice con verificación de límites.
    T& operator[](std::size_t indice) {
        verificarIndice(indice);
        return datos_[indice];
    }

    const T& operator[](std::size_t indice) const {
        verificarIndice(indice);
        return datos_[indice];
    }

    // ---------- Modificación ----------

    // Agrega al final. Recibe el valor por copia a propósito: si el valor
    // fuera una referencia a un elemento del propio arreglo, redimensionar
    // la dejaría apuntando a memoria liberada.
    void agregar(T valor) {
        asegurarEspacio();
        datos_[tamano_] = std::move(valor);
        ++tamano_;
    }

    // Inserta en la posición dada (0 <= posicion <= tamano).
    void insertar(std::size_t posicion, T valor) {
        if (posicion > tamano_) {
            throw std::out_of_range("ArregloDinamico::insertar: posicion fuera de rango");
        }
        asegurarEspacio();
        for (std::size_t i = tamano_; i > posicion; --i) {
            datos_[i] = std::move(datos_[i - 1]);
        }
        datos_[posicion] = std::move(valor);
        ++tamano_;
    }

    // Elimina el elemento de la posición dada y lo devuelve.
    T eliminarEn(std::size_t posicion) {
        verificarIndice(posicion);
        T eliminado = std::move(datos_[posicion]);
        for (std::size_t i = posicion; i + 1 < tamano_; ++i) {
            datos_[i] = std::move(datos_[i + 1]);
        }
        --tamano_;
        return eliminado;
    }

    // Elimina y devuelve el último elemento (útil para el heap y la pila).
    T quitarUltimo() {
        if (tamano_ == 0) {
            throw std::out_of_range("ArregloDinamico::quitarUltimo: arreglo vacio");
        }
        --tamano_;
        return std::move(datos_[tamano_]);
    }

    // Vacía el arreglo conservando la capacidad.
    void limpiar() { tamano_ = 0; }

    // Garantiza una capacidad mínima sin cambiar el tamaño.
    void reservar(std::size_t nuevaCapacidad) {
        if (nuevaCapacidad > capacidad_) {
            redimensionar(nuevaCapacidad);
        }
    }

    // ---------- Iteración (permite usar for (const auto& x : arreglo)) ----------

    T* begin() { return datos_; }
    T* end() { return datos_ + tamano_; }
    const T* begin() const { return datos_; }
    const T* end() const { return datos_ + tamano_; }

private:
    T* datos_;
    std::size_t tamano_;
    std::size_t capacidad_;

    void verificarIndice(std::size_t indice) const {
        if (indice >= tamano_) {
            throw std::out_of_range("ArregloDinamico: indice fuera de rango");
        }
    }

    // Si el arreglo está lleno, duplica la capacidad.
    void asegurarEspacio() {
        if (tamano_ == capacidad_) {
            redimensionar(capacidad_ == 0 ? 1 : capacidad_ * 2);
        }
    }

    // Crea un bloque nuevo, traslada los elementos y libera el anterior.
    void redimensionar(std::size_t nuevaCapacidad) {
        T* nuevo = new T[nuevaCapacidad];
        for (std::size_t i = 0; i < tamano_; ++i) {
            nuevo[i] = std::move(datos_[i]);
        }
        delete[] datos_;
        datos_ = nuevo;
        capacidad_ = nuevaCapacidad;
    }

    void intercambiar(ArregloDinamico& otro) noexcept {
        std::swap(datos_, otro.datos_);
        std::swap(tamano_, otro.tamano_);
        std::swap(capacidad_, otro.capacidad_);
    }
};

#endif // ARREGLO_DINAMICO_H
