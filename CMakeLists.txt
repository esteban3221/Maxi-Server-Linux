cmake_minimum_required(VERSION 3.25)
project("Maxicajero Server Local" VERSION 2.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -fPIC -march=native -flto=auto -fno-semantic-interposition")
string(TOLOWER "${CMAKE_SYSTEM_PROCESSOR}" ARCH_NAME)
set(EXECUTABLE_NAME "Maxicajero-Server-${ARCH_NAME}")

# --- Obtener número de Build desde Git ---
find_package(Git QUIET)
if(GIT_FOUND)
    execute_process(
        COMMAND ${GIT_EXECUTABLE} rev-list --count HEAD
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        OUTPUT_VARIABLE PROJECT_VERSION_BUILD
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
endif()

# Si no hay git o es un repo nuevo, inicializamos en 0
if(NOT PROJECT_VERSION_BUILD)
    set(PROJECT_VERSION_BUILD 0)
endif()

# Generar el archivo hpp con los datos de CMake
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/include/version.hpp.in
    ${CMAKE_CURRENT_BINARY_DIR}/include/config/version.hpp
    @ONLY
)

message(STATUS "Proyecto: ${PROJECT_NAME} v${PROJECT_VERSION} (Build ${PROJECT_VERSION_BUILD})")

option(ENABLE_PRETTY_JSON "Activar formato visual para JSON en logs" ON)

if(ENABLE_PRETTY_JSON)
    add_definitions(-DMODO_DEBUG_JSON)
endif()

# Configurar archivo de versión
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/include/version.hpp.in
    ${CMAKE_CURRENT_BINARY_DIR}/include/config/version.hpp
)

find_package(Crow REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(cpr REQUIRED)
pkg_check_modules(GTKMM REQUIRED gtkmm-4.0)
pkg_check_modules(LIBNOTIFY REQUIRED libnotify)
find_package(SQLite3 REQUIRED)
find_package(GTest REQUIRED)

# include_directories(
#     ${GTKMM_INCLUDE_DIRS}
#     ${LIBNOTIFY_INCLUDE_DIRS}
#     ${CMAKE_CURRENT_BINARY_DIR}/include
#     ${CMAKE_CURRENT_SOURCE_DIR}/include
# )

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/include/version.hpp.in
    ${CMAKE_CURRENT_BINARY_DIR}/include/config/version.hpp
)

set(SOURCES
    src/main.cpp
    #===================MODEL===================
    src/model/log.cpp
    src/model/level_cash.cpp
    src/model/log_historial.cpp
    src/model/pago_pendiente.cpp
    src/model/pago_terminal.cpp
    src/model/roles.cpp
    src/model/usuario_historial.cpp
    src/model/usuarios_roles.cpp
    src/model/usuarios.cpp
    src/model/configuracion.cpp
    src/model/terminales.cpp
    src/model/detalle_movimiento.cpp
    #===================VIEW====================
    src/view/base.venta_pago.cpp
    src/view/base.nip.cpp
    src/view/main_window.cpp
    src/view/refill.cpp
    src/view/config.cpp
    src/view/carrousel.cpp
    src/view/cortinilla_carga.cpp

    src/view/config/base.validador.cpp
    src/view/config/validador.cpp

    src/view/config/empresa.cpp
    src/view/config/general.cpp
    src/view/config/impresora.cpp
    src/view/config/info.cpp
    src/view/config/roles.cpp
    src/view/config/wifi.cpp
    src/view/metodo_pago.cpp
    #================CONTROLLER=================
    src/controller/session.cpp
    src/controller/main_window.cpp
    src/controller/pago.cpp
    src/controller/pago_manual.cpp
    src/controller/refill.cpp
    src/controller/config.cpp
    src/controller/configuracion.cpp
    src/controller/nip.cpp
    src/controller/nuevo_nip.cpp
    src/controller/log.cpp
    src/controller/config/terminal.cpp
    src/controller/config/print_operation.cpp

    src/controller/config/validador.cpp
    src/controller/config/empresa.cpp
    src/controller/config/general.cpp
    src/controller/config/impresora.cpp
    src/controller/config/info.cpp
    src/controller/config/roles.cpp
    src/controller/config/wifi.cpp

    src/controller/venta/efectivo.cpp
    src/controller/venta/tarjeta.cpp
    src/controller/venta/metodo_pago.cpp
    src/controller/venta/dialmonto.cpp
    #================GLOBAL=================
    src/shared/global.cpp
    #================STATE MACHINE=================
    src/core/context/validador.cpp
    src/core/hub_cash.cpp
)

include_directories(include
    include/core
    include/core/context
    include/core/interfaces
    include/core/states
    include/controller
    include/controller/venta
    include/view
    include/view/config
    include/db
    include/model
    include/shared
)

add_subdirectory(lib/Sqlit3-cpp-lib/)
add_subdirectory(lib/eSSP)

add_executable(${EXECUTABLE_NAME} ${SOURCES})

target_link_libraries(${EXECUTABLE_NAME}
    PUBLIC
    Sqlite-cpp
    eSSP_lib
    ${GTKMM_LIBRARIES}
    ${LIBNOTIFY_LIBRARIES}
    cpr::cpr
    Crow::Crow
)
target_include_directories(${EXECUTABLE_NAME} PRIVATE
    ${GTKMM_INCLUDE_DIRS}
    ${LIBNOTIFY_INCLUDE_DIRS}
    ${CMAKE_CURRENT_BINARY_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    lib/eSSP/inc
    lib/eSSP/lib
)

enable_testing()
add_subdirectory(test)
