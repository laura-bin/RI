#pragma once
/** *************************************************************************************
 * Exercice sur l'envoi de fichiers
 * ================================
 *
 * Common constants
 *
 * RI 2020 - Laura Binacchi - Fedora 32
 ****************************************************************************************/

#define PORT            "8888"          // port number
#define BUF_SIZE        1024            // max number of bytes we can get at once
#define ID_BYTE         'a'             // identification byte

#define DIR_FILE        "./files"       // directory containing the downloadable files
#define DIR_DL          "./download"    // directory containing the downloaded files

#define EXIT_SUCCESS    0
#define EXIT_FAILURE    1