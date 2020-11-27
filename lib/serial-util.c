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
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "serial-util.h"

#define MANTISSA_F32_BITS (32 - 8)
#define MANTISSA_F64_BITS (64 - 11)

/* WRITE FUNCTIONS */

char *write_u16(uint16_t i, char *out_buf) {
    i = htobe16(i);
    memcpy(out_buf, &i, sizeof(uint16_t));
    return out_buf + sizeof(uint16_t);
}

char *write_u32(uint32_t i, char *out_buf) {
    i = htobe32(i);
    memcpy(out_buf, &i, sizeof(uint32_t));
    return out_buf + sizeof(uint32_t);
}

char *write_u64(uint64_t i, char *out_buf) {
    i = htobe64(i);
    memcpy(out_buf, &i, sizeof(uint64_t));
    return out_buf + sizeof(uint64_t);
}

char *write_f32(float f, char *out_buf) {
    float un_exponentiated_mantissa;
    int exponent;
    int32_t mantissa;

    un_exponentiated_mantissa = frexpf(f, &exponent);
    out_buf = write_u16(exponent, out_buf);

    mantissa = ldexpf(un_exponentiated_mantissa, MANTISSA_F32_BITS);
    return write_u32(mantissa, out_buf);
}

char *write_f64(double d, char *out_buf) {
    double un_exponentiated_mantissa;
    int exponent;
    int64_t mantissa;

    un_exponentiated_mantissa = frexp(d, &exponent);
    out_buf = write_u16(exponent, out_buf);

    mantissa = ldexp(un_exponentiated_mantissa, MANTISSA_F64_BITS);
    return write_u64(mantissa, out_buf);
}

char *write_str(char *str, char *out_buf) {
    size_t len = strlen(str);
    out_buf = write_u64(len, out_buf);
    memcpy(out_buf, str, len);
    return out_buf + len;
}

/* READ FUNCTIONS */

char *read_u16(char *buf, uint16_t *out_i) {
    memcpy(out_i, buf, sizeof(uint16_t));
    *out_i = be16toh(*out_i);
    return buf + sizeof(uint16_t);
}

char *read_u32(char *buf, uint32_t *out_i) {
    memcpy(out_i, buf, sizeof(uint32_t));
    *out_i = be32toh(*out_i);
    return buf + sizeof(uint32_t);
}

char *read_u64(char *buf, uint64_t *out_i) {
    memcpy(out_i, buf, sizeof(uint64_t));
    *out_i = be64toh(*out_i);
    return buf + sizeof(uint64_t);
}

char *read_f32(char *buf, float *out_f) {
    int16_t exponent;
    int32_t mantissa;
    buf = read_u16(buf, (uint16_t*) &exponent);
    buf = read_u32(buf, (uint32_t*) &mantissa);

    *out_f = ldexpf(ldexpf(mantissa, -MANTISSA_F32_BITS), exponent);

    return buf;
}

char *read_f64(char *buf, double *out_d) {
    int16_t exponent;
    int64_t mantissa;
    buf = read_u16(buf, (uint16_t*) &exponent);
    buf = read_u64(buf, (uint64_t*) &mantissa);

    *out_d = ldexp(ldexp(mantissa, -MANTISSA_F64_BITS), exponent);

    return buf;
}

char *read_str(char *buf, char **out_str) {
    size_t len;
    buf = read_u64(buf, &len);
    *out_str = malloc(len);
    memcpy(*out_str, buf, len);

    return buf + len;
}
