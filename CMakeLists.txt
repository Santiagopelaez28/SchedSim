cmake_minimum_required(VERSION 3.16)

project(SchedSim VERSION 1.0.0 LANGUAGES CXX)

# SchedSim usa C++17 (std::optional, variables inline, etc.).
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Si no se indica otra cosa, se compila optimizado.
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Tipo de compilacion" FORCE)
endif()

option(SCHEDSIM_TESTS "Compilar las pruebas (se ejecutan con ctest)" ON)
option(SCHEDSIM_SANITIZERS
       "Compilar con AddressSanitizer y UBSan para detectar errores de memoria (solo GCC/Clang en Linux o macOS)"
       OFF)

# Opciones comunes: advertencias activadas y, si se pide, sanitizadores.
add_library(schedsim_opciones INTERFACE)
if(MSVC)
  target_compile_options(schedsim_opciones INTERFACE /W4 /utf-8)
else()
  target_compile_options(schedsim_opciones INTERFACE -Wall -Wextra -Wpedantic)
  if(SCHEDSIM_SANITIZERS)
    target_compile_options(schedsim_opciones INTERFACE -fsanitize=address,undefined -fno-omit-frame-pointer -g)
    target_link_options(schedsim_opciones INTERFACE -fsanitize=address,undefined)
  endif()
endif()

# Programa principal. Todo el codigo es de solo cabeceras (struct/, core/, gui/),
# asi que basta con main.cpp.
add_executable(schedsim main.cpp)
target_link_libraries(schedsim PRIVATE schedsim_opciones)

# Pruebas: cada pruebas/prueba_*.cpp es un ejecutable y una prueba de CTest.
if(SCHEDSIM_TESTS)
  enable_testing()
  file(GLOB SCHEDSIM_PRUEBAS CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/pruebas/prueba_*.cpp")
  foreach(fuente IN LISTS SCHEDSIM_PRUEBAS)
    get_filename_component(nombre "${fuente}" NAME_WE)
    add_executable(${nombre} "${fuente}")
    target_link_libraries(${nombre} PRIVATE schedsim_opciones)
    add_test(NAME ${nombre} COMMAND ${nombre})
  endforeach()
endif()
