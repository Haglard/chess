/**
 * ##VERSION## "chess_moves_pawn.c 1.2"
 *
 */

#include "chess_moves_pawn.h"
#include "chess_moves.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/**
 * @brief Genera tutte le mosse legali per i pedoni neri.
 */
void generate_black_pawn_moves(const bitboard_state_t *state, dynamic_vector_t *moves) {
    uint64_t pawns = state->black_pawns;

    // Occupato da QUALUNQUE pezzo
    uint64_t occupied = state->white_pawns | state->white_knights | state->white_bishops |
                        state->white_rooks | state->white_queens | state->white_kings |
                        state->black_pawns | state->black_knights | state->black_bishops |
                        state->black_rooks | state->black_queens | state->black_kings;
    uint64_t empty = ~occupied;

    // ------------------------------------------
    // 1) Mosse avanti di una casella (verso il basso)
    // ------------------------------------------
    // I pedoni neri si spostano di 8 bit in meno (pawns >> 8)
    uint64_t single_push = (pawns >> 8) & empty;
    while (single_push) {
        int to = __builtin_ctzll(single_push);
        single_push &= single_push - 1;
        int from = to + 8;

        // Verifichiamo promozione (rank=0 in 0-based)
        if (to / 8 == 0) {
            // Quattro possibili promozioni (Cavallo=1, Alfiere=2, Torre=3, Regina=4)
            add_move(moves, from, to, 1, 0, 0);
            add_move(moves, from, to, 2, 0, 0);
            add_move(moves, from, to, 3, 0, 0);
            add_move(moves, from, to, 4, 0, 0);
        } else {
            add_move(moves, from, to, 0, 0, 0);
        }
    }

    // ------------------------------------------
    // 2) Mosse avanti di due caselle (dalla 7ª traversa => rank=6)
    // ------------------------------------------
    uint64_t initial_pawns = pawns & 0x00FF000000000000ULL; // bit set su rank=6
    uint64_t double_push = ((initial_pawns >> 8) & empty) >> 8 & empty;
    while (double_push) {
        int to = __builtin_ctzll(double_push);
        double_push &= double_push - 1;
        int from = to + 16;
        add_move(moves, from, to, 0, 0, 0);
    }

    // ------------------------------------------
    // 3) Catture in diagonale a destra (verso il basso-destra)
    // ------------------------------------------
    // (pawns >> 7), escludendo colonna a => ~(0x0101010101010101ULL)
    // e deve catturare pezzi bianchi
    uint64_t captures_right = (pawns >> 7) & ~(0x0101010101010101ULL) &
                              (state->white_pawns | state->white_knights | state->white_bishops |
                               state->white_rooks | state->white_queens | state->white_kings);
    while (captures_right) {
        int to = __builtin_ctzll(captures_right);
        captures_right &= captures_right - 1;
        int from = to + 7;

        // Promozione?
        if (to / 8 == 0) {
            add_move(moves, from, to, 1, 0, 0);
            add_move(moves, from, to, 2, 0, 0);
            add_move(moves, from, to, 3, 0, 0);
            add_move(moves, from, to, 4, 0, 0);
        } else {
            add_move(moves, from, to, 0, 0, 0);
        }
    }

    // ------------------------------------------
    // 4) Catture in diagonale a sinistra (verso il basso-sinistra)
    // ------------------------------------------
    // (pawns >> 9), escludendo colonna h => ~(0x8080808080808080ULL)
    // cattura su pezzi bianchi
    uint64_t captures_left = (pawns >> 9) & ~(0x8080808080808080ULL) &
                             (state->white_pawns | state->white_knights | state->white_bishops |
                              state->white_rooks | state->white_queens | state->white_kings);
    while (captures_left) {
        int to = __builtin_ctzll(captures_left);
        captures_left &= captures_left - 1;
        int from = to + 9;

        // Promozione?
        if (to / 8 == 0) {
            add_move(moves, from, to, 1, 0, 0);
            add_move(moves, from, to, 2, 0, 0);
            add_move(moves, from, to, 3, 0, 0);
            add_move(moves, from, to, 4, 0, 0);
        } else {
            add_move(moves, from, to, 0, 0, 0);
        }
    }

    // ------------------------------------------
    // 5) En Passant
    // ------------------------------------------
    //  Per i pedoni neri, l'en passant si applica se il pedone è su rank=3 (0-based).
    //  Esempio: un pedone nero su e4 (rank=3, file=4) può catturare en passant su e3.
    if (state->en_passant != 255) {
        uint8_t ep_square = state->en_passant;
        uint64_t ep_bit = (uint64_t)1 << ep_square;

        // Catture en passant a destra => (pawns >> 7) & ep_bit
        // Ma bisogna controllare che (from / 8) == 3
        uint64_t en_passant_right = (pawns >> 7) & ep_bit;
        if (en_passant_right) {
            int to = ep_square;     // es. e3
            int from = to + 7;      // pedone era su d4 (rank=3)
            if ((from / 8) == 3) {
                add_move(moves, from, to, 0, 0, 1);
            }
        }

        // Catture en passant a sinistra => (pawns >> 9) & ep_bit
        // E serve (from / 8) == 3
        uint64_t en_passant_left = (pawns >> 9) & ep_bit;
        if (en_passant_left) {
            int to = ep_square;
            int from = to + 9;
            if ((from / 8) == 3) {
                add_move(moves, from, to, 0, 0, 1);
            }
        }
    }
}

/**
 * @brief Genera tutte le mosse legali per i pedoni bianchi.
 */
void generate_white_pawn_moves(const bitboard_state_t *state, dynamic_vector_t *moves) {
    // Bitboard dei pedoni bianchi
    uint64_t pawns = state->white_pawns;

    // Bitboard dei pezzi occupati (bianchi o neri)
    uint64_t occupied = state->white_pawns | state->white_knights | state->white_bishops |
                        state->white_rooks | state->white_queens | state->white_kings |
                        state->black_pawns | state->black_knights | state->black_bishops |
                        state->black_rooks | state->black_queens | state->black_kings;
    // Caselle libere
    uint64_t empty = ~occupied;

    // ---------------------------------------------------------
    // 1) Mosse avanti di una casella
    // ---------------------------------------------------------
    uint64_t single_push = (pawns << 8) & empty;
    while (single_push) {
        int to = __builtin_ctzll(single_push);
        single_push &= single_push - 1;
        int from = to - 8;

        // Se arriviamo in 8a traversa (rank == 7 in 0-based), promozione
        if (to / 8 == 7) {
            // Quattro possibili promozioni: Cavallo=1, Alfiere=2, Torre=3, Regina=4
            add_move(moves, from, to, 1, 0, 0);
            add_move(moves, from, to, 2, 0, 0);
            add_move(moves, from, to, 3, 0, 0);
            add_move(moves, from, to, 4, 0, 0);
        } else {
            add_move(moves, from, to, 0, 0, 0);
        }
    }

    // ---------------------------------------------------------
    // 2) Mosse avanti di due caselle (solo se il pedone è ancora sulla seconda traversa)
    // ---------------------------------------------------------
    uint64_t initial_pawns = pawns & 0x000000000000FF00ULL;  // rank 1 (0-based)
    uint64_t double_push = ((initial_pawns << 8) & empty) << 8 & empty;
    while (double_push) {
        int to = __builtin_ctzll(double_push);
        double_push &= double_push - 1;
        int from = to - 16;
        add_move(moves, from, to, 0, 0, 0);
    }

    // ---------------------------------------------------------
    // 3) Catture in diagonale a destra
    // ---------------------------------------------------------
    // (pawns << 9) escludendo colonna 'a' => ~(0x0101010101010101ULL)
    // e deve catturare pezzi neri
    uint64_t captures_right = (pawns << 9) & ~(0x0101010101010101ULL) &
                              (state->black_pawns | state->black_knights | state->black_bishops |
                               state->black_rooks | state->black_queens | state->black_kings);
    while (captures_right) {
        int to = __builtin_ctzll(captures_right);
        captures_right &= captures_right - 1;
        int from = to - 9;

        // Se arriviamo in 8a traversa => promozione
        if (to / 8 == 7) {
            add_move(moves, from, to, 1, 0, 0);
            add_move(moves, from, to, 2, 0, 0);
            add_move(moves, from, to, 3, 0, 0);
            add_move(moves, from, to, 4, 0, 0);
        } else {
            add_move(moves, from, to, 0, 0, 0);
        }
    }

    // ---------------------------------------------------------
    // 4) Catture in diagonale a sinistra
    // ---------------------------------------------------------
    // (pawns << 7) escludendo colonna 'h' => ~(0x8080808080808080ULL)
    // cattura su pezzi neri
    uint64_t captures_left = (pawns << 7) & ~(0x8080808080808080ULL) &
                             (state->black_pawns | state->black_knights | state->black_bishops |
                              state->black_rooks | state->black_queens | state->black_kings);
    while (captures_left) {
        int to = __builtin_ctzll(captures_left);
        captures_left &= captures_left - 1;
        int from = to - 7;

        // Promozione?
        if (to / 8 == 7) {
            add_move(moves, from, to, 1, 0, 0);
            add_move(moves, from, to, 2, 0, 0);
            add_move(moves, from, to, 3, 0, 0);
            add_move(moves, from, to, 4, 0, 0);
        } else {
            add_move(moves, from, to, 0, 0, 0);
        }
    }

    // ---------------------------------------------------------
    // 5) En Passant
    // ---------------------------------------------------------
    if (state->en_passant != 255) {
        uint8_t ep_square = state->en_passant;
        uint64_t ep_bit = (uint64_t)1 << ep_square;

        // Catture en passant a destra
        // Pedone cattura spostandosi +9, ma deve venire dalla traversa 4 (0-based)
        // ossia rank=4 => row=4 => from/8 == 4
        uint64_t en_passant_right = (pawns << 9) & ep_bit;
        if (en_passant_right) {
            int to = ep_square;    // es. e3
            int from = to - 9;     // pedone su f4, rank=4
            if ((from / 8) == 4) {
                add_move(moves, from, to, 0, 0, 1);
            }
        }

        // Catture en passant a sinistra
        // +7 => analogamente controlliamo che rank=4
        uint64_t en_passant_left = (pawns << 7) & ep_bit;
        if (en_passant_left) {
            int to = ep_square;
            int from = to - 7;
            if ((from / 8) == 4) {
                add_move(moves, from, to, 0, 0, 1);
            }
        }
    }
}
