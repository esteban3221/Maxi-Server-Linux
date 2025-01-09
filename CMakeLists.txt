cmake_minimum_required(VERSION 3.25)
project(Backend)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG") 

find_package(Crow REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(cpr REQUIRED)
pkg_check_modules(GTKMM gtkmm-4.0)
pkg_check_modules(LIBNOTIFY REQUIRED libnotify)
find_package(SQLite3 REQUIRED)
find_package(GTest REQUIRED) # Google Test instalado en el sistema

include_directories(
    ${GTKMM_INCLUDE_DIRS}
    ${LIBNOTIFY_INCLUDE_DIRS}
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
    #===================VIEW====================
    src/view/base/venta_pago.cpp
    src/view/main_window.cpp
    #================CONTROLLER=================
    src/controller/c_sharp_validator_validator.cpp
    src/controller/session.cpp
    src/controller/main_window.cpp
    src/controller/venta.cpp
    src/controller/pago.cpp
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

# Habilitar pruebas
enable_testing()
add_subdirectory(test)
