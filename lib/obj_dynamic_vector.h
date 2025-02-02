/**
 * @file
 * @brief Interfaccia di un vettore dinamico generico in C.
 *
 * ##VERSION## "obj_dynamic_vector.h 1.0"
 *
 * Questo file dichiara le funzioni per creare e gestire un vettore dinamico di
 * puntatori `void*`. Il vettore può crescere automaticamente quando vengono
 * inseriti nuovi elementi (tramite \ref dv_push_back).
 * 
 * L'utente è responsabile di liberare eventualmente gli oggetti puntati dalle
 * celle del vettore, se necessario, in quanto questa libreria non conosce la
 * loro natura (allocazione dinamica, stack, ecc.).
 */

#ifndef OBJ_DYNAMIC_VECTOR_H
#define OBJ_DYNAMIC_VECTOR_H

#include "obj_mem.h"
#include "obj_trace.h"
#include <stddef.h> /* per size_t */

/**
 * @struct dynamic_vector_s
 * @brief Struttura opaca del vettore dinamico.
 *
 * I dettagli interni (ad esempio, campi relativi a capacità e dimensione) non
 * sono visibili all'utente, che può interagire solo tramite le funzioni
 * dichiarate in questo header.
 */
typedef struct dynamic_vector_s dynamic_vector_t;

/**
 * @brief Crea un nuovo vettore dinamico vuoto.
 *
 * @return dynamic_vector_t*  Puntatore al vettore creato, oppure NULL in caso di errore.
 *
 * @note L'utente deve in seguito chiamare \ref dv_free per liberare la struttura,
 *       evitando fughe di memoria.
 */
dynamic_vector_t* dv_create(void);

/**
 * @brief Libera il vettore dinamico, deallocando la struttura interna.
 *
 * @param[in] dv Puntatore al vettore da liberare (se NULL, la funzione non fa nulla).
 *
 * @note Questa funzione NON libera automaticamente gli oggetti puntati dagli
 *       elementi del vettore (poiché non ne conosce la natura). Se necessario,
 *       l'utente dovrà occuparsene prima di chiamare \ref dv_free.
 */
void dv_free(dynamic_vector_t *dv);

/**
 * @brief Restituisce il numero di elementi attualmente contenuti nel vettore.
 *
 * @param[in] dv Puntatore al vettore.
 * @return size_t Numero di elementi contenuti nel vettore, oppure 0 se \p dv è NULL.
 */
size_t dv_size(const dynamic_vector_t *dv);

/**
 * @brief Restituisce l'elemento in posizione \p index (0-based).
 *
 * @param[in] dv    Puntatore al vettore.
 * @param[in] index Posizione dell'elemento (deve essere < \ref dv_size(dv)).
 * @return void* Puntatore generico all'elemento, oppure NULL se \p dv è NULL o \p index è fuori range.
 *
 * @note La funzione non effettua alcuna copia dell'oggetto, si limita a restituire
 *       il puntatore memorizzato internamente.
 */
void* dv_get(const dynamic_vector_t *dv, size_t index);

/**
 * @brief Imposta l'elemento in posizione \p index a un nuovo valore (\c void*).
 *
 * @param[in] dv     Puntatore al vettore.
 * @param[in] index  Posizione dell'elemento (deve essere < \ref dv_size(dv)).
 * @param[in] value  Puntatore al nuovo valore da impostare.
 *
 * @note Non viene effettuata alcuna copia dell'oggetto puntato: la funzione
 *       si limita ad assegnare il puntatore in quella posizione.
 */
void dv_set(dynamic_vector_t *dv, size_t index, void *value);

/**
 * @brief Aggiunge (in coda) un nuovo elemento (\c void*) al vettore.
 *
 * Se necessario, il vettore viene ridimensionato automaticamente.
 *
 * @param[in] dv    Puntatore al vettore.
 * @param[in] value Puntatore al valore da inserire.
 * @return int      0 se l'inserimento ha avuto successo, -1 in caso di errore
 *                  (ad es. allocazione fallita).
 *
 * @note Il tempo medio di inserimento è O(1) (amortizzato),
 *       poiché eventuali riallocazioni avvengono in modo esponenziale.
 */
int dv_push_back(dynamic_vector_t *dv, void *value);

#endif /* OBJ_DYNAMIC_VECTOR_H */
