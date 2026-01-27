cmake_minimum_required(VERSION 3.25)
project("Maxicajero Server Local" VERSION 2.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -fPIC")

message(STATUS "Proyecto: ${PROJECT_NAME}")
message(STATUS "Versión: ${PROJECT_VERSION}")
message(STATUS "Versión mayor: ${PROJECT_VERSION_MAJOR}")
message(STATUS "Versión menor: ${PROJECT_VERSION_MINOR}")
message(STATUS "Parche: ${PROJECT_VERSION_PATCH}")

# Configurar archivo de versión
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/include/version.hpp.in
    ${CMAKE_CURRENT_BINARY_DIR}/include/config/version.hpp
)

find_package(Crow REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(cpr REQUIRED)
pkg_check_modules(GTKMM gtkmm-4.0)
pkg_check_modules(LIBNOTIFY REQUIRED libnotify)
find_package(SQLite3 REQUIRED)
find_package(GTest REQUIRED)

include_directories(
    ${GTKMM_INCLUDE_DIRS}
    ${LIBNOTIFY_INCLUDE_DIRS}
    ${CMAKE_CURRENT_BINARY_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

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
    src/model/roles.cpp
    src/model/usuario_historial.cpp
    src/model/usuarios_roles.cpp
    src/model/usuarios.cpp
    src/model/configuracion.cpp
    #===================VIEW====================
    src/view/base.venta_pago.cpp
    src/view/base.nip.cpp
    src/view/main_window.cpp
    src/view/refill.cpp
    src/view/config.cpp
    src/view/carrousel.cpp

    src/view/config/base.validador.cpp
    src/view/config/validador.cpp

    src/view/config/empresa.cpp
    src/view/config/general.cpp
    src/view/config/impresora.cpp
    src/view/config/info.cpp
    src/view/config/roles.cpp
    src/view/config/wifi.cpp
    #================CONTROLLER=================
    src/controller/c_sharp_validator_validator.cpp
    src/controller/session.cpp
    src/controller/main_window.cpp
    src/controller/venta.cpp
    src/controller/pago.cpp
    src/controller/pago_manual.cpp
    src/controller/refill.cpp
    src/controller/config.cpp
    src/controller/configuracion.cpp
    src/controller/nip.cpp
    src/controller/nuevo_nip.cpp
    src/controller/log.cpp

    src/controller/config/validador.cpp

    src/controller/config/empresa.cpp
    src/controller/config/general.cpp
    src/controller/config/impresora.cpp
    src/controller/config/info.cpp
    src/controller/config/roles.cpp
    src/controller/config/wifi.cpp
    #================GLOBAL=================
    src/shared/global.cpp
)

include_directories(include 
                    include/controller
                    include/controller/accion
                    include/view
                    include/view/config
                    include/db
                    include/model
                    include/shared
)

add_subdirectory(lib/Sqlit3-cpp-lib/)

add_executable(Maxicajero ${SOURCES})

target_link_libraries(Maxicajero 
                        PUBLIC 
                        Sqlite-cpp
                        ${GTKMM_LIBRARIES} 
                        ${LIBNOTIFY_LIBRARIES}
                        cpr::cpr
                        Crow::Crow
)
target_include_directories(Maxicajero PRIVATE ${GTKMM_INCLUDE_DIRS})

enable_testing()
add_subdirectory(test)
