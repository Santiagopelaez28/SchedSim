#ifndef ARREGLO_DINAMICO_H
#define ARREGLO_DINAMICO_H

#include <cstddef>
#include <stdexcept>
#include <utility>

/**
 * ArregloDinamico<T>
 * 
 * Este arreglo toma como valores validos objetos con constructor propio como lo son los procesos generados 
 * por el programa, mantiene los métodos como insertar al final e insertar y borrar valores en una posición especifica,
 * el arreglo duplica su tamaño cada vez que se llena, es decir, cuando tamano_ == capacidad_.
 * 
 * Esta plantilla es utilizada para realizar el monticulo min-heap para el manejo de la cola con prioridad.
 */
template <typename T>
class ArregloDinamico
{
public:

    explicit ArregloDinamico(std::size_t capacidadInicial = 4)
        : datos_(nullptr),
          tamano_(0),
          capacidad_(capacidadInicial == 0 ? 1 : capacidadInicial)
    {
        datos_ = new T[capacidad_];
    }

    ~ArregloDinamico() { delete[] datos_; }     // Destructor

    // Constructor de copia
    ArregloDinamico(const ArregloDinamico &otro)
        : datos_(new T[otro.capacidad_]),
          tamano_(otro.tamano_),
          capacidad_(otro.capacidad_)
    {
        for (std::size_t i = 0; i < tamano_; ++i)
        {
            datos_[i] = otro.datos_[i];
        }
    }

    // Asignación por copia (intercambia valores entre un arreglo y otro sin perder memoria)
    ArregloDinamico &operator=(const ArregloDinamico &otro)
    {
        if (this != &otro)
        {
            ArregloDinamico copia(otro);
            intercambiar(copia);
        }
        return *this;
    }

    // Constructor de movimiento tomando el bloque de memoria de otro arreglo
    ArregloDinamico(ArregloDinamico &&otro) noexcept
        : datos_(otro.datos_), tamano_(otro.tamano_), capacidad_(otro.capacidad_)
    {
        otro.datos_ = nullptr;
        otro.tamano_ = 0;
        otro.capacidad_ = 0;
    }

    // Asignación por movimiento.
    ArregloDinamico &operator=(ArregloDinamico &&otro) noexcept
    {
        if (this != &otro)
        {
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


    // Métodos getters
    std::size_t tamano() const { return tamano_; }
    std::size_t capacidad() const { return capacidad_; }
    // Validación de arreglo vacio
    bool estaVacio() const { return tamano_ == 0; }

    /** Acceso por índice con verificación de límites.
     * Utiliza el metodo de verificar indice por si existe o no en el arreglo.
     * @param indice Para verificar si dicho indice existe en el arreglo.
     * @returns Un proceso que corresponde al valor insertado en la posición antes verifcada.
    */
    T &operator[](std::size_t indice)
    {
        verificarIndice(indice);
        return datos_[indice];
    }

    // Método de tipo constante, no es posible alterar el valor dentro del arreglo si se accede mediante éste.
    const T &operator[](std::size_t indice) const
    {
        verificarIndice(indice);
        return datos_[indice];
    }

    /** Agrega al final tomando una copia del valor a ingresar 
     * @param valor De tipo de dato correspondiente al objeto, lo añade en la posición con valor = tamano_ y luego aumenta este ultimo
    */
    void agregar(T valor)
    {
        asegurarEspacio();
        datos_[tamano_] = std::move(valor);
        tamano_++;
    }

    /** Inserta en una posición dada 
     * @param posicion El indice donde se insertará el dato
     * @param valor El dato, en este caso de tipo proceso, a insertar en la posición dada
    */
    void insertar(std::size_t posicion, T valor)
    {
        if (posicion > tamano_)
        {
            throw std::out_of_range("ArregloDinamico::insertar: posicion fuera de rango");
        }
        asegurarEspacio();
        for (std::size_t i = tamano_; i > posicion; --i)
        {
            datos_[i] = std::move(datos_[i - 1]);
        }
        datos_[posicion] = std::move(valor);
        tamano_++;
    }

    /** Elimina el elemento de la posición ingresada y lo devuelve 
     * @param posicion El indice de donde se eliminará el dato
    */
    T eliminarEn(std::size_t posicion)
    {
        verificarIndice(posicion);
        T eliminado = std::move(datos_[posicion]);
        for (std::size_t i = posicion; i + 1 < tamano_; ++i)
        {
            datos_[i] = std::move(datos_[i + 1]);
        }
        tamano_--;
        return eliminado;
    }

    // Elimina y devuelve el último elemento
    T quitarUltimo()
    {
        if (tamano_ == 0)
        {
            throw std::out_of_range("ArregloDinamico::quitarUltimo: arreglo vacio");
        }
        tamano_--;
        return std::move(datos_[tamano_]);
    }

    // Vacía el arreglo conservando la capacidad
    void limpiar() { tamano_ = 0; }

    /** Garantiza una capacidad mínima sin cambiar el tamaño 
     * @param nuevaCapacidad La capacidad objetivo para reservar dicho espacio en memoria
    */
    void reservar(std::size_t nuevaCapacidad)
    {
        if (nuevaCapacidad > capacidad_)
        {
            redimensionar(nuevaCapacidad);
        }
    }

    // Iteración

    T *begin() { return datos_; }
    T *end() { return datos_ + tamano_; }
    const T *begin() const { return datos_; }
    const T *end() const { return datos_ + tamano_; }

private:
    T *datos_;
    std::size_t tamano_;
    std::size_t capacidad_;

    void verificarIndice(std::size_t indice) const
    {
        if (indice >= tamano_)
        {
            throw std::out_of_range("ArregloDinamico: indice fuera de rango");
        }
    }

    // Si el arreglo está lleno, duplica la capacidad.
    void asegurarEspacio()
    {
        if (tamano_ == capacidad_)
        {
            redimensionar(capacidad_ == 0 ? 1 : capacidad_ * 2);
        }
    }

    /** Crea un bloque nuevo, traslada los elementos y libera el anterior. 
     * Toma una capacidad objetivo y genera un arreglo vacio con dicha capacidad, luego, mueve los recursos
     * de memoria de cada dato dentro del arreglo anterior al nuevo y elimina el anterior, el tamaño y los datos 
     * se mantienen iguales.
     * @param nuevaCapacidad La capacidad final con la cual se generará el nuevo arreglo
    */
    void redimensionar(std::size_t nuevaCapacidad)
    {
        T *nuevo = new T[nuevaCapacidad];
        for (std::size_t i = 0; i < tamano_; ++i)
        {
            nuevo[i] = std::move(datos_[i]);
        }
        delete[] datos_;
        datos_ = nuevo;
        capacidad_ = nuevaCapacidad;
    }

    /** Mueve los datos de un arreglo a otro y viceversa sin generar copias
     * Utiliza swap para intercambiar los recursos de memoria entre dos arreglos,
     * esto cambia también el tamaño y la capacidad para evitar errores.
     */
    void intercambiar(ArregloDinamico &otro) noexcept
    {
        std::swap(datos_, otro.datos_);
        std::swap(tamano_, otro.tamano_);
        std::swap(capacidad_, otro.capacidad_);
    }
};

#endif
