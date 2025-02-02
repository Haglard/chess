/******************************************************************************
# ##VERSION## "chess_moves.h 1.0"
#
# Nome Progetto  : ChessEngine
# Versione       : 1.0
# Data Creazione : 17/12/2024
# Autore         : [Il tuo nome]
#
# Descrizione    : Questo file header definisce l'interfaccia per la gestione delle
#                  mosse nel gioco degli scacchi utilizzando Bitboards. Include la
#                  definizione della struttura `chess_move_t` e le dichiarazioni
#                  delle funzioni necessarie per generare, gestire e manipolare
#                  le mosse legali.
#
# Dipendenze     : chess_state.h, obj_dynamic_vector.h, stdint.h
#
# Funzionalità   :
#   - Definizione della struttura `chess_move_t` per rappresentare una mossa.
#   - Dichiarazione di funzioni di callback per ottenere, liberare e gestire
#     le mosse legali.
#   - Dichiarazione di funzioni specifiche per generare mosse per ciascun tipo
#     di pezzo.
#
# Uso:
#   - Includere questo header nei file che necessitano di generare o gestire
#     mosse nel gioco degli scacchi.
#   - Utilizzare le funzioni dichiarate per ottenere le mosse legali da uno
#     stato di gioco, gestire la memoria delle mosse e accedere ai dettagli
#     delle singole mosse.
#
# Esempio:
#   #include "chess_moves.h"
#   #include "chess_state.h"
#
#   int main() {
#       bitboard_state_t current_state = initialize_game_state();
#       dynamic_vector_t* legal_moves = chess_get_moves(&current_state);
#
#       for (int i = 0; i < chess_get_num_moves(legal_moves); i++) {
#           chess_move_t* move = (chess_move_t*)chess_get_move_at(legal_moves, i);
#           // Processa la mossa
#       }
#
#       chess_free_moves(legal_moves);
#       return 0;
#   }
#
******************************************************************************/

#ifndef CHESS_MOVES_H
#define CHESS_MOVES_H

#include "chess_state.h"
#include "obj_dynamic_vector.h"
#include <stdint.h>

/**
 * @brief Rappresentazione di una mossa nel gioco degli scacchi utilizzando Bitboards.
 *
 * Ogni mossa è definita da:
 * - `from`: Indice della casella di partenza (0-63, dove 0 = a1, 1 = b1, ..., 63 = h8).
 * - `to`: Indice della casella di arrivo (0-63).
 * - `promotion`: Tipo di promozione se applicabile (0 = nessuna promozione, 1 = Cavallo, 2 = Alfiere, 3 = Torre, 4 = Regina).
 * - `is_castling`: Flag che indica se la mossa è un arrocco (1 = sì, 0 = no).
 * - `is_en_passant`: Flag che indica se la mossa è un en passant (1 = sì, 0 = no).
 */
typedef struct {
    uint8_t from;            /**< Casella di partenza (0-63) */
    uint8_t to;              /**< Casella di arrivo (0-63) */
    uint8_t promotion;      /**< Tipo di promozione (0 = nessuna, 1 = N, 2 = B, 3 = R, 4 = Q) */
    uint8_t is_castling;    /**< Flag per arrocco (1 = sì, 0 = no) */
    uint8_t is_en_passant;  /**< Flag per en passant (1 = sì, 0 = no) */
} chess_move_t;

/**
 * @brief Funzione di callback per ottenere tutte le mosse legali da uno stato. Genera tutte le pseudo mosse 
 * legali nello stato corrente. Inclusa la cattura del re avversario. Assume che ci sia una fase di 
 * filtro successiva che escluda le mosse non valide in base alle regole complesse degli scacchi. Ad
 * esempio, genera la mossa che cattura il re avversario, genera le mosse en-passant in base ai flag di stato,
 * genera le mosse di arrocco lungo e corto in base ai flag di stato ma non controlla se una delle caselle
 * su cui si muove il re siano sotto scacco, non controlla se un pezzo è pinned e nel caso in cui venga mosso
 * il proprio re vada sotto scacco.
 *
 * @param[in] state Stato corrente del gioco (`bitboard_state_t`).
 * @return Un puntatore a `dynamic_vector_t` contenente tutte le mosse legali (`chess_move_t*`).
 *
 * @note La funzione deve allocare dinamicamente la struttura `dynamic_vector_t` e le mosse.
 *       È responsabilità dell'utente liberare la memoria utilizzando `chess_free_moves`.
 */
dynamic_vector_t* chess_get_moves(const void *state);

/**
 * @brief Funzione di callback per liberare il vettore di mosse.
 *
 * @param[in] moves_vec Vettore dinamico contenente le mosse (`chess_move_t*`).
 *
 * @note La funzione libera tutte le mosse all'interno del vettore e poi il vettore stesso.
 */
void chess_free_moves(dynamic_vector_t *moves_vec);

/**
 * @brief Funzione di callback per ottenere il numero di mosse nel vettore.
 *
 * @param[in] moves_vec Vettore dinamico contenente le mosse (`chess_move_t*`).
 * @return Numero di mosse presenti nel vettore.
 */
int chess_get_num_moves(const dynamic_vector_t *moves_vec);

/**
 * @brief Funzione di callback per accedere a una mossa specifica.
 *
 * @param[in] moves_vec Vettore dinamico contenente le mosse (`chess_move_t*`).
 * @param[in] index Indice della mossa da accedere (da 0 a n-1).
 * @return Puntatore alla mossa richiesta (`chess_move_t*`), oppure `NULL` se l'indice è fuori range.
 */
void* chess_get_move_at(const dynamic_vector_t *moves_vec, int index);

/**
 * @brief Funzione di callback per creare una copia profonda di una singola mossa.
 *
 * @param[in] move Mossa da copiare (`chess_move_t*`).
 * @return Puntatore a una nuova mossa copiata (`chess_move_t*`), allocata dinamicamente, oppure `NULL` in caso di errore.
 */
void* chess_copy_move(const void *move);

/**
 * @brief Funzione di callback per liberare la memoria di una singola mossa.
 *
 * @param[in] move Mossa da liberare (`chess_move_t*`).
 */
void chess_free_move(void *move);

/*
** Utilità di supporto 
**
*/

/* --------------------------------------------------------------------------
 * MASCHERE DI SUPPORTO
 * --------------------------------------------------------------------------
 *
 * Queste costanti servono per "spegnere" i bit corrispondenti
 * a certe colonne. Sono utili per evitare "wrap-around" nei bitboard
 * quando si calcolano gli spostamenti di cavalli, re, ecc.
 */

#define NOT_A_FILE  0xfefefefefefefefeULL  // 11111110 su ogni byte
#define NOT_H_FILE  0x7f7f7f7f7f7f7f7fULL  // 01111111 su ogni byte
#define NOT_AB_FILE 0xfcfcfcfcfcfcfcfcULL  // 11111100 su ogni byte
#define NOT_HG_FILE 0x3f3f3f3f3f3f3f3fULL  // 00111111 su ogni byte

/**
 * @brief Aggiunge una nuova mossa al vettore delle mosse.
 *
 * @param[in] moves       Vettore dinamico dove aggiungere la mossa.
 * @param[in] from        Casella di partenza (0-63).
 * @param[in] to          Casella di arrivo (0-63).
 * @param[in] promotion   Tipo di promozione (0 = nessuna, 1 = Cavallo, 2 = Alfiere, 3 = Torre, 4 = Regina).
 * @param[in] is_castling Flag per arrocco (1 = sì, 0 = no).
 * @param[in] is_en_passant Flag per en passant (1 = sì, 0 = no).
 */
void add_move(dynamic_vector_t *moves, uint8_t from, uint8_t to, 
              uint8_t promotion, uint8_t is_castling, uint8_t is_en_passant);

/**
 * @brief Esplora una direzione (ray) spostandosi di `shift` bit a ogni step
 *        finché non incontra un pezzo o esce dalla scacchiera.
 *
 * @param from_bit   Bitboard con un singolo bit “di partenza” già acceso
 * @param from       Indice (0-63) della casella di partenza
 * @param shift      Quanti bit spostare a ogni passo (±1, ±7, ±8, ±9, ecc.)
 * @param block_mask Maschera di confine per la direzione (es. 0x808080... se shift 7)
 * @param black_occ  Bitboard dei pezzi neri
 * @param white_occ  Bitboard dei pezzi bianchi
 * @param moves      Vettore dinamico dove aggiungere le mosse
 */
void explore_ray(uint64_t from_bit, int from, int shift,
                 uint64_t block_mask,
                 uint64_t own_occ, uint64_t opp_occ,
                 dynamic_vector_t *moves);

#endif /* CHESS_MOVES_H */
