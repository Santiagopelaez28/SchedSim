#include <exception>
#include <iostream>

#include "gui/MenuConsola.h"

// SchedSim V1: simulador de planificacion de procesos (consola).
// Toda la interaccion vive en MenuConsola; main solo lo crea y lo ejecuta.
int main() {
    try {
        MenuConsola menu(std::cin, std::cout);
        return menu.ejecutar();
    } catch (const std::exception& e) {
        std::cerr << "Error inesperado: " << e.what() << "\n";
        return 1;
    }
}
