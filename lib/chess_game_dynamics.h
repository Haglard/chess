// chess_game_dynamics.h
#ifndef CHESS_GAME_DYNAMICS_H
#define CHESS_GAME_DYNAMICS_H

#include "chess_state.h"
#include "chess_moves.h"
#include <stdint.h>   // per uint64_t
#include <stdbool.h>  // per bool

/**
 * @brief Applica una mossa allo stato di gioco corrente utilizzando Bitboards.
 *
 * @param[in] state Stato di gioco corrente (rappresentato da `bitboard_state_t`).
 * @param[in] move Mossa da applicare (rappresentata da `chess_move_t`).
 * @return Nuovo stato di gioco risultante dopo l'applicazione della mossa,
 *         allocato dinamicamente, oppure NULL in caso di errore.
 *
 * @note È responsabilità dell'utente liberare la memoria del nuovo stato con `chess_free_state`.
 */
void* chess_apply_move(const void *state, const void *move);

/**
 * @brief Verifica se uno stato di gioco è terminale (scacco matto, stallo, patta, ecc.).
 *
 * @param[in] state Stato di gioco da verificare (rappresentato da `bitboard_state_t`).
 * @return 1 se lo stato è terminale, 0 altrimenti.
 */
int chess_is_terminal(const void *state);

/**
 * @brief Valuta uno stato di gioco.
 *
 * @param[in] state Stato di gioco da valutare (rappresentato da `bitboard_state_t`).
 * @return Valore euristico dello stato (positivo per vantaggio dei bianchi, negativo per vantaggio dei neri).
 */
int chess_evaluate(const void *state);

/**
 * @brief Determina quale giocatore deve muovere.
 *
 * @param[in] state Stato corrente del gioco (rappresentato da `bitboard_state_t`).
 * @return Identificatore del giocatore da muovere (1 = Bianco, -1 = Nero).
 */
int chess_player_to_move(const void *state);

/*
 * Tutte le varie funzioni di supporto per gestire il gioco
 */

/**
 * @brief Restituisce l'indice (0..63) della casella del Re di \p player, o -1 se non trovato.
 */
int get_king_position(const bitboard_state_t *state, int player);

/**
 * @brief Verifica se il pezzo che muove in \p move è "pinned" (lascia il Re in scacco).
 */
bool is_move_pinned(const bitboard_state_t *state, const chess_move_t *move);

/**
 * @brief Verifica se il Re di \p player è sotto scacco nello stato \p state.
 */
bool is_king_in_check(const bitboard_state_t *state, int player);

/**
 * @brief Verifica se una casella \p square è sotto attacco da un pedone di \p attacker_player.
 */
bool is_attacked_by_pawn(const bitboard_state_t *state,
                         int square,
                         int attacker_player);

/**
 * @brief Verifica se una casella \p square è sotto attacco da un cavallo di \p attacker_player.
 */
bool is_attacked_by_knight(const bitboard_state_t *state,
                           int square,
                           int attacker_player);

/**
 * @brief Esegue l'esplorazione "a raggio" a partire da \p starting_bit in direzione \p delta,
 *        finché non s'incontra un pezzo o si esce dalla scacchiera.
 */
uint64_t ray_moves(uint64_t starting_bit, int delta, uint64_t occupancy);

/**
 * @brief Verifica se \p square è attaccata diagonalmente da un alfiere o regina \p attacker_player.
 */
bool is_attacked_by_bishop_or_queen(const bitboard_state_t *state,
                                    int square,
                                    int attacker_player);

/**
 * @brief Verifica se \p square è attaccata su riga/colonna da una torre o regina \p attacker_player.
 */
bool is_attacked_by_rook_or_queen(const bitboard_state_t *state,
                                  int square,
                                  int attacker_player);

/**
 * @brief Verifica se \p square è attaccata dal re di \p attacker_player.
 */
bool is_attacked_by_king(const bitboard_state_t *state,
                         int square,
                         int attacker_player);

/**
 * @brief Verifica se la casella \p square è sotto attacco da parte di \p attacker_player
 *        (pedoni, cavalli, alfieri, torri, regina, re).
 */
bool is_square_attacked(const bitboard_state_t *state, int square, int attacker_player);

/**
 * @brief Esegue la mossa di arrocco (sposta Re e Torre).
 */
void apply_castling(bitboard_state_t *new_state, const chess_move_t *move);

/**
 * @brief Esegue la cattura en passant (rimuove il pedone avversario).
 */
void apply_en_passant(bitboard_state_t *new_state, const chess_move_t *move);

/**
 * @brief Applica una mossa regolare (non arrocco, non en passant, non promozione).
 *        Gestisce eventuali catture (rimuovendo il pezzo avversario).
 *
 * @param[in,out] new_state Stato da modificare.
 * @param[in]     move      Mossa da applicare.
 * @return \c true se questa mossa ha coinvolto un pedone o è stata una cattura;
 *         \c false in caso di mossa regolare con pezzo diverso dal pedone e senza cattura.
 */
bool apply_regular_move(bitboard_state_t *new_state, const chess_move_t *move);

/**
 * @brief Gestisce la promozione del pedone (in Cavallo/Alfiere/Torre/Regina).
 */
void handle_promotion(bitboard_state_t *new_state, const chess_move_t *move);

/**
 * @brief Aggiorna i diritti di arrocco (annulla se muove Re o una torre d'angolo).
 */
void update_castling_rights(bitboard_state_t *new_state, const chess_move_t *move);

/**
 * @brief Aggiorna il campo en_passant (abilita se pedone avanza di 2).
 */
void update_en_passant(bitboard_state_t *new_state, const chess_move_t *move);

/**
 * @brief Aggiorna i contatori di mezze mosse e di mosse totali.
 */
void update_move_counters(bitboard_state_t *new_state, bool was_capture_or_pawn_move);

/**
 * @brief Verifica se l’arrocco in \p ch_move è legale (caselle in mezzo libere e non attaccate, re non in scacco).
 */
bool is_legal_castle(bitboard_state_t *temp_state, const chess_move_t *ch_move);

/**
 * @brief Restituisce le caselle “in mezzo” (escludendo la destinazione) per un arrocco \p (from->to).
 */
int get_castling_squares(int from, int to, int squares[3]);


#endif /* CHESS_GAME_DYNAMICS_H */
