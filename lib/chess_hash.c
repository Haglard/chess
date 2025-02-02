/**
 * ##VERSION## "chess_hash.c 1.0"
*/

#include "chess_hash.h"
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>

/**
 * @brief Struttura che contiene le chiavi Zobrist predefinite.
 *
 * Le chiavi Zobrist sono generate dinamicamente e incorporate nel codice.
 * È fondamentale che ogni combinazione pezzo-casella abbia una chiave unica.
 */
typedef struct {
    uint64_t piece_keys[12][64];      /**< Chiavi per ogni tipo di pezzo e casella */
    uint64_t castling_keys[16];       /**< Chiavi per ogni combinazione di diritti di arrocco */
    uint64_t en_passant_keys[64];     /**< Chiavi per ogni casella en passant possibile */
    uint64_t side_to_move_key;        /**< Chiave per il giocatore corrente */
} zobrist_keys_t;

/**
 * @brief Chiavi Zobrist generate dinamicamente.
 *
 * Queste chiavi vengono inizializzate tramite la funzione `chess_hash_init()`.
 */
static zobrist_keys_t zobrist_keys;

/**
 * @brief Stato di inizializzazione delle chiavi Zobrist.
 */
static int zobrist_initialized = 0;

/**
 * @brief Stato del PRNG per la generazione delle chiavi Zobrist.
 *
 * Utilizziamo un semplice generatore di numeri pseudo-casuali Xorshift64.
 */
static uint64_t prng_state = 88172645463325252ULL;

/**
 * @brief Generatore di numeri pseudo-casuali Xorshift64.
 *
 * @return Numero pseudo-casuale a 64 bit.
 */
static uint64_t next_rand64() {
    prng_state ^= prng_state << 13;
    prng_state ^= prng_state >> 7;
    prng_state ^= prng_state << 17;
    return prng_state;
}

/**
 * @brief Inizializza le chiavi Zobrist per l'hashing.
 *
 * Questa funzione deve essere chiamata una volta all'inizio del gioco
 * per generare tutte le chiavi Zobrist necessarie.
 */
void chess_hash_init(void) {
    if (zobrist_initialized) {
        // Evita re-inizializzazioni
        return;
    }

    // Seed del PRNG basato sul tempo attuale
    prng_state ^= (uint64_t)time(NULL);

    // Generazione delle chiavi per i pezzi
    for (int piece = 0; piece < 12; piece++) {
        for (int square = 0; square < 64; square++) {
            zobrist_keys.piece_keys[piece][square] = next_rand64();
        }
    }

    // Generazione delle chiavi per i diritti di arrocco
    for (int i = 0; i < 16; i++) {
        zobrist_keys.castling_keys[i] = next_rand64();
    }

    // Generazione delle chiavi per le caselle en passant
    for (int i = 0; i < 64; i++) {
        zobrist_keys.en_passant_keys[i] = next_rand64();
    }

    // Generazione della chiave per il giocatore corrente
    zobrist_keys.side_to_move_key = next_rand64();

    zobrist_initialized = 1;
}

/**
 * @brief Calcola l'hash Zobrist per uno stato di gioco.
 *
 * Questa funzione utilizza le chiavi Zobrist generate dinamicamente
 * per calcolare l'hash dello stato di gioco.
 *
 * @param state Puntatore allo stato di gioco (`bitboard_state_t`).
 * @return Hash Zobrist dello stato di gioco (uint64_t).
 */
uint64_t chess_hash_state(const void *state_void) {
    if (!zobrist_initialized) {
        fprintf(stderr, "Errore: chess_hash_init() non è stata chiamata.\n");
        return 0;
    }

    if (!state_void) return 0;
    
    const bitboard_state_t *state = (const bitboard_state_t*)state_void;
    uint64_t hash = 0;

    // Calcola l'hash per i pezzi
    uint64_t pieces[12] = {
        state->white_pawns, state->white_knights, state->white_bishops,
        state->white_rooks, state->white_queens, state->white_kings,
        state->black_pawns, state->black_knights, state->black_bishops,
        state->black_rooks, state->black_queens, state->black_kings
    };

    for (int piece = 0; piece < 12; piece++) {
        uint64_t bb = pieces[piece];
        while (bb) {
            int sq = __builtin_ctzll(bb); // Trova la posizione della LSB (bit meno significativo impostato)
            hash ^= zobrist_keys.piece_keys[piece][sq];
            bb &= bb - 1; // Rimuove la LSB
        }
    }

    // Calcola l'hash per i diritti di arrocco
    hash ^= zobrist_keys.castling_keys[state->castling_rights & 0xF]; // Limitato a 4 bit

    // Calcola l'hash per la casella en passant, se disponibile
    if (state->en_passant < 64) {
        hash ^= zobrist_keys.en_passant_keys[state->en_passant];
    }

    // Calcola l'hash per il giocatore corrente
    if (state->current_player == -1) { // Supponiamo che 1 = Bianco, -1 = Nero
        hash ^= zobrist_keys.side_to_move_key;
    }

    return hash;
}

/**
 * @brief Confronta due stati di gioco per l'uguaglianza.
 *
 * Questa funzione verifica se due stati di gioco sono identici.
 *
 * @param stateA Puntatore al primo stato del gioco (`bitboard_state_t`).
 * @param stateB Puntatore al secondo stato del gioco (`bitboard_state_t`).
 * @return 1 se i due stati sono uguali, 0 altrimenti.
 */
int chess_equals_state(const void *stateA_void, const void *stateB_void) {
    if (!stateA_void || !stateB_void) return 0;
    const bitboard_state_t *stateA = (const bitboard_state_t*)stateA_void;
    const bitboard_state_t *stateB = (const bitboard_state_t*)stateB_void;

    return (
        stateA->white_pawns == stateB->white_pawns &&
        stateA->white_knights == stateB->white_knights &&
        stateA->white_bishops == stateB->white_bishops &&
        stateA->white_rooks == stateB->white_rooks &&
        stateA->white_queens == stateB->white_queens &&
        stateA->white_kings == stateB->white_kings &&
        stateA->black_pawns == stateB->black_pawns &&
        stateA->black_knights == stateB->black_knights &&
        stateA->black_bishops == stateB->black_bishops &&
        stateA->black_rooks == stateB->black_rooks &&
        stateA->black_queens == stateB->black_queens &&
        stateA->black_kings == stateB->black_kings &&
        stateA->castling_rights == stateB->castling_rights &&
        stateA->en_passant == stateB->en_passant &&
        stateA->halfmove_clock == stateB->halfmove_clock &&
        stateA->fullmove_number == stateB->fullmove_number &&
        stateA->current_player == stateB->current_player
    );
}
