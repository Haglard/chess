/**
 * ##VERSION## "chess_moves_black.c 1.2"
 *
 */

#include "chess_moves_bishop.h"
#include "chess_moves.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/**
 * @brief Genera tutte le mosse legali per gli alfieri neri.
 */
void generate_black_bishop_moves(const bitboard_state_t *state, dynamic_vector_t *moves) {
    uint64_t bishops = state->black_bishops;

    // Bitboard di occupazione
    uint64_t black_occ = state->black_pawns | state->black_knights | state->black_bishops |
                         state->black_rooks | state->black_queens | state->black_kings;
    uint64_t white_occ = state->white_pawns | state->white_knights | state->white_bishops |
                         state->white_rooks | state->white_queens | state->white_kings;

    while (bishops) {
        int from = __builtin_ctzll(bishops);
        bishops &= bishops - 1;

        uint64_t from_bit = (1ULL << from);

        // Diagonale Nord-Est: shift = +7, blocco se colonna h => 0x8080808080808080
        explore_ray(from_bit, from, +7, 0x8080808080808080ULL, black_occ, white_occ, moves);
        // Diagonale Nord-Ovest: shift = +9, blocco se colonna a => 0x0101010101010101
        explore_ray(from_bit, from, +9, 0x0101010101010101ULL, black_occ, white_occ, moves);
        // Diagonale Sud-Est: shift = -9, blocco se colonna h
        explore_ray(from_bit, from, -9, 0x8080808080808080ULL, black_occ, white_occ, moves);
        // Diagonale Sud-Ovest: shift = -7, blocco se colonna a
        explore_ray(from_bit, from, -7, 0x0101010101010101ULL, black_occ, white_occ, moves);
    }
}

/**
 * @brief Genera tutte le mosse legali per gli alfieri bianchi.
 */
void generate_white_bishop_moves(const bitboard_state_t *state, dynamic_vector_t *moves) {
    uint64_t bishops = state->white_bishops;

    // Creiamo i bitboard di occupazione
    uint64_t white_occ = state->white_pawns   | state->white_knights   | state->white_bishops |
                         state->white_rooks   | state->white_queens    | state->white_kings;
    uint64_t black_occ = state->black_pawns   | state->black_knights   | state->black_bishops |
                         state->black_rooks   | state->black_queens    | state->black_kings;

    while (bishops) {
        int from = __builtin_ctzll(bishops);
        bishops &= bishops - 1;

        uint64_t from_bit = (1ULL << from);

        // Diagonale Nord-Est: shift = +7, blocco se colonna h => mask = 0x8080808080808080
        explore_ray(from_bit, from, 7, 0x8080808080808080ULL, white_occ, black_occ, moves);
        // Diagonale Nord-Ovest: shift = +9, blocco se colonna a => mask = 0x0101010101010101
        explore_ray(from_bit, from, 9, 0x0101010101010101ULL, white_occ, black_occ, moves);
        // Diagonale Sud-Est: shift = -9, blocco se colonna h => mask = 0x8080808080808080
        explore_ray(from_bit, from, -9, 0x8080808080808080ULL, white_occ, black_occ, moves);
        // Diagonale Sud-Ovest: shift = -7, blocco se colonna a => mask = 0x0101010101010101
        explore_ray(from_bit, from, -7, 0x0101010101010101ULL, white_occ, black_occ, moves);
    }
}
