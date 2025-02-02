// chess_moves_rook.h
#ifndef CHESS_MOVES_ROOK_H
#define CHESS_MOVES_ROOK_H

#include "chess_state.h"
#include "obj_dynamic_vector.h"
#include <stdint.h>

/* Prototipi delle funzioni per generare le mosse dei pezzi neri */
void generate_black_rook_moves(const bitboard_state_t *state, dynamic_vector_t *moves);
void generate_white_rook_moves(const bitboard_state_t *state, dynamic_vector_t *moves);

#endif /* CHESS_MOVES_ROOK_H */
