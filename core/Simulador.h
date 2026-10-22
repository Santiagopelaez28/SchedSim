#ifndef SIMULADOR_H
#define SIMULADOR_H

#include <algorithm>
#include <cstddef>
#include <iomanip>
#include <limits>
#include <ostream>
#include <stdexcept>
#include <string>
#include <utility>

#include "../struct/ArregloDinamico.h"
#include "../struct/ColaPrioridad.h"
#include "Planificador.h"
#include "Proceso.h"

// Índice que marca un tramo de CPU ociosa (no corresponde a ningún proceso).
constexpr std::size_t INDICE_OCIOSO = std::numeric_limits<std::size_t>::max();

// Un tramo continuo del diagrama de Gantt. Es liviano a propósito (24 bytes):
// guarda el índice del proceso en la tabla, no su nombre; el nombre se obtiene
// al imprimir. pid = -1 e indice = INDICE_OCIOSO indican CPU ociosa.
struct SegmentoGantt {
    std::size_t indice = INDICE_OCIOSO;
    int pid = -1;
    int inicio = 0;
    int fin = 0;

    SegmentoGantt() = default;
    SegmentoGantt(int pid_, std::size_t indice_, int inicio_, int fin_)
        : indice(indice_), pid(pid_), inicio(inicio_), fin(fin_) {}
};

// Un proceso que aún no ha llegado: se ordena por (llegada, posición en la tabla).
struct EntradaLlegada {
    int llegada = 0;
    std::size_t indice = 0;

    EntradaLlegada() = default;
    EntradaLlegada(int llegada_, std::size_t indice_) : llegada(llegada_), indice(indice_) {}
};

struct PrecedeLlegada {
    bool operator()(const EntradaLlegada& a, const EntradaLlegada& b) const {
        if (a.llegada != b.llegada) return a.llegada < b.llegada;
        return a.indice < b.indice;
    }
};

/**
 * Simulador
 *
 * Recibe una tabla de procesos y ejecuta cualquier Planificador sobre ella
 * avanzando el reloj turno a turno. Trabaja sobre su propia copia de la tabla,
 * así se pueden comparar varios algoritmos creando un Simulador por algoritmo
 * con el mismo conjunto de procesos.
 *
 * Los procesos que aún no han llegado esperan en una cola con prioridad ordenada
 * por llegada, así admitir llegadas cuesta O(log n) y no hay que recorrer la tabla.
 * El registro del diagrama de Gantt es opcional (registrarGantt) porque con
 * muchos procesos ocupa mucha memoria y nadie lo lee.
 *
 * Convención al coincidir eventos: las llegadas ocurridas durante un turno (o
 * justo cuando termina) entran a la cola de listos ANTES que el proceso que
 * fue expulsado por agotar su quantum.
 */
class Simulador {
public:
    explicit Simulador(const ArregloDinamico<Proceso>& procesos)
        : procesos_(procesos), tiempoTotal_(0), tiempoOcupado_(0) {
        long long sumaRafagas = 0;
        long long llegadaMaxima = 0;
        for (const Proceso& p : procesos_) {
            if (p.rafaga <= 0) {
                throw std::invalid_argument("Simulador: la rafaga del proceso " + p.nombre +
                                            " debe ser mayor que 0");
            }
            if (p.llegada < 0) {
                throw std::invalid_argument("Simulador: la llegada del proceso " + p.nombre +
                                            " no puede ser negativa");
            }
            sumaRafagas += p.rafaga;
            llegadaMaxima = std::max<long long>(llegadaMaxima, p.llegada);
        }
        // El reloj nunca pasa de (última llegada + suma de ráfagas): debe caber en int.
        if (sumaRafagas + llegadaMaxima > std::numeric_limits<int>::max()) {
            throw std::invalid_argument("Simulador: los tiempos de la simulacion exceden el rango de int");
        }
    }

    // Ejecuta la simulación completa con el planificador dado.
    void ejecutar(Planificador& planificador) {
        reiniciar();
        const std::size_t total = procesos_.tamano();
        std::size_t terminados = 0;
        int reloj = 0;

        while (terminados < total) {
            admitirLlegadas(planificador, reloj);

            // Sin procesos listos: la CPU queda ociosa hasta la próxima llegada.
            if (!planificador.hayListos()) {
                int proxima = proximaLlegada();
                if (proxima < 0) {
                    throw std::logic_error("Simulador: no hay procesos listos ni por llegar");
                }
                agregarSegmento(-1, INDICE_OCIOSO, reloj, proxima);
                reloj = proxima;
                continue;
            }

            std::size_t indice = planificador.siguiente();
            Proceso& p = procesos_[indice];
            if (p.inicio < 0) p.inicio = reloj;
            p.estado = EstadoProceso::Ejecutando;

            int limite = planificador.quantum(p);
            if (limite <= 0) {
                throw std::logic_error("Simulador: el planificador devolvio un quantum no positivo");
            }
            int uso = std::min(limite, p.restante);

            agregarSegmento(p.pid, indice, reloj, reloj + uso);
            reloj += uso;
            tiempoOcupado_ += uso;
            p.restante -= uso;

            // Llegadas durante el turno: entran antes que el proceso expulsado.
            admitirLlegadas(planificador, reloj);

            if (p.restante == 0) {
                p.estado = EstadoProceso::Terminado;
                p.fin = reloj;
                ++terminados;
            } else {
                p.estado = EstadoProceso::Listo;
                planificador.agregar(indice, p);    // vuelve a la cola de listos
            }
        }
        tiempoTotal_ = reloj;
    }

    // Activa o desactiva el registro del diagrama de Gantt (activo por defecto).
    // Con muchos procesos conviene desactivarlo: los tramos ocupan memoria y
    // las métricas no dependen de ellos.
    void registrarGantt(bool activo) { registrarGantt_ = activo; }
    bool registraGantt() const { return registrarGantt_; }

    // ---------- Resultados ----------

    const ArregloDinamico<Proceso>& procesos() const { return procesos_; }
    const ArregloDinamico<SegmentoGantt>& gantt() const { return gantt_; }
    int tiempoTotal() const { return tiempoTotal_; }
    int tiempoOcupado() const { return tiempoOcupado_; }

    // Porcentaje del tiempo total (desde t = 0) en que la CPU estuvo ocupada.
    double utilizacionCPU() const {
        return tiempoTotal_ == 0 ? 0.0 : 100.0 * tiempoOcupado_ / tiempoTotal_;
    }

    double esperaPromedio() const { return promedio(&Proceso::espera); }
    double retornoPromedio() const { return promedio(&Proceso::retorno); }
    double respuestaPromedio() const { return promedio(&Proceso::respuesta); }

    // Mayor tiempo de espera de un proceso: muestra el peor caso (por ejemplo, la
    // inanición de un proceso poco urgente), que el promedio puede ocultar.
    int esperaMaxima() const {
        int maxima = 0;
        for (const Proceso& p : procesos_) {
            maxima = std::max(maxima, p.espera());
        }
        return maxima;
    }

    // ---------- Salida por consola ----------

    // Valor para "sin límite" en los parámetros de cantidad de filas o de tramos.
    static constexpr std::size_t SIN_LIMITE = std::numeric_limits<std::size_t>::max();

    /**
     * Diagrama de Gantt: por cada bloque, una línea con los nombres y otra con los
     * instantes de inicio (y, al final, el instante en que termina el último tramo).
     *
     *   anchoMaximo  ancho máximo de línea en caracteres. Si el diagrama no cabe, se
     *                parte en varios bloques. 0 = todo en un solo bloque.
     *   maxTramos    solo se muestran los primeros maxTramos tramos.
     */
    void imprimirGantt(std::ostream& os, std::size_t anchoMaximo = 0,
                       std::size_t maxTramos = SIN_LIMITE) const {
        if (!registrarGantt_) {
            os << "(diagrama de Gantt desactivado)\n";
            return;
        }
        if (gantt_.estaVacio()) {
            os << "(sin ejecucion)\n";
            return;
        }
        const std::size_t total = gantt_.tamano();
        const std::size_t mostrados = std::min(total, maxTramos);
        if (mostrados == 0) {
            os << "(sin tramos que mostrar)\n";
            return;
        }

        std::size_t ancho = 5;
        for (std::size_t i = 0; i < mostrados; ++i) {
            ancho = std::max(ancho, etiquetaDeSegmento(gantt_[i]).size() + 3);
        }
        const int celda = static_cast<int>(ancho);

        // Tramos por bloque: se deja sitio para el "|" final y hasta 10 dígitos del último instante.
        std::size_t porBloque = mostrados;
        if (anchoMaximo > 0) {
            const std::size_t reservado = 11;
            const std::size_t disponible = anchoMaximo > reservado ? anchoMaximo - reservado : 0;
            porBloque = std::max<std::size_t>(1, disponible / ancho);
        }

        const std::ios_base::fmtflags banderas = os.flags();
        os << std::left;
        for (std::size_t desde = 0; desde < mostrados; desde += porBloque) {
            const std::size_t hasta = std::min(mostrados, desde + porBloque);
            if (desde > 0) os << "\n";
            for (std::size_t i = desde; i < hasta; ++i) {
                os << "| " << std::setw(celda - 2) << etiquetaDeSegmento(gantt_[i]);
            }
            os << "|\n";
            for (std::size_t i = desde; i < hasta; ++i) {
                os << std::setw(celda) << gantt_[i].inicio;
            }
            os << gantt_[hasta - 1].fin << "\n";
        }
        if (mostrados < total) {
            os << "... se omiten " << (total - mostrados) << " tramos mas (se muestran "
               << mostrados << " de " << total << ")\n";
        }
        os.flags(banderas);
    }

    // Tabla de entrada: lo que se va a simular (menor prioridad = más urgente).
    // Solo se muestran las primeras maxFilas filas. El ancho de cada columna se
    // adapta a los valores mostrados, así que las columnas nunca se pegan.
    void imprimirProcesos(std::ostream& os, std::size_t maxFilas = SIN_LIMITE) const {
        const std::size_t total = procesos_.tamano();
        const std::size_t mostradas = std::min(total, maxFilas);
        if (mostradas == 0 && total > 0) {
            os << "(" << total << " procesos; se omite el detalle)\n";
            return;
        }

        int wPid = 3, wNombre = 8, wLlegada = 7, wRafaga = 6, wPrioridad = 9;
        for (std::size_t i = 0; i < mostradas; ++i) {
            const Proceso& p = procesos_[i];
            wPid = std::max(wPid, anchoNumero(p.pid));
            wNombre = std::max(wNombre, static_cast<int>(p.nombre.size()));
            wLlegada = std::max(wLlegada, anchoNumero(p.llegada));
            wRafaga = std::max(wRafaga, anchoNumero(p.rafaga));
            wPrioridad = std::max(wPrioridad, anchoNumero(p.prioridad));
        }

        const std::ios_base::fmtflags banderas = os.flags();
        os << std::left << std::setw(wPid + 2) << "PID" << std::setw(wNombre + 2) << "Nombre"
           << std::right << std::setw(wLlegada + 2) << "Llegada" << std::setw(wRafaga + 2)
           << "Rafaga" << std::setw(wPrioridad + 2) << "Prioridad" << "\n";
        for (std::size_t i = 0; i < mostradas; ++i) {
            const Proceso& p = procesos_[i];
            os << std::left << std::setw(wPid + 2) << p.pid << std::setw(wNombre + 2) << p.nombre
               << std::right << std::setw(wLlegada + 2) << p.llegada << std::setw(wRafaga + 2)
               << p.rafaga << std::setw(wPrioridad + 2) << p.prioridad << "\n";
        }
        if (mostradas < total) {
            os << "... se omiten " << (total - mostradas) << " procesos mas (se muestran "
               << mostradas << " de " << total << ")\n";
        }
        os.flags(banderas);
    }

    // Tabla de resultados por proceso (solo las primeras maxFilas filas) seguida del
    // resumen. El ancho de cada columna se adapta a los valores mostrados.
    void imprimirMetricas(std::ostream& os, std::size_t maxFilas = SIN_LIMITE) const {
        const std::size_t total = procesos_.tamano();
        const std::size_t mostradas = std::min(total, maxFilas);

        const std::ios_base::fmtflags banderas = os.flags();
        if (mostradas == 0 && total > 0) {
            os << "(" << total << " procesos; se omite el detalle)\n";
        } else {
            int wPid = 3, wNombre = 8, wLlegada = 7, wRafaga = 6, wInicio = 6, wFin = 4;
            int wRetorno = 7, wEspera = 6, wRespuesta = 9;
            for (std::size_t i = 0; i < mostradas; ++i) {
                const Proceso& p = procesos_[i];
                wPid = std::max(wPid, anchoNumero(p.pid));
                wNombre = std::max(wNombre, static_cast<int>(p.nombre.size()));
                wLlegada = std::max(wLlegada, anchoNumero(p.llegada));
                wRafaga = std::max(wRafaga, anchoNumero(p.rafaga));
                wInicio = std::max(wInicio, anchoNumero(p.inicio));
                wFin = std::max(wFin, anchoNumero(p.fin));
                wRetorno = std::max(wRetorno, anchoNumero(p.retorno()));
                wEspera = std::max(wEspera, anchoNumero(p.espera()));
                wRespuesta = std::max(wRespuesta, anchoNumero(p.respuesta()));
            }

            os << std::left << std::setw(wPid + 2) << "PID" << std::setw(wNombre + 2) << "Nombre"
               << std::right << std::setw(wLlegada + 2) << "Llegada" << std::setw(wRafaga + 2)
               << "Rafaga" << std::setw(wInicio + 2) << "Inicio" << std::setw(wFin + 2) << "Fin"
               << std::setw(wRetorno + 2) << "Retorno" << std::setw(wEspera + 2) << "Espera"
               << std::setw(wRespuesta + 2) << "Respuesta" << "\n";
            for (std::size_t i = 0; i < mostradas; ++i) {
                const Proceso& p = procesos_[i];
                os << std::left << std::setw(wPid + 2) << p.pid << std::setw(wNombre + 2)
                   << p.nombre << std::right << std::setw(wLlegada + 2) << p.llegada
                   << std::setw(wRafaga + 2) << p.rafaga << std::setw(wInicio + 2) << p.inicio
                   << std::setw(wFin + 2) << p.fin << std::setw(wRetorno + 2) << p.retorno()
                   << std::setw(wEspera + 2) << p.espera() << std::setw(wRespuesta + 2)
                   << p.respuesta() << "\n";
            }
            if (mostradas < total) {
                os << "... se omiten " << (total - mostradas) << " procesos mas (se muestran "
                   << mostradas << " de " << total << ")\n";
            }
        }
        os.flags(banderas);

        os << "\n";
        imprimirResumen(os);
    }

    // Totales y promedios de toda la simulación (no depende de cuántos procesos se listen).
    void imprimirResumen(std::ostream& os) const {
        const std::ios_base::fmtflags banderas = os.flags();
        const std::streamsize precision = os.precision();

        os << std::left << std::setw(21) << "Procesos:" << std::right << std::setw(10)
           << procesos_.tamano() << "\n";
        os << std::left << std::setw(21) << "Tiempo total:" << std::right << std::setw(10)
           << tiempoTotal_ << "\n";
        os << std::left << std::setw(21) << "CPU ocupada:" << std::right << std::setw(10)
           << tiempoOcupado_ << "\n";

        os << std::fixed << std::setprecision(2);
        os << std::left << std::setw(21) << "Uso de CPU:" << std::right << std::setw(10)
           << utilizacionCPU() << " %\n";
        os << std::left << std::setw(21) << "Espera promedio:" << std::right << std::setw(10)
           << esperaPromedio() << "\n";
        os << std::left << std::setw(21) << "Retorno promedio:" << std::right << std::setw(10)
           << retornoPromedio() << "\n";
        os << std::left << std::setw(21) << "Respuesta promedio:" << std::right << std::setw(10)
           << respuestaPromedio() << "\n";
        os << std::left << std::setw(21) << "Espera maxima:" << std::right << std::setw(10)
           << esperaMaxima() << "\n";

        os.flags(banderas);
        os.precision(precision);
    }
private:
    ArregloDinamico<Proceso> procesos_;
    ArregloDinamico<SegmentoGantt> gantt_;
    int tiempoTotal_;
    int tiempoOcupado_;
    ColaPrioridad<EntradaLlegada, PrecedeLlegada> porLlegar_;   // procesos que aún no llegan
    bool registrarGantt_ = true;

    static std::string etiquetaDe(const Proceso& p) {
        return p.nombre.empty() ? "P" + std::to_string(p.pid) : p.nombre;
    }

    // Cantidad de caracteres que ocupa un entero al imprimirlo (incluye el signo).
    static int anchoNumero(int valor) {
        long long v = valor;
        int caracteres = 1;
        if (v < 0) {
            ++caracteres;
            v = -v;
        }
        while (v >= 10) {
            v /= 10;
            ++caracteres;
        }
        return caracteres;
    }

    // Texto de un tramo del Gantt: nombre del proceso o "--" si la CPU estuvo ociosa.
    std::string etiquetaDeSegmento(const SegmentoGantt& s) const {
        return s.indice == INDICE_OCIOSO ? std::string("--") : etiquetaDe(procesos_[s.indice]);
    }

    void reiniciar() {
        for (Proceso& p : procesos_) {
            p.restante = p.rafaga;
            p.estado = EstadoProceso::Nuevo;
            p.inicio = -1;
            p.fin = -1;
        }
        gantt_.limpiar();
        tiempoTotal_ = 0;
        tiempoOcupado_ = 0;
        porLlegar_.limpiar();
        for (std::size_t i = 0; i < procesos_.tamano(); ++i) {
            porLlegar_.insertarUltimo(EntradaLlegada(procesos_[i].llegada, i));
        }
    }

    // Entrega al planificador todos los procesos cuya llegada ya ocurrió, en orden
    // de llegada y, si empatan, de posición en la tabla. Cada proceso sale de la
    // cola de llegadas una sola vez: O(log n) por proceso.
    void admitirLlegadas(Planificador& planificador, int reloj) {
        while (!porLlegar_.estaVacia() && porLlegar_.primero().llegada <= reloj) {
            const std::size_t indice = porLlegar_.borrarPrimero().indice;
            procesos_[indice].estado = EstadoProceso::Listo;
            planificador.agregar(indice, procesos_[indice]);
        }
    }

    // Menor llegada entre los procesos que todavía no han llegado (-1 si no hay).
    int proximaLlegada() const {
        return porLlegar_.estaVacia() ? -1 : porLlegar_.primero().llegada;
    }

    // Añade un tramo al Gantt, fusionándolo con el anterior si es del mismo
    // proceso y es contiguo (por ejemplo, cuando un proceso queda solo en la cola).
    void agregarSegmento(int pid, std::size_t indice, int inicio, int fin) {
        if (!registrarGantt_) return;
        if (!gantt_.estaVacio()) {
            SegmentoGantt& ultimo = gantt_[gantt_.tamano() - 1];
            if (ultimo.indice == indice && ultimo.fin == inicio) {
                ultimo.fin = fin;
                return;
            }
        }
        gantt_.agregar(SegmentoGantt(pid, indice, inicio, fin));
    }

    double promedio(int (Proceso::*metrica)() const) const {
        if (procesos_.estaVacio()) return 0.0;
        double suma = 0.0;
        for (const Proceso& p : procesos_) {
            suma += (p.*metrica)();
        }
        return suma / static_cast<double>(procesos_.tamano());
    }
};

#endif 