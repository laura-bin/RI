#pragma once
/****************************************************************************************
* Exercice sur la serialisation
* =============================
*
* Programmation d'une bibliotheque de fonctions destinees 
* a la serialisation de differents types de données
*
* RI 2020 - Laura Binacchi - Fedora 32
****************************************************************************************/

#include <stdint.h>

/**
 * Writes a 16 bytes unsigned int in the buffer
 * 
 * @param buffer
 * @param i: data writen
 *
 * @return the next free space in the buffer
 */
char *write_u16(char *buffer, uint16_t i);

/**
 * Writes a 32 bytes unsigned int in the buffer
 * 
 * @param buffer
 * @param i: data writen
 *
 * @return the next free space in the buffer
 */
char *write_u32(char *buffer, uint32_t i);

/**
 * Writes a 64 bytes unsigned int in the buffer
 * 
 * @param buffer
 * @param i: data writen
 *
 * @return the next free space in the buffer
 */
char *write_u64(char *buffer, uint64_t i);

/**
 * Writes a 32 bytes float in the buffer
 * 
 * @param buffer
 * @param f: data writen
 *
 * @return the next free space in the buffer
 */
char *write_f32(char *buffer, float f);

/**
 * Writes a 64 bytes float (double) in the buffer
 * 
 * @param buffer
 * @param d: data writen
 *
 * @return the next free space in the buffer
 */
char *write_f64(char *buffer, double d);

/**
 * Writes a string in the buffer prefixed by its size
 * 
 * @param buffer
 * @param str: data writen
 *
 * @return the next free space in the buffer
 */
char *write_str(char *buffer, char *str);

/**
 * Reads a 16 bytes unsigned int from the buffer
 * 
 * @param buffer
 * @param i: data read
 *
 * @return the next space to read from the buffer
 */
char *read_u16(char *buffer, uint16_t *i);

/**
 * Reads a 32 bytes unsigned int from the buffer
 * 
 * @param buffer
 * @param i: data read
 *
 * @return the next space to read from the buffer
 */
char *read_u32(char *buffer, uint32_t *i);

/**
 * Reads a 64 bytes unsigned int from the buffer
 * 
 * @param buffer
 * @param i: data read
 *
 * @return the next space to read from the buffer
 */
char *read_u64(char *buffer, uint64_t *i);

/**
 * Reads a 32 bytes float from the buffer
 * 
 * @param buffer
 * @param f: data read
 *
 * @return the next space to read from the buffer
 */
char *read_f32(char *buffer, float *f);

/**
 * Reads a 64 bytes float (double) from the buffer
 * 
 * @param buffer
 * @param d: data read
 *
 * @return the next space to read from the buffer
 */
char *read_f64(char *buffer, double *d);

/**
 * Reads a string prefixed by its size from the buffer
 * 
 * @param buffer
 * @param str: data read
 *
 * @return the next space to read from the buffer
 */
char *read_str(char *buffer, char **str);
