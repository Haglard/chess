/******************************************************************************
# ##VERSION## "chess_hash.h 1.0"
#
# Nome Progetto  : ChessEngine
# Versione       : 1.0
# Data Creazione : 17/12/2024
# Autore         : [Il tuo nome]
#
# Descrizione    : Questo file header definisce l'interfaccia per il modulo di
#                  hashing Zobrist utilizzato nel motore di scacchi. Fornisce
#                  le dichiarazioni delle funzioni necessarie per inizializzare
#                  le chiavi Zobrist, calcolare l'hash di uno stato di gioco e
#                  confrontare due stati di gioco.
#
# Dipendenze     : chess_state.h, stdint.h
#
# Funzionalità   :
#   - Inizializza le chiavi Zobrist per l'hashing.
#   - Calcola l'hash Zobrist per uno stato di gioco.
#   - Confronta due stati di gioco per verificarne l'uguaglianza.
#
# Uso:
#   - Includere questo header nei file che necessitano delle funzioni di hashing.
#   - Chiamare `chess_hash_init()` all'inizio del gioco per generare le chiavi Zobrist.
#   - Utilizzare `chess_hash_state()` per ottenere l'hash di uno stato di gioco.
#   - Utilizzare `chess_equals_state()` per confrontare due stati di gioco.
#
# Esempio:
#   #include "chess_hash.h"
#   #include "chess_state.h"
#
#   int main() {
#       chess_hash_init();
#       
#       bitboard_state_t state1 = initialize_state();
#       bitboard_state_t state2 = initialize_state();
#
#       uint64_t hash1 = chess_hash_state(&state1);
#       uint64_t hash2 = chess_hash_state(&state2);
#
#       if (chess_equals_state(&state1, &state2)) {
#           printf("Gli stati di gioco sono identici.\n");
#       } else {
#           printf("Gli stati di gioco sono diversi.\n");
#       }
#
#       return 0;
#   }
#
******************************************************************************/

#ifndef CHESS_HASH_H
#define CHESS_HASH_H

#include "chess_state.h"
#include <stdint.h>

/**
 * @brief Inizializza le chiavi Zobrist per l'hashing.
 *
 * Questa funzione deve essere chiamata una volta all'inizio del gioco
 * per generare tutte le chiavi Zobrist necessarie. Le chiavi Zobrist sono
 * utilizzate per calcolare un hash unico dello stato corrente della scacchiera,
 * facilitando il riconoscimento di posizioni ripetute e l'ottimizzazione delle
 * ricerche algoritmiche.
 *
 * @note
 *   - È responsabilità dell'utente chiamare questa funzione prima di utilizzare
 *     altre funzioni di hashing.
 *   - Le chiavi generate sono uniche e necessarie per garantire la correttezza
 *     dell'hashing Zobrist.
 */
void chess_hash_init(void);

/**
 * @brief Calcola l'hash Zobrist per uno stato di gioco.
 *
 * Questa funzione prende come unico parametro il puntatore allo stato di gioco
 * (`bitboard_state_t`) e restituisce un valore hash a 64 bit. Le chiavi Zobrist
 * sono predefinite e incorporate direttamente nel modulo, eliminando la necessità
 * di inizializzazioni manuali durante il calcolo dell'hash.
 *
 * @param state Puntatore allo stato di gioco (`bitboard_state_t`).
 * @return Hash Zobrist dello stato di gioco (uint64_t).
 *
 * @note
 *   - Assicurarsi che `chess_hash_init()` sia stata chiamata prima di utilizzare
 *     questa funzione.
 *   - L'hash restituito rappresenta in modo univoco la configurazione corrente
 *     della scacchiera, inclusi i posizionamenti dei pezzi, i diritti di arrocco,
 *     la casella en passant e il giocatore corrente.
 */
uint64_t chess_hash_state(const void *state);

/**
 * @brief Confronta due stati di gioco per l'uguaglianza.
 *
 * Questa funzione verifica se due stati di gioco sono identici, confrontando
 * tutti i campi rilevanti della struttura `bitboard_state_t`. È utile per
 * determinare se una posizione di scacchi è già stata raggiunta in una sequenza
 * di mosse, facilitando l'identificazione di patte o cicli ripetitivi.
 *
 * @param state1 Puntatore al primo stato del gioco (`bitboard_state_t`).
 * @param state2 Puntatore al secondo stato del gioco (`bitboard_state_t`).
 * @return 1 se i due stati sono uguali, 0 altrimenti.
 *
 * @note
 *   - Questa funzione confronta direttamente i campi delle strutture di stato,
 *     assicurando una verifica completa dell'uguaglianza.
 *   - Non tiene conto di eventuali altri parametri non inclusi nella struttura.
 */
int chess_equals_state(const void *state1, const void *state2);

#endif /* CHESS_HASH_H */
