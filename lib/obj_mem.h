/**
 * @file obj_mem.h
 * @brief Gestione della memoria con debug opzionale.
 *
 * ##VERSION## "obj_mem.h 1.1"
 *
 * Questo header fornisce meccanismi avanzati per la gestione della memoria,
 * inclusa una modalità di debug opzionale attivabile tramite la macro `OBJ_MEM_DEBUG_ON`.
 *
 * ### Funzionalità Principali:
 * - **Debug della Memoria**: Tracciamento delle allocazioni e deallocazioni.
 * - **Report delle Allocazioni**: Possibilità di stampare la lista delle allocazioni attive.
 * - **Dump Esadecimale della Memoria**: Visualizzazione dettagliata del contenuto della memoria.
 *
 * ### Modalità di Funzionamento:
 * - Se `OBJ_MEM_DEBUG_ON` è definita:
 *   - `malloc` viene reindirizzato a `obj_mem_malloc`.
 *   - `free` viene reindirizzato a `obj_mem_free`.
 *   - `realloc` viene reindirizzato a `obj_mem_realloc`.
 *   - `calloc` viene reindirizzato a `obj_mem_calloc`.
 *   - È possibile ottenere report dettagliati tramite `dump_allocated_memory()` e
 *     `dump_allocated_memory_hex()`.
 *
 * - Se `OBJ_MEM_DEBUG_ON` non è definita:
 *   - Le chiamate a `malloc`, `free`, `realloc` e `calloc` rimangono quelle standard della libc.
 *   - Le funzioni di dump sono disabilitate.
 *
 * ### Macro di Configurazione:
 * - `OBJ_MEM_DEBUG_ON`: Abilita il debug della memoria.
 * - `OBJ_MEM_INTERNAL`: Deve essere definita in `obj_mem.c` per evitare il
 *   loop ricorsivo delle ridefinizioni di `malloc`, `free`, `realloc` e `calloc`.
 *
 * @note È consigliato abilitare `OBJ_MEM_DEBUG_ON` solo in fase di sviluppo,
 *       poiché l'overhead potrebbe influire sulle prestazioni.
 */

#ifndef OBJ_MEM_H
#define OBJ_MEM_H

#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>

/* --------------------------------------------------------------------------
 * Sezione Debug della Memoria
 * --------------------------------------------------------------------------
 */

/**
 * @def OBJ_MEM_DEBUG_ON
 * @brief Macro opzionale per abilitare il debug della memoria.
 *
 * Se definita, reindirizza le chiamate a `malloc`, `free`, `realloc` e `calloc`
 * verso `obj_mem_malloc`, `obj_mem_free`, `obj_mem_realloc` e `obj_mem_calloc`.
 * Le funzioni di dump diventano disponibili.
 *
 * Se non è definita, le chiamate rimangono quelle standard e le funzioni
 * di dump sono disabilitate.
 */
// #define OBJ_MEM_DEBUG_ON 1  // Decommentare per abilitare il debug

#if defined(OBJ_MEM_DEBUG_ON) /* Memory debug attivo */

/**
 * @brief Alloca memoria con tracciamento avanzato (sostituisce `malloc`).
 *
 * @param[in] size  Dimensione della memoria da allocare in byte.
 * @param[in] file  Nome del file sorgente da cui viene chiamata l'allocazione.
 * @param[in] line  Numero di linea del file sorgente da cui viene chiamata l'allocazione.
 * @return Puntatore alla memoria allocata, oppure `NULL` in caso di fallimento.
 *
 * @note I parametri \p file e \p line consentono di tracciare la provenienza
 *       dell'allocazione nel codice.
 */
void* obj_mem_malloc(size_t size, const char *file, int line);

/**
 * @brief Dealloca memoria con tracciamento avanzato (sostituisce `free`).
 *
 * @param[in] ptr  Puntatore alla memoria da deallocare.
 *
 * @note L'implementazione può mantenere un registro interno delle allocazioni
 *       per rilevare eventuali leak o double-free.
 */
void obj_mem_free(void* ptr);

/**
 * @brief Ridimensiona un blocco di memoria con tracciamento avanzato (sostituisce `realloc`).
 *
 * @param[in] ptr       Puntatore al blocco precedentemente allocato (può essere `NULL`).
 * @param[in] new_size  Nuova dimensione richiesta in byte.
 * @param[in] file      Nome del file sorgente che richiama la `realloc` (usare `__FILE__`).
 * @param[in] line      Numero di linea del file sorgente (usare `__LINE__`).
 * @return Puntatore al nuovo blocco di memoria, o `NULL` in caso di errore.
 *
 * Se \p ptr è `NULL`, la funzione si comporta come un'allocazione ex-novo (equivalente a
 * \c obj_mem_malloc(\p new_size, \p file, \p line)).
 * Se \p new_size è 0, la funzione si comporta come \c obj_mem_free(\p ptr), restituendo `NULL`.
 */
void* obj_mem_realloc(void* ptr, size_t new_size, const char *file, int line);

/**
 * @brief Alloca memoria per un array di \p nmemb elementi, ciascuno di dimensione \p size,
 *        con tracciamento avanzato (sostituisce `calloc`).
 *
 * @param[in] nmemb Numero di elementi da allocare.
 * @param[in] size  Dimensione di ciascun elemento (in byte).
 * @param[in] file  Nome del file sorgente che richiama la `calloc` (usare `__FILE__`).
 * @param[in] line  Numero di linea del file sorgente (usare `__LINE__`).
 * @return Puntatore alla memoria allocata (azzerata), oppure `NULL` in caso di fallimento.
 *
 * Questa funzione si comporta come `calloc(nmemb, size)`, ma traccia le allocazioni
 * per il debug della memoria. L'area restituita è azzerata (tutti i byte a 0).
 */
void* obj_mem_calloc(size_t nmemb, size_t size, const char *file, int line);

/**
 * @brief Stampa informazioni sulle allocazioni correnti.
 *
 * Elenca tutti i blocchi di memoria attualmente allocati, mostrando dettagli
 * su dimensione, file sorgente e numero di linea di allocazione.
 *
 * @note Disponibile solo quando `OBJ_MEM_DEBUG_ON` è definita.
 *       Ignorata (ridefinita a vuoto) altrimenti.
 */
void dump_allocated_memory(void);

/**
 * @brief Stampa informazioni dettagliate sulle allocazioni correnti con dump esadecimale.
 *
 * Elenca i blocchi di memoria allocati e mostra il contenuto della memoria in formato esadecimale.
 *
 * @note Disponibile solo quando `OBJ_MEM_DEBUG_ON` è definita.
 *       Ignorata (ridefinita a vuoto) altrimenti.
 */
void dump_allocated_memory_hex(void);

#endif /* OBJ_MEM_DEBUG_ON */

/* --------------------------------------------------------------------------
 * Ridefinizione di malloc, free, realloc e calloc in modalità Debug
 * --------------------------------------------------------------------------
 *
 * Se stiamo compilando in modalità debug (OBJ_MEM_DEBUG_ON) ma non siamo
 * all'interno di obj_mem.c (che implementa le funzioni), allora ridefiniamo
 * malloc, free, realloc e calloc per utilizzare obj_mem_malloc, obj_mem_free,
 * obj_mem_realloc e obj_mem_calloc.
 */

#if !defined(OBJ_MEM_C) && defined(OBJ_MEM_DEBUG_ON)

/**
 * @brief Ridefinisce \c malloc per utilizzare \c obj_mem_malloc in modalità debug.
 */
#define malloc(SZ) obj_mem_malloc((SZ), __FILE__, __LINE__)

/**
 * @brief Ridefinisce \c free per utilizzare \c obj_mem_free in modalità debug.
 */
#define free(PTR) obj_mem_free((PTR))

/**
 * @brief Ridefinisce \c realloc per utilizzare \c obj_mem_realloc in modalità debug.
 */
#define realloc(PTR, SZ) obj_mem_realloc((PTR), (SZ), __FILE__, __LINE__)

/**
 * @brief Ridefinisce \c calloc per utilizzare \c obj_mem_calloc in modalità debug.
 */
#define calloc(NM, SZ) obj_mem_calloc((NM), (SZ), __FILE__, __LINE__)

#endif /* !OBJ_MEM_C && OBJ_MEM_DEBUG_ON */

/* --------------------------------------------------------------------------
 * Disabilitazione delle funzioni di dump in modalità non-debug
 * --------------------------------------------------------------------------
 *
 * Se non è definito OBJ_MEM_DEBUG_ON e non stiamo compilando in obj_mem.c,
 * ridefiniamo le funzioni di dump come macro vuote per evitare chiamate inutili.
 */

#if !defined(OBJ_MEM_DEBUG_ON) && !defined(OBJ_MEM_C)

/**
 * @brief Disabilita \c dump_allocated_memory quando \c OBJ_MEM_DEBUG_ON non è definita.
 */
#define dump_allocated_memory() do {} while (0)

/**
 * @brief Disabilita \c dump_allocated_memory_hex quando \c OBJ_MEM_DEBUG_ON non è definita.
 */
#define dump_allocated_memory_hex() do {} while (0)

#endif /* !OBJ_MEM_DEBUG_ON && !OBJ_MEM_C */

#endif /* OBJ_MEM_H */
