#include <stdint.h>
#include "chess_moves.h"       // Per add_move(...)
#include "chess_state.h"    // Definisce bitboard_state_t, castling_rights, ecc.

#define MASK_BLACK_SHORT_CASTLING ((1ULL << 61) | (1ULL << 62))  // f8, g8
#define MASK_BLACK_LONG_CASTLING  ((1ULL << 59) | (1ULL << 58) | (1ULL << 57)) // d8, c8, b8

void generate_black_king_moves(const bitboard_state_t *state, dynamic_vector_t *moves)
{
    uint64_t king = state->black_kings;

    // Occupato da QUALUNQUE pezzo (bianco o nero)
    uint64_t occupied = state->white_pawns   | state->white_knights | state->white_bishops |
                        state->white_rooks   | state->white_queens  | state->white_kings  |
                        state->black_pawns   | state->black_knights | state->black_bishops |
                        state->black_rooks   | state->black_queens  | state->black_kings;

    while (king) {
        int from = __builtin_ctzll(king);
        king &= (king - 1);

        // Estraiamo colonna (file) e traversa (rank)
        int from_file = from % 8;  // 0..7 (a..h)
        int from_rank = from / 8;  // 0..7 (1..8 in “reale”)

        // Possibili 8 direzioni di movimento del re (senza arrocco)
        // offset[8] = { +1, -1, +8, -8, +9, +7, -7, -9 } in uno standard bitboard,
        // ma useremo i check colonna/riga per evitare wrap.
        
        // - - - - - - - - - - - - - - - -
        // 1) Nord   => from + 8, se rank < 7
        if (from_rank < 7) {
            int to = from + 8;
            // Se la casella NON è occupata da pezzo nero
            uint64_t black_occ = (state->black_pawns   | state->black_knights | state->black_bishops |
                                  state->black_rooks   | state->black_queens  | state->black_kings);
            if (!((1ULL << to) & black_occ)) {
                add_move(moves, (uint8_t)from, (uint8_t)to, 0, 0, 0);
            }
        }
        // 2) Sud    => from - 8, se rank > 0
        if (from_rank > 0) {
            int to = from - 8;
            uint64_t black_occ = (state->black_pawns   | state->black_knights | state->black_bishops |
                                  state->black_rooks   | state->black_queens  | state->black_kings);
            if (!((1ULL << to) & black_occ)) {
                add_move(moves, (uint8_t)from, (uint8_t)to, 0, 0, 0);
            }
        }
        // 3) Est    => from + 1, se file < 7
        if (from_file < 7) {
            int to = from + 1;
            uint64_t black_occ = (state->black_pawns   | state->black_knights | state->black_bishops |
                                  state->black_rooks   | state->black_queens  | state->black_kings);
            if (!((1ULL << to) & black_occ)) {
                add_move(moves, (uint8_t)from, (uint8_t)to, 0, 0, 0);
            }
        }
        // 4) Ovest  => from - 1, se file > 0
        if (from_file > 0) {
            int to = from - 1;
            uint64_t black_occ = (state->black_pawns   | state->black_knights | state->black_bishops |
                                  state->black_rooks   | state->black_queens  | state->black_kings);
            if (!((1ULL << to) & black_occ)) {
                add_move(moves, (uint8_t)from, (uint8_t)to, 0, 0, 0);
            }
        }
        // 5) Nord-Est => +9 = +8 +1, se (from_rank < 7 && from_file < 7)
        if (from_rank < 7 && from_file < 7) {
            int to = from + 9;
            uint64_t black_occ = (state->black_pawns   | state->black_knights | state->black_bishops |
                                  state->black_rooks   | state->black_queens  | state->black_kings);
            if (!((1ULL << to) & black_occ)) {
                add_move(moves, (uint8_t)from, (uint8_t)to, 0, 0, 0);
            }
        }
        // 6) Nord-Ovest => +7 = +8 -1, se (from_rank < 7 && from_file > 0)
        if (from_rank < 7 && from_file > 0) {
            int to = from + 7;
            uint64_t black_occ = (state->black_pawns   | state->black_knights | state->black_bishops |
                                  state->black_rooks   | state->black_queens  | state->black_kings);
            if (!((1ULL << to) & black_occ)) {
                add_move(moves, (uint8_t)from, (uint8_t)to, 0, 0, 0);
            }
        }
        // 7) Sud-Est => -7 = -8 +1, se (from_rank > 0 && from_file < 7)
        if (from_rank > 0 && from_file < 7) {
            int to = from - 7;
            uint64_t black_occ = (state->black_pawns   | state->black_knights | state->black_bishops |
                                  state->black_rooks   | state->black_queens  | state->black_kings);
            if (!((1ULL << to) & black_occ)) {
                add_move(moves, (uint8_t)from, (uint8_t)to, 0, 0, 0);
            }
        }
        // 8) Sud-Ovest => -9 = -8 -1, se (from_rank > 0 && from_file > 0)
        if (from_rank > 0 && from_file > 0) {
            int to = from - 9;
            uint64_t black_occ = (state->black_pawns   | state->black_knights | state->black_bishops |
                                  state->black_rooks   | state->black_queens  | state->black_kings);
            if (!((1ULL << to) & black_occ)) {
                add_move(moves, (uint8_t)from, (uint8_t)to, 0, 0, 0);
            }
        }

        // ------------------------------------------------
        // Gestione ARROCCO (se re su e8 = bit 60)
        // ------------------------------------------------
        if (from == 60) {
            // ARROCCO CORTO (bit 0x4 = black short)
            // Se il castling_rights e se le caselle f8,g8 (bit 61,62) sono libere
            if ((state->castling_rights & 0x4) &&
                !(occupied & MASK_BLACK_SHORT_CASTLING))
            {
                // e8 -> g8 = from + 2
                add_move(moves, (uint8_t)from, (uint8_t)(from + 2), 0, 1, 0);
            }
            // ARROCCO LUNGO (bit 0x8 = black long)
            // Se castling_rights e se d8,c8,b8 (bit 59,58,57) sono libere
            if ((state->castling_rights & 0x8) &&
                !(occupied & MASK_BLACK_LONG_CASTLING))
            {
                // e8 -> c8 = from - 2
                add_move(moves, (uint8_t)from, (uint8_t)(from - 2), 0, 1, 0);
            }
        }
    }
}

// Maschere per le caselle di arrocco bianco
#define MASK_WHITE_SHORT_CASTLING ((1ULL << 5) | (1ULL << 6))    // f1,g1
#define MASK_WHITE_LONG_CASTLING  ((1ULL << 3) | (1ULL << 2) | (1ULL << 1)) // d1,c1,b1

void generate_white_king_moves(const bitboard_state_t *state, dynamic_vector_t *moves)
{
    // Il bitboard (0..63) dei re bianchi
    uint64_t king = state->white_kings;

    // Tutte le caselle occupate (bianche o nere)
    uint64_t occupied = state->white_pawns   | state->white_knights | state->white_bishops |
                        state->white_rooks   | state->white_queens  | state->white_kings  |
                        state->black_pawns   | state->black_knights | state->black_bishops |
                        state->black_rooks   | state->black_queens  | state->black_kings;

    // Mentre ci sono ancora bit di re bianco (in teoria di solito ce n'è uno solo, 
    // ma gestiamo il caso generico)
    while (king) {
        // from = indice (0..63) del bit meno significativo attivo in 'king'
        int from = __builtin_ctzll(king);
        // toglie quel bit
        king &= (king - 1);

        // Calcoliamo colonna e riga (file, rank)
        int from_file = from % 8;  // 0..7
        int from_rank = from / 8;  // 0..7

        // Creiamo una maschera dei pezzi bianchi per evitare di catturare pezzi propri
        uint64_t white_occ = state->white_pawns | state->white_knights | state->white_bishops |
                             state->white_rooks | state->white_queens  | state->white_kings;

        // 1) Nord => from + 8, se from_rank < 7
        if (from_rank < 7) {
            int to = from + 8;
            // se non c'è un pezzo bianco in quella casella
            if (!((1ULL << to) & white_occ)) {
                add_move(moves, (uint8_t)from, (uint8_t)to, 0, 0, 0);
            }
        }
        // 2) Sud => from - 8, se from_rank > 0
        if (from_rank > 0) {
            int to = from - 8;
            if (!((1ULL << to) & white_occ)) {
                add_move(moves, (uint8_t)from, (uint8_t)to, 0, 0, 0);
            }
        }
        // 3) Est => from + 1, se from_file < 7
        if (from_file < 7) {
            int to = from + 1;
            if (!((1ULL << to) & white_occ)) {
                add_move(moves, (uint8_t)from, (uint8_t)to, 0, 0, 0);
            }
        }
        // 4) Ovest => from - 1, se from_file > 0
        if (from_file > 0) {
            int to = from - 1;
            if (!((1ULL << to) & white_occ)) {
                add_move(moves, (uint8_t)from, (uint8_t)to, 0, 0, 0);
            }
        }
        // 5) Nord-Est => from + 9, se (from_file<7 && from_rank<7)
        if (from_file < 7 && from_rank < 7) {
            int to = from + 9;
            if (!((1ULL << to) & white_occ)) {
                add_move(moves, (uint8_t)from, (uint8_t)to, 0, 0, 0);
            }
        }
        // 6) Nord-Ovest => from + 7, se (from_file>0 && from_rank<7)
        if (from_file > 0 && from_rank < 7) {
            int to = from + 7;
            if (!((1ULL << to) & white_occ)) {
                add_move(moves, (uint8_t)from, (uint8_t)to, 0, 0, 0);
            }
        }
        // 7) Sud-Est => from - 7, se (from_file<7 && from_rank>0)
        if (from_file < 7 && from_rank > 0) {
            int to = from - 7;
            if (!((1ULL << to) & white_occ)) {
                add_move(moves, (uint8_t)from, (uint8_t)to, 0, 0, 0);
            }
        }
        // 8) Sud-Ovest => from - 9, se (from_file>0 && from_rank>0)
        if (from_file > 0 && from_rank > 0) {
            int to = from - 9;
            if (!((1ULL << to) & white_occ)) {
                add_move(moves, (uint8_t)from, (uint8_t)to, 0, 0, 0);
            }
        }

        // -----------------------------------
        // Gestione ARROCCO (se re su e1=bit 4)
        // -----------------------------------
        if (from == 4) {
            // ARROCCO CORTO (bit 0x1 = white short)
            // f1=5, g1=6 => se entrambe libere
            if ((state->castling_rights & 0x1) &&
                !(occupied & MASK_WHITE_SHORT_CASTLING))
            {
                // e1->g1 => from+2
                add_move(moves, (uint8_t)from, (uint8_t)(from + 2), 0, 1, 0);
            }

            // ARROCCO LUNGO (bit 0x2 = white long)
            // d1=3, c1=2, b1=1 => se tutte libere
            if ((state->castling_rights & 0x2) &&
                !(occupied & MASK_WHITE_LONG_CASTLING))
            {
                // e1->c1 => from-2
                add_move(moves, (uint8_t)from, (uint8_t)(from - 2), 0, 1, 0);
            }
        }
    }
}
