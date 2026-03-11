#ifndef __ITL_TYPES_H
#define __ITL_TYPES_H

/* * Modernización de itl_types.h para compatibilidad C++20 y GLib
 */

#ifdef __cplusplus
    #include <cstdint>
#else
    #include <stdint.h>
    #include <stdbool.h>
#endif

// Solo definimos TRUE/FALSE si no vienen ya de glib.h o similares
#ifndef TRUE
    #define TRUE 1
#endif
#ifndef FALSE
    #define FALSE 0
#endif

// Mapeo de tipos para el SDK viejo usando estándares modernos (stdint.h)
typedef int8_t   INT8;
typedef uint8_t  UINT8;
typedef volatile int8_t  VINT8;
typedef volatile uint8_t VUINT8;

typedef int16_t  INT16;
typedef uint16_t UINT16;
typedef volatile int16_t VINT16;
typedef volatile uint16_t VUINT16;

typedef int32_t  INT32;
typedef uint32_t UINT32;
typedef volatile int32_t VINT32;
typedef volatile uint32_t VUINT32;

typedef int64_t  INT64;
typedef uint64_t UINT64;
typedef volatile int64_t VINT64;
typedef volatile uint64_t VUINT64;

typedef int SSP_PORT;

#endif