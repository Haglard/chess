/**
 * @file
 * @brief Libreria per l'algoritmo MiniMax con alpha-beta pruning e cache opzionale.
 *
 * ##VERSION## "minimax.h 1.0"
 *
 * Questo file definisce il descrittore di gioco (\ref game_descriptor_t) e le funzioni
 * di MiniMax (\ref minimax_ab e \ref get_best_move). In particolare, supporta:
 * - Funzioni di callback per gestire stati e mosse.
 * - Funzioni di hash e confronto per utilizzare una cache (transposition table).
 * - Un vettore dinamico per memorizzare le mosse (\c dynamic_vector_t) definito
 *   in \c obj_dynamic_vector.h.
 *
 * @author
 *   *Tuo Nome (opzionale)*
 *
 * @date
 *   *Data (opzionale)*
 */

#ifndef MINIMAX_H
#define MINIMAX_H

#include "obj_mem.h"
#include "obj_trace.h"

#include <stddef.h>   // per size_t, NULL
#include <limits.h>   // per INT_MAX, INT_MIN
#include <stdint.h>   // per uint64_t

/**
 * @def MAX_DEPTH
 * Profondità massima di default per la ricerca MiniMax.
 */
#ifndef MAX_DEPTH
#define MAX_DEPTH 10
#endif

/**
 * @brief Header per la cache generica.
 *
 * Si presuppone che "obj_cache.h" definisca:
 *  - \c typedef struct generic_hash_table generic_hash_table_t;
 *  - Funzioni: \c cache_create(hash_func_t, equals_func_t), \c cache_destroy(...),
 *              \c cache_lookup(...), \c cache_store(...), ecc.
 *  - Tipi di callback: \c hash_func_t, \c equals_func_t
 */
#include "obj_cache.h"

/**
 * @brief Header per il vettore dinamico delle mosse.
 *
 * Si presuppone che "obj_dynamic_vector.h" definisca:
 *  - \c typedef struct dynamic_vector_s dynamic_vector_t;
 *  - \c dv_create(), \c dv_free(), \c dv_size(), \c dv_get(), ecc.
 */
#include "obj_dynamic_vector.h"

/* --------------------------------------------------------------------------
 * CALLBACK per lo stato e le mosse di gioco
 * --------------------------------------------------------------------------
 */

/**
 * @typedef copy_state_fn
 * @brief Funzione di callback per creare una copia profonda di uno stato di gioco.
 * @param[in] state Stato da copiare.
 * @return Un nuovo puntatore a stato, allocato dinamicamente.
 */
typedef void* (*copy_state_fn)(const void *state);

/**
 * @typedef free_state_fn
 * @brief Funzione di callback per liberare la memoria di uno stato.
 * @param[in] state Stato da liberare.
 */
typedef void  (*free_state_fn)(void *state);

/**
 * @typedef get_moves_fn
 * @brief Funzione di callback per ottenere tutte le mosse possibili da uno stato.
 * @param[in] state Stato corrente da cui generare le mosse.
 * @return Un \c dynamic_vector_t* contenente le mosse (puntatori generici).
 */
typedef dynamic_vector_t* (*get_moves_fn)(const void *state);

/**
 * @typedef free_moves_fn
 * @brief Funzione di callback per liberare il vettore di mosse.
 * @param[in] moves_vec Vettore dinamico da deallocare.
 */
typedef void              (*free_moves_fn)(dynamic_vector_t *moves_vec);

/**
 * @typedef get_num_moves_fn
 * @brief Funzione di callback per ottenere il numero di mosse nel \c dynamic_vector_t.
 * @param[in] moves_vec Vettore dinamico delle mosse.
 * @return Numero di mosse.
 */
typedef int               (*get_num_moves_fn)(const dynamic_vector_t *moves_vec);

/**
 * @typedef get_move_at_fn
 * @brief Funzione di callback per accedere alla i-esima mossa in un \c dynamic_vector_t.
 * @param[in] moves_vec Vettore dinamico delle mosse.
 * @param[in] index Indice della mossa (da 0 a n-1).
 * @return Puntatore generico alla mossa.
 */
typedef void*             (*get_move_at_fn)(const dynamic_vector_t *moves_vec, int index);

/**
 * @typedef copy_move_fn
 * @brief Funzione di callback per creare una copia profonda di una singola mossa.
 * @param[in] move Mossa da copiare.
 * @return Una nuova mossa (puntatore generico), allocata dinamicamente.
 */
typedef void* (*copy_move_fn)(const void *move);

/**
 * @typedef free_move_fn
 * @brief Funzione di callback per liberare la memoria di una singola mossa.
 * @param[in] move Mossa da deallocare.
 */
typedef void  (*free_move_fn)(void *move);

/**
 * @typedef apply_move_fn
 * @brief Funzione di callback per applicare una mossa a uno stato, ottenendo il nuovo stato risultante.
 * @param[in] state Stato iniziale.
 * @param[in] move Mossa da applicare.
 * @return Nuovo stato risultante (allocato dinamicamente).
 */
typedef void*  (*apply_move_fn)(const void *state, const void *move);

/**
 * @typedef is_terminal_fn
 * @brief Funzione di callback per verificare se lo stato è terminale (nessuna mossa o vittoria/sconfitta).
 * @param[in] state Stato da verificare.
 * @return 1 se lo stato è terminale, 0 altrimenti.
 */
typedef int    (*is_terminal_fn)(const void *state);

/**
 * @typedef evaluate_fn
 * @brief Funzione di callback per calcolare il punteggio/euristica di uno stato.
 * @param[in] state Stato da valutare.
 * @return Valore numerico indicante la valutazione (es. +∞ se vince, −∞ se perde).
 */
typedef int    (*evaluate_fn)(const void *state);

/**
 * @typedef player_to_move_fn
 * @brief Funzione di callback per determinare quale giocatore deve muovere.
 * @param[in] state Stato corrente.
 * @return Identificatore del giocatore (es. 1 = massimizzatore, -1 = minimizzatore).
 */
typedef int    (*player_to_move_fn)(const void *state);

/* --------------------------------------------------------------------------
 * CALLBACK per l'hashing e il confronto di stati
 * --------------------------------------------------------------------------
 */

/**
 * @typedef hash_state_fn
 * @brief Calcola un valore di hash (64 bit) a partire da uno stato di gioco.
 * @param[in] state Stato per il quale calcolare l'hash.
 * @return Valore hash (es. 64 bit) che rappresenta lo stato.
 */
typedef uint64_t (*hash_state_fn)(const void *state);

/**
 * @typedef equals_state_fn
 * @brief Confronta due stati di gioco per verificarne l'uguaglianza.
 * @param[in] stateA Primo stato.
 * @param[in] stateB Secondo stato.
 * @return 1 se gli stati sono uguali, 0 altrimenti.
 */
typedef int      (*equals_state_fn)(const void *stateA, const void *stateB);

/**
 * @struct game_descriptor
 * @brief Descrittore di gioco contenente tutte le funzioni di callback necessarie.
 *
 * Questa struttura permette alla libreria MiniMax di interagire con lo stato di gioco,
 * le mosse e l'eventuale cache (tramite funzioni di hash e confronto).
 */
typedef struct game_descriptor {
    /* Gestione dello stato */
    copy_state_fn     copy_state;    /**< Callback per copiare uno stato */
    free_state_fn     free_state;    /**< Callback per liberare uno stato */

    /* Gestione delle mosse */
    get_moves_fn      get_moves;     /**< Callback per ottenere le mosse da uno stato */
    free_moves_fn     free_moves;    /**< Callback per liberare il vettore di mosse */
    get_num_moves_fn  get_num_moves; /**< Callback per conoscere il numero di mosse disponibili */
    get_move_at_fn    get_move_at;   /**< Callback per accedere alla i-esima mossa */

    copy_move_fn      copy_move;     /**< Callback per copiare una singola mossa */
    free_move_fn      free_move;     /**< Callback per liberare una singola mossa */

    /* Applicazione e valutazione */
    apply_move_fn     apply_move;    /**< Callback per applicare una mossa e ottenere il nuovo stato */
    is_terminal_fn    is_terminal;   /**< Callback per verificare se lo stato è terminale */
    evaluate_fn       evaluate;      /**< Callback per calcolare il punteggio/euristica di uno stato */
    player_to_move_fn player_to_move;/**< Callback per determinare quale giocatore deve muovere */

    /* Hashing e confronto dello stato (per la cache) */
    hash_state_fn     hash_state;    /**< Callback per calcolare l'hash dello stato */
    equals_state_fn   equals_state;  /**< Callback per confrontare due stati */
} game_descriptor_t;

/* --------------------------------------------------------------------------
 * FIRME DELLE FUNZIONI MINIMAX
 * --------------------------------------------------------------------------
 */

/**
 * @brief Esegue l'algoritmo MiniMax (con alpha-beta pruning) sullo stato specificato.
 *
 * @param[in]  gd           Puntatore al descrittore di gioco.
 * @param[in]  state        Stato di gioco corrente.
 * @param[in]  depth        Profondità massima di ricerca.
 * @param[in]  alpha        Valore alpha iniziale (di solito \c INT_MIN).
 * @param[in]  beta         Valore beta iniziale (di solito \c INT_MAX).
 * @param[in]  cache_handle Puntatore alla struttura della cache (o \c NULL se non si usa la cache).
 * @return Valore di valutazione (euristico o finale) dello stato analizzato.
 *
 * Se \c cache_handle non è \c NULL, la funzione esegue \c cache_lookup e \c cache_store
 * per evitare di ricalcolare stati già analizzati.
 */
int minimax_ab(
    const game_descriptor_t *gd,
    const void *state,
    int depth,
    int alpha,
    int beta,
    void *cache_handle
);

/**
 * @brief Determina la mossa migliore per lo stato corrente, in base all'algoritmo MiniMax.
 *
 * @param[in]  gd           Puntatore al descrittore di gioco.
 * @param[in]  state        Stato di gioco corrente.
 * @param[in]  depth        Profondità massima di ricerca.
 * @param[in]  cache_handle Puntatore alla struttura della cache (o \c NULL se non si usa la cache).
 * @return La mossa selezionata (allocata dinamicamente con \c copy_move).
 *
 * @note L'oggetto restituito deve essere liberato dall'utente con \c free_move
 *       quando non serve più.
 *
 * @note Se lo stato è terminale o non ci sono mosse, la funzione restituisce \c NULL.
 */
void* get_best_move(
    const game_descriptor_t *gd,
    const void *state,
    int depth,
    void *cache_handle
);

#endif /* MINIMAX_H */
