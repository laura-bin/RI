#pragma once
/** *************************************************************************************
 * Exercice sur la serialisation
 * =============================
 *
 * Programmation d'une bibliotheque de fonctions destinees
 * a la serialisation de donnees (integer, float, double, etc.)
 *
 * RI 2020 - Laura Binacchi - Fedora 32
 ****************************************************************************************/

#include <stdint.h>

/**
 * Writes a given 16 bytes unsigned integer in the buffer
 *
 * @param i: input data
 * @param out_buf: output buffer
 *
 * @return the next free space in the buffer
 */
char *write_u16(uint16_t i, char *out_buf);

/**
 * Writes a given 32 bytes unsigned integer in the buffer
 *
 * @param i: input data
 * @param out_buf: output buffer
 *
 * @return the next free space in the buffer
 */
char *write_u32(uint32_t i, char *out_buf);

/**
 * Writes a given 64 bytes unsigned integer in the buffer
 *
 * @param i: input data
 * @param out_buf: output buffer
 *
 * @return the next free space in the buffer
 */
char *write_u64(uint64_t i, char *out_buf);

/**
 * Writes a given 32 bytes float in the buffer
 *
 * @param f: input data
 * @param out_buf: output buffer
 *
 * @return the next free space in the buffer
 */
char *write_f32(float f, char *out_buf);

/**
 * Writes a given 64 bytes float (double) in the buffer
 *
 * @param d: input data
 * @param out_buf: output buffer
 *
 * @return the next free space in the buffer
 */
char *write_f64(double d, char *out_buf);

/**
 * Writes a given string in the buffer
 *
 * @param str: input data
 * @param out_buf: output buffer
 *
 * @return the next free space in the buffer
 */
char *write_str(char *str, char *out_buf);



/**
 * Reads a 16 bytes unsigned integer from the buffer
 *
 * @param buf: input buffer
 * @param out_i: output integer
 *
 * @return the next space to read from the buffer
 */
char *read_u16(char *buf, uint16_t *out_i);

/**
 * Reads a 32 bytes unsigned integer from the buffer
 *
 * @param buf: input buffer
 * @param out_i: output integer
 *
 * @return the next space to read from the buffer
 */
char *read_u32(char *buf, uint32_t *out_i);

/**
 * Reads a 64 bytes unsigned integer from the buffer
 *
 * @param buf: input buffer
 * @param out_i: output integer
 *
 * @return the next space to read from the buffer
 */
char *read_u64(char *buf, uint64_t *out_i);

/**
 * Reads a 32 bytes float from the buffer
 *
 * @param buf: input buffer
 * @param out_f: output float
 *
 * @return the next space to read from the buffer
 */
char *read_f32(char *buf, float *out_f);

/**
 * Reads a 64 bytes float (double) from the buffer
 *
 * @param buf: input buffer
 * @param out_d: output double
 *
 * @return the next space to read from the buffer
 */
char *read_f64(char *buf, double *out_d);

/**
 * Reads a string prefixed by its size from the buffer
 *
 * @param buf: input buffer
 * @param out_str: output string
 *
 * @return the next space to read from the buffer
 */
char *read_str(char *buf, char **out_str);
