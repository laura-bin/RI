/****************************************************************************************
* Exercice sur la serialisation
* =============================
*
* Programmation d'une bibliotheque de fonctions destinees 
* a la serialisation de donnees (integer, float, double, etc).
*
* RI 2020 - Laura Binacchi - Fedora 32
****************************************************************************************/

// https://linux.die.net/man/3/be64toh
#include <endian.h>
#include <string.h>
#include <math.h>

#include "serial-util.h"

#define MANTISSA_F32_BITS (32 - 8)
#define MANTISSA_F64_BITS (64 - 11)

/* WRITE FUNCTIONS */

char *write_u16(char *buffer, uint16_t i) {
    i = htobe16(i);
    memcpy(buffer, &i, sizeof(uint16_t));
    return buffer + sizeof(uint16_t);
}

char *write_u32(char *buffer, uint32_t i) {
    i = htobe32(i);
    memcpy(buffer, &i, sizeof(uint32_t));
    return buffer + sizeof(uint32_t);
}

char *write_u64(char *buffer, uint64_t i) {
    i = htobe64(i);
    memcpy(buffer, &i, sizeof(uint64_t));
    return buffer + sizeof(uint64_t);
}

char *write_f32(char *buffer, float f) {
    float un_exponentiated_mantissa;
    int exponent;
    int32_t mantissa;

    un_exponentiated_mantissa = frexpf(f, &exponent);
    buffer = write_u16(buffer, exponent);

    mantissa = ldexpf(un_exponentiated_mantissa, MANTISSA_F32_BITS);
    return write_u32(buffer, mantissa);
}

char *write_f64(char *buffer, double d) {
    double un_exponentiated_mantissa;
    int exponent;
    int64_t mantissa;

    un_exponentiated_mantissa = frexp(d, &exponent);
    buffer = write_u16(buffer, exponent);

    mantissa = ldexp(un_exponentiated_mantissa, MANTISSA_F64_BITS);
    return write_u64(buffer, mantissa);
}

char *write_str(char *buffer, char *str) {
    size_t length = strlen(str);
    buffer = write_u64(buffer, length);
    memcpy(buffer, str, length);
    return buffer + length;
}

/* READ FUNCTIONS */

char *read_u16(char *buffer, uint16_t *i) {
    memcpy(&i, buffer, sizeof(uint16_t));
    i = be16toh(i);
    return buffer + sizeof(uint16_t);
}

char *read_u32(char *buffer, uint32_t *i) {
    memcpy(&i, buffer, sizeof(uint32_t));
    i = be32toh(i);
    return buffer + sizeof(uint32_t);
}

char *read_u64(char *buffer, uint64_t *i) {
    memcpy(&i, buffer, sizeof(uint64_t));
    i = be64toh(i);
    return buffer + sizeof(uint64_t);
}

char *read_f32(char *buffer, float *f) {
    int16_t exponent;
    int32_t mantissa;
    buffer = read_u16(buffer, &exponent);
    buffer = read_u32(buffer, &mantissa);

    *f = ldexpf(ldexpf(mantissa, -MANTISSA_F32_BITS), exponent);

    return buffer;
}

char *read_f64(char *buffer, double *d) {
    int16_t exponent;
    int64_t mantissa;
    buffer = read_u16(buffer, &exponent);
    buffer = read_u64(buffer, &mantissa);

    *d = ldexp(ldexp(mantissa, -MANTISSA_F64_BITS), exponent);

    return buffer;
}

char *read_str(char *buffer, char **str) {
    size_t length;
    buffer = read_u64(buffer, length);
    *str = malloc(length);
    memcpy(*str, buffer, length);

    return buffer + length;
}
