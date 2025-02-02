/**
 * @file
 * @brief Interfaccia per una cache generica basata su tabella hash.
 *
 * ##VERSION## "obj_cache.h 1.1"
 *
 * Fornisce una struttura hash generica (\ref generic_hash_table_t) che permette
 * di memorizzare e recuperare coppie (chiave, valore). L'hash e la funzione di
 * confronto per le chiavi sono definiti tramite callback fornite dall'utente.
 *
 * La struttura è volutamente opaca: l'utente non conosce i dettagli implementativi
 * (bucket, liste di collisione, ecc.).
 */

#ifndef OBJ_CACHE_H
#define OBJ_CACHE_H

#include "obj_mem.h"
#include "obj_trace.h"
#include <stdint.h>  /* per uint64_t */
#include <stdlib.h>  /* per size_t */

#define INITIAL_CAPACITY 1024

/* --------------------------------------------------------------------------
 * Funzioni di hash e di confronto
 * -------------------------------------------------------------------------- */

/**
 * @typedef hash_func_t
 * @brief Puntatore a funzione per calcolare l'hash di una chiave.
 *
 * @param[in] key  Puntatore generico alla chiave da hashare.
 * @return uint64_t Valore di hash (in genere 64 bit).
 *
 * L'implementazione deve identificare univocamente la chiave,
 * riducendone i dati a un valore numerico.
 */
typedef uint64_t (*hash_func_t)(const void *key);

/**
 * @typedef equals_func_t
 * @brief Puntatore a funzione per confrontare due chiavi.
 *
 * @param[in] keyA  Prima chiave.
 * @param[in] keyB  Seconda chiave.
 * @return int      1 se le due chiavi sono uguali, 0 altrimenti.
 *
 * È importante che questa funzione sia coerente con la funzione di hash:
 * due chiavi ritenute uguali dovrebbero restituire lo stesso hash,
 * anche se la tabella hash può comunque gestire collisioni.
 */
typedef int (*equals_func_t)(const void *keyA, const void *keyB);

/* --------------------------------------------------------------------------
 * Struttura dati principale: generic_hash_table
 * -------------------------------------------------------------------------- */

/**
 * @struct generic_hash_table
 * @brief Struttura opaca che rappresenta la tabella hash.
 *
 * La definizione completa è nascosta al di fuori di questo header;
 * gli utenti interagiscono soltanto tramite i metodi esposti.
 */
typedef struct generic_hash_table generic_hash_table_t;

/**
 * @brief Crea una nuova tabella hash generica.
 *
 * @param[in] hash_cb  Callback per calcolare l'hash di una chiave (obbligatoria).
 * @param[in] eq_cb    Callback per confrontare due chiavi (obbligatoria).
 * @return generic_hash_table_t*  Puntatore alla tabella hash creata,
 *                                oppure NULL in caso di errore.
 *
 * L'utente è responsabile di chiamare \ref cache_destroy quando
 * non serve più la tabella.
 */
generic_hash_table_t* cache_create(hash_func_t hash_cb, equals_func_t eq_cb);

/**
 * @brief Distrugge la tabella hash, liberando tutta la memoria utilizzata.
 *
 * @param[in] cache  Puntatore alla tabella hash da distruggere.
 *
 * @note Questa funzione NON libera automaticamente le chiavi o i valori,
 *       poiché non conosce la loro natura (stack, heap, altro). È responsabilità
 *       del chiamante assicurarsi di aver gestito correttamente la vita di
 *       chiavi/valori, oppure estendere questa funzione per accettare callback
 *       di cleanup personalizzate.
 */
void cache_destroy(generic_hash_table_t *cache);

/* --------------------------------------------------------------------------
 * Operazioni base su (key, value)
 * -------------------------------------------------------------------------- */

/**
 * @brief Cerca una voce associata a \p key nella cache.
 *
 * @param[in] cache  Puntatore alla tabella hash.
 * @param[in] key    Chiave da cercare.
 * @return void*     Il valore associato alla chiave, oppure NULL se non trovato.
 *
 * Per determinare il bucket e la voce corrispondente, la funzione
 * utilizza le callback \c hash_cb e \c eq_cb fornite in fase di creazione.
 */
void* cache_lookup(generic_hash_table_t *cache, const void *key);

/**
 * @brief Inserisce o sostituisce la voce (\p key, \p value) nella cache.
 *
 * @param[in] cache  Puntatore alla tabella hash.
 * @param[in] key    Chiave da memorizzare.
 * @param[in] value  Valore associato.
 *
 * Se la chiave \p key esiste già nella tabella (confronto mediante \c eq_cb),
 * il valore associato viene aggiornato con \p value. Altrimenti, viene
 * aggiunta una nuova voce. La funzione si basa su \c hash_cb e \c eq_cb
 * per individuare il bucket corretto.
 */
void cache_store(generic_hash_table_t *cache, const void *key, const void *value);

/* --------------------------------------------------------------------------
 * Modalità di navigazione 1: funzione for-each (callback-based)
 * -------------------------------------------------------------------------- */

/**
 * @typedef cache_enum_fn
 * @brief Funzione di callback per enumerare le coppie (key, value) contenute nella cache.
 *
 * @param[in] key       Puntatore alla chiave (interna alla cache).
 * @param[in] value     Puntatore al valore associato.
 * @param[in] user_data Parametro arbitrario passato dall'utente a \ref cache_for_each.
 */
typedef void (*cache_enum_fn)(const void *key, const void *value, void *user_data);

/**
 * @brief Esegue la \p callback su ogni coppia (key, value) memorizzata nella cache.
 *
 * @param[in] cache    Puntatore alla tabella hash.
 * @param[in] fn       Funzione di callback da invocare.
 * @param[in] user_data Parametro arbitrario che verrà passato a \p fn.
 *
 * L'ordine di enumerazione non è garantito, potrebbe dipendere
 * dall'implementazione interna (numero di bucket, risoluzione collisioni, ecc.).
 */
void cache_for_each(generic_hash_table_t *cache, cache_enum_fn fn, void *user_data);

/* --------------------------------------------------------------------------
 * Modalità di navigazione 2: iteratore esplicito
 * -------------------------------------------------------------------------- */

/**
 * @struct cache_iterator
 * @brief Struttura opaca che rappresenta un iteratore sulla cache.
 *
 * Permette di scorrere, uno per uno, tutti gli elementi (key, value) della cache.
 */
typedef struct cache_iterator cache_iterator_t;

/**
 * @brief Crea un iteratore per scorrere tutti gli elementi nella cache.
 *
 * @param[in] cache La cache su cui iterare.
 * @return Puntatore a un iteratore valido, oppure NULL in caso di errore.
 *
 * @note L'iteratore rimane valido finché non si modifica la cache in modo sostanziale
 *       (ad es. inserendo o rimuovendo chiavi). In tal caso, il comportamento
 *       dell'iteratore è indefinito.
 */
cache_iterator_t* cache_iterator_create(generic_hash_table_t *cache);

/**
 * @brief Distrugge l'iteratore, liberando le risorse associate.
 *
 * @param[in] iter Puntatore all'iteratore da distruggere.
 */
void cache_iterator_destroy(cache_iterator_t *iter);

/**
 * @brief Ritorna l'elemento corrente dell'iteratore e avanza all'elemento successivo.
 *
 * @param[in]  iter   Iteratore creato con \ref cache_iterator_create.
 * @param[out] pkey   Riceverà il puntatore alla chiave (se presente).
 * @param[out] pvalue Riceverà il puntatore al valore (se presente).
 * @return int        1 se è stato restituito un elemento valido, 0 se non ce ne sono più.
 *
 * Esempio d’uso:
 * @code
 *   cache_iterator_t *it = cache_iterator_create(cache);
 *   const void *key;
 *   void *value;
 *   while (cache_iterator_next(it, &key, &value)) {
 *       // usa key/value
 *   }
 *   cache_iterator_destroy(it);
 * @endcode
 */
int cache_iterator_next(cache_iterator_t *iter, const void **pkey, void **pvalue);

#endif /* OBJ_CACHE_H */

