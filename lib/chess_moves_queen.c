/**
 * ##VERSION## "chess_moves_queen.c 1.2"
 *
 */

#include "chess_moves_queen.h"
#include "chess_moves.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>


/**
 * @brief Genera tutte le mosse legali per le regine nere.
 *
 * La regina è un pezzo "sliding" che combina i movimenti
 * dell'alfiere (diagonali) e della torre (linee ortogonali).
 * A differenza del codice "semplificato", qui calcoliamo
 * davvero le mosse delle regine, senza usare bishop/rook.
 */
void generate_black_queen_moves(const bitboard_state_t *state, dynamic_vector_t *moves) {
    // Bitboard di tutte le regine nere
    uint64_t queens = state->black_queens;

    // Bitboard di occupazione
    uint64_t black_occ = state->black_pawns | state->black_knights | state->black_bishops |
                         state->black_rooks | state->black_queens | state->black_kings;
    uint64_t white_occ = state->white_pawns | state->white_knights | state->white_bishops |
                         state->white_rooks | state->white_queens | state->white_kings;

    while (queens) {
        int from = __builtin_ctzll(queens);
        queens &= queens - 1;

        uint64_t from_bit = 1ULL << from;

        // Diagonali (alfiere-like)
        explore_ray(from_bit, from, +7, 0x8080808080808080ULL, black_occ, white_occ, moves); // NE
        explore_ray(from_bit, from, +9, 0x0101010101010101ULL, black_occ, white_occ, moves); // NO
        explore_ray(from_bit, from, -7, 0x0101010101010101ULL, black_occ, white_occ, moves); // SO
        explore_ray(from_bit, from, -9, 0x8080808080808080ULL, black_occ, white_occ, moves); // SE

        // Linee (torre-like)
        explore_ray(from_bit, from, +1, 0x8080808080808080ULL, black_occ, white_occ, moves); // Est
        explore_ray(from_bit, from, -1, 0x0101010101010101ULL, black_occ, white_occ, moves); // Ovest
        explore_ray(from_bit, from, +8, 0xFF00000000000000ULL, black_occ, white_occ, moves); // Nord
        explore_ray(from_bit, from, -8, 0x00000000000000FFULL, black_occ, white_occ, moves); // Sud
    }
}


/**
 * @brief Genera tutte le mosse legali per le regine bianche.
 *
 * La regina è un pezzo "sliding" che combina i movimenti
 * dell'alfiere (diagonali) e della torre (linee orizzontali/verticali).
 * Qui, invece di invocare le funzioni di alfiere e torre, generiamo
 * direttamente le mosse per i bitboard delle regine.
 */
void generate_white_queen_moves(const bitboard_state_t *state, dynamic_vector_t *moves) {
    // Bitboard di tutte le regine bianche
    uint64_t queens = state->white_queens;

    // Prepara i bitboard di occupazione (per sapere quali caselle sono bloccate)
    uint64_t white_occ = state->white_pawns   | state->white_knights   | state->white_bishops |
                         state->white_rooks   | state->white_queens    | state->white_kings;
    uint64_t black_occ = state->black_pawns   | state->black_knights   | state->black_bishops |
                         state->black_rooks   | state->black_queens    | state->black_kings;

    while (queens) {
        // Estrai l'indice (0-63) della regina più a destra (LSB)
        int from = __builtin_ctzll(queens);
        // Rimuovi questa regina dal bitboard (così troveremo la prossima, se c'è)
        queens &= queens - 1;

        // Costruiamo il bit singolo della casella
        uint64_t from_bit = 1ULL << from;

        // Ora, come un alfiere: esploriamo 4 direzioni diagonali
        // (notare i relativi shift e maschere di confine)
        explore_ray(from_bit, from, +7, 0x8080808080808080ULL, white_occ, black_occ, moves); // Nord-Est
        explore_ray(from_bit, from, +9, 0x0101010101010101ULL, white_occ, black_occ, moves); // Nord-Ovest
        explore_ray(from_bit, from, -7, 0x0101010101010101ULL, white_occ, black_occ, moves); // Sud-Ovest
        explore_ray(from_bit, from, -9, 0x8080808080808080ULL, white_occ, black_occ, moves); // Sud-Est

        // Ora, come una torre: esploriamo 4 direzioni ortogonali
        explore_ray(from_bit, from, +1, 0x8080808080808080ULL, white_occ, black_occ, moves); // Est
        explore_ray(from_bit, from, -1, 0x0101010101010101ULL, white_occ, black_occ, moves); // Ovest
        explore_ray(from_bit, from, +8, 0xFF00000000000000ULL, white_occ, black_occ, moves); // Nord
        explore_ray(from_bit, from, -8, 0x00000000000000FFULL, white_occ, black_occ, moves); // Sud
    }
}
