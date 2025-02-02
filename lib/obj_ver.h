/**
 * @file obj_ver.h
 * @brief Dichiarazioni per la stampa delle versioni dei moduli.
 *
 * ##VERSION## "obj_ver.h 1.0"
 *
 * Questo header fornisce una funzione per stampare le versioni di tutti i file sorgente
 * (`.c` e `.h`) utilizzati nella build del programma. Le versioni vengono generate
 * dinamicamente dal Makefile durante la fase di compilazione.
 *
 * ## Funzionalità Principali
 * - Stampa a video delle versioni di tutti i moduli coinvolti nella build.
 * - Formato della versione: `<nomefile>.<estensione> versione`.
 *
 * ## Esempio di Utilizzo
 * \code{.c}
 * #include "obj_ver.h"
 *
 * int main() {
 *     print_versions();
 *     return 0;
 * }
 * \endcode
 */

#ifndef OBJ_VER_H
#define OBJ_VER_H

/**
 * @brief Stampa le versioni di tutti i moduli coinvolti nella build.
 *
 * Questa funzione scrive a video una lista delle versioni di tutti i file sorgente
 * (`.c` e `.h`) inclusi nella build del programma, secondo il formato
 * `<nomefile>.<estensione> versione`.
 *
 * @note Le versioni sono tipicamente generate dal Makefile durante la fase di compilazione.
 */
void print_versions(void);

#endif /* OBJ_VER_H */
