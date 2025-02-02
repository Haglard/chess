/**
 * ##VERSION## "chess_moves_rook.c 1.2"
 *
 */

#include "chess_moves_rook.h"
#include "chess_moves.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

void generate_black_rook_moves(const bitboard_state_t *state, dynamic_vector_t *moves) {
    uint64_t rooks = state->black_rooks;

    // Bitboard dei pezzi neri e bianchi
    uint64_t black_occ = (state->black_pawns   | state->black_knights | state->black_bishops |
                          state->black_rooks   | state->black_queens  | state->black_kings);
    uint64_t white_occ = (state->white_pawns   | state->white_knights | state->white_bishops |
                          state->white_rooks   | state->white_queens  | state->white_kings);

    while (rooks) {
        int from = __builtin_ctzll(rooks);
        rooks &= rooks - 1;

        uint64_t from_bit = (1ULL << from);

        // Est
        explore_ray(from_bit, from, +1, 0x8080808080808080ULL, black_occ, white_occ, moves);
        // Ovest
        explore_ray(from_bit, from, -1, 0x0101010101010101ULL, black_occ, white_occ, moves);
        // Nord
        explore_ray(from_bit, from, +8, 0xFF00000000000000ULL, black_occ, white_occ, moves);
        // Sud
        explore_ray(from_bit, from, -8, 0x00000000000000FFULL, black_occ, white_occ, moves);
    }
}

/**
 * @brief Genera tutte le mosse legali per le torri bianche.
 */
void generate_white_rook_moves(const bitboard_state_t *state, dynamic_vector_t *moves) {
    uint64_t rooks = state->white_rooks;

    // Bitboard di occupazione
    uint64_t white_occ = state->white_pawns   | state->white_knights   | state->white_bishops |
                         state->white_rooks   | state->white_queens    | state->white_kings;
    uint64_t black_occ = state->black_pawns   | state->black_knights   | state->black_bishops |
                         state->black_rooks   | state->black_queens    | state->black_kings;

    while (rooks) {
        int from = __builtin_ctzll(rooks);
        rooks &= rooks - 1;

        uint64_t from_bit = (1ULL << from);

        // Direzione Est: shift = +1, blocco se colonna h => mask = 0x8080808080808080
        explore_ray(from_bit, from, 1, 0x8080808080808080ULL, white_occ, black_occ, moves);
        // Direzione Ovest: shift = -1, blocco se colonna a => mask = 0x0101010101010101
        explore_ray(from_bit, from, -1, 0x0101010101010101ULL, white_occ, black_occ, moves);
        // Direzione Nord: shift = +8, blocco se rank 8 => mask = 0xFF00000000000000
        explore_ray(from_bit, from, 8, 0xFF00000000000000ULL, white_occ, black_occ, moves);
        // Direzione Sud: shift = -8, blocco se rank 1 => mask = 0x00000000000000FF
        explore_ray(from_bit, from, -8, 0x00000000000000FFULL, white_occ, black_occ, moves);
    }
}
