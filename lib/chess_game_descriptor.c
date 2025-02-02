/**
 * ##VERSION## "chess_game_descriptor.c 1.0"
*/

#define CHESS_GAME_DESCRIPTOR_C

#include "minimax.h"
#include "chess_game_descriptor.h"
#include "chess_state.h"
#include "chess_moves.h"
#include "chess_game_dynamics.h"
#include "chess_hash.h"
#include <stdlib.h>

game_descriptor_t* initialize_chess_game_descriptor() {
    // Inizializza il modulo hash
    chess_hash_init();

    game_descriptor_t* gd = (game_descriptor_t*)malloc(sizeof(game_descriptor_t));
    if (!gd) {
        // Gestione errore
        return NULL;
    }

    // Gestione dello stato
    gd->copy_state = chess_copy_state;
    gd->free_state = chess_free_state;

    // Gestione delle mosse
    gd->get_moves = chess_get_moves;
    gd->free_moves = chess_free_moves;
    gd->get_num_moves = chess_get_num_moves;
    gd->get_move_at = chess_get_move_at;
    gd->copy_move = chess_copy_move;
    gd->free_move = chess_free_move;

    // Applicazione e valutazione
    gd->apply_move = chess_apply_move;
    gd->is_terminal = chess_is_terminal;
    gd->evaluate = chess_evaluate;
    gd->player_to_move = chess_player_to_move;

    // Hashing e confronto
    gd->hash_state = chess_hash_state;
    gd->equals_state = chess_equals_state;

    return gd;
}
