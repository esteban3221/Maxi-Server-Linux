# Declarar que este subproyecto soporta C
project(eSSP LANGUAGES C)

# Definir los archivos fuente (basado en tu árbol de directorios)
set(ESSP_SOURCES
    lib/Encryption.c
    lib/ITLSSPProc.c
    lib/Random.c
    lib/serialfunc.c
    lib/ssp_commands.c
    lib/SSPComs.c
    lib/SSPDownload.c
)

# Crear la librería estática
add_library(eSSP_lib STATIC ${ESSP_SOURCES})

# Configurar los directorios de inclusión para que otros puedan encontrarlos
target_include_directories(eSSP_lib PUBLIC 
    inc 
    lib
)

# Como es un SDK viejo, es probable que necesite ignorar algunos warnings
if(MSVC)
    target_compile_options(eSSP_lib PRIVATE /W0)
else()
    target_compile_options(eSSP_lib PRIVATE -w)
endif()