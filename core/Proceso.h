#ifndef PROCESO_H
#define PROCESO_H

#include <string>
#include <utility>

enum class EstadoProceso { Nuevo, Listo, Ejecutando, Terminado };

/**
 * Proceso
 *
 * Atributos: pid, nombre, llegada, rafaga y prioridad.
 * Datos que calcula el simulador: restante, estado, inicio y fin.
 *
 * Tiene constructor por defecto para poder ser usado en ArregloDinamico.
 */
struct Proceso {
    int pid = 0;
    std::string nombre;
    int llegada = 0;      // instante en que llega al sistema
    int rafaga = 0;       // tiempo total de CPU que necesita
    int prioridad = 0;    // menor número = mayor prioridad (1 es la más urgente)

    int restante = 0;                         // tiempo de CPU que aún le falta
    EstadoProceso estado = EstadoProceso::Nuevo;
    int inicio = -1;                          // primer instante en que usa la CPU
    int fin = -1;                             // instante en que termina

    Proceso() = default;

    Proceso(int pid_, std::string nombre_, int llegada_, int rafaga_, int prioridad_ = 0)
        : pid(pid_),
          nombre(std::move(nombre_)),
          llegada(llegada_),
          rafaga(rafaga_),
          prioridad(prioridad_),
          restante(rafaga_) {}

    // Métricas (cuando el proceso termina)
    int retorno() const { return fin - llegada; }          // desde que llega hasta que termina
    int espera() const { return retorno() - rafaga; }      // tiempo en la cola de listos
    int respuesta() const { return inicio - llegada; }     // hasta su primera vez en CPU
};

#endif 