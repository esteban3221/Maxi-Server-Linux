cmake_minimum_required(VERSION 3.25)
project(Backend)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG") 

find_package(Crow REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(cpr REQUIRED)
find_package(nlohmann_json 3.2.0 REQUIRED)
pkg_check_modules(GTKMM gtkmm-4.0)
pkg_check_modules(LIBNOTIFY REQUIRED libnotify)
find_package(jwt-cpp REQUIRED)
find_package(SQLite3 REQUIRED)


include_directories(
    ${jwt-cpp_INCLUDE_DIRS}
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
    src/view/main_window.cpp
    #================CONTROLLER=================
    
    src/controller/validator.cpp

    #================GLOBAL=================
    src/shared/global.cpp
    #TEST
    src/controller/rest_api.cpp
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
target_include_directories(Sqlite-cpp PUBLIC include)

add_executable(Maxicajero ${SOURCES})

target_link_libraries(Maxicajero 
                        PUBLIC 
                        Sqlite-cpp
                        ${GTKMM_LIBRARIES} 
                        ${LIBNOTIFY_LIBRARIES}
                        nlohmann_json::nlohmann_json 
                        cpr::cpr
                        Crow::Crow
)
target_include_directories(Maxicajero PRIVATE ${GTKMM_INCLUDE_DIRS})