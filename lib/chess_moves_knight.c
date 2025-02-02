/**
 * ##VERSION## "chess_moves_knight.c 1.2"
 *
 */

#include "chess_moves_knight.h"
#include "chess_moves.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* -------------------- CAVALLI NERI -------------------- */

void generate_black_knight_moves(const bitboard_state_t *state, dynamic_vector_t *moves) {
    // (Invariato rispetto al codice iniziale, salvo gestione corretta dei propri pezzi)
    uint64_t knights = state->black_knights;

    while (knights) {
        int from = __builtin_ctzll(knights);
        knights &= knights - 1;

        uint64_t knight_bit = 1ULL << from;
        uint64_t attacks = 0;

        attacks |= (knight_bit & NOT_H_FILE) << 17; // +2 rank, +1 file
        attacks |= (knight_bit & NOT_A_FILE) << 15; // +2 rank, -1 file
        attacks |= (knight_bit & NOT_H_FILE) >> 15; // -2 rank, +1 file
        attacks |= (knight_bit & NOT_A_FILE) >> 17; // -2 rank, -1 file

        attacks |= (knight_bit & NOT_HG_FILE) << 10; // +1 rank, +2 file
        attacks |= (knight_bit & NOT_AB_FILE) << 6;  // +1 rank, -2 file
        attacks |= (knight_bit & NOT_HG_FILE) >> 6;  // -1 rank, +2 file
        attacks |= (knight_bit & NOT_AB_FILE) >> 10; // -1 rank, -2 file

        // Rimuove i pezzi neri
        uint64_t black_occ = state->black_pawns | state->black_knights | state->black_bishops |
                             state->black_rooks | state->black_queens | state->black_kings;
        attacks &= ~black_occ;

        while (attacks) {
            int to = __builtin_ctzll(attacks);
            attacks &= attacks - 1;
            add_move(moves, (uint8_t)from, (uint8_t)to, 0, 0, 0);
        }
    }
}

/**
 * @brief Genera tutte le mosse legali per i cavalli bianchi.
 */
void generate_white_knight_moves(const bitboard_state_t *state, dynamic_vector_t *moves) {
    uint64_t knights = state->white_knights;

    while (knights) {
        int from = __builtin_ctzll(knights);
        knights &= knights - 1;

        uint64_t knight_bit = 1ULL << from;
        uint64_t attacks = 0;

        // +2 traversa, +1 colonna
        attacks |= (knight_bit & NOT_H_FILE) << 17;
        // +2 traversa, -1 colonna
        attacks |= (knight_bit & NOT_A_FILE) << 15;
        // -2 traversa, +1 colonna
        attacks |= (knight_bit & NOT_H_FILE) >> 15;
        // -2 traversa, -1 colonna
        attacks |= (knight_bit & NOT_A_FILE) >> 17;

        // +1 traversa, +2 colonne
        attacks |= (knight_bit & NOT_HG_FILE) << 10;
        // +1 traversa, -2 colonne
        attacks |= (knight_bit & NOT_AB_FILE) << 6;
        // -1 traversa, +2 colonne
        attacks |= (knight_bit & NOT_HG_FILE) >> 6;
        // -1 traversa, -2 colonne
        attacks |= (knight_bit & NOT_AB_FILE) >> 10;

        // Rimuove i pezzi bianchi
        uint64_t white_occ = state->white_pawns | state->white_knights | state->white_bishops |
                             state->white_rooks | state->white_queens | state->white_kings;
        attacks &= ~white_occ;

        // Aggiungi le mosse valide
        while (attacks) {
            int to = __builtin_ctzll(attacks);
            attacks &= attacks - 1;
            add_move(moves, from, to, 0, 0, 0);
        }
    }
}

