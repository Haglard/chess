/**
 * ##VERSION## "chess_moves.c 1.0"
 */

#include "chess_moves.h"
#include "chess_moves_pawn.h"
#include "chess_moves_knight.h"
#include "chess_moves_bishop.h"
#include "chess_moves_rook.h"
#include "chess_moves_queen.h"
#include "chess_moves_king.h"
#include <stdlib.h>
#include <string.h>

/* --------------------------------------------------------------------------
 * DEFINIZIONE DELLE FUNZIONI DI GENERAZIONE COMPLETA DELLE MOSSE
 * --------------------------------------------------------------------------
 */

/**
 * @brief Implementa la funzione di callback per ottenere tutte le mosse legali.
 *
 * Questa funzione aggrega tutte le mosse legali per ogni tipo di pezzo presente
 * nello stato corrente del gioco. Utilizza le funzioni specifiche per generare
 * mosse per pedoni, cavalli, alfieri, torri, regine e re, e le aggiunge al vettore
 * dinamico fornito.
 *
 * @param[in] state_void Puntatore allo stato corrente del gioco (`bitboard_state_t`).
 * @return Un puntatore a `dynamic_vector_t` contenente tutte le mosse legali (`chess_move_t*`).
 *
 * @note
 *   - La funzione alloca dinamicamente il vettore di mosse, pertanto è responsabilità
 *     dell'utente liberare la memoria utilizzando `chess_free_moves`.
 *   - Assicurarsi che lo stato di gioco passato sia valido e correttamente inizializzato.
 */
dynamic_vector_t* chess_get_moves(const void *state_void) {
    if (!state_void) return NULL;
    const bitboard_state_t *state = (const bitboard_state_t*)state_void;
    dynamic_vector_t *moves = dv_create();
    if (!moves) return NULL;

    // Verifichiamo chi deve muovere (Bianco = 1, Nero = -1 o altro)
    if (state->current_player == 1) {
        // Muove il Bianco => generiamo solo le mosse dei pezzi bianchi
        generate_white_pawn_moves(state, moves);
        generate_white_knight_moves(state, moves);
        generate_white_bishop_moves(state, moves);
        generate_white_rook_moves(state, moves);
        generate_white_queen_moves(state, moves);
        generate_white_king_moves(state, moves);
    } else {
        // Muove il Nero
        generate_black_pawn_moves(state, moves);
        generate_black_knight_moves(state, moves);
        generate_black_bishop_moves(state, moves);
        generate_black_rook_moves(state, moves);
        generate_black_queen_moves(state, moves);
        generate_black_king_moves(state, moves);
    }

    return moves;
}


/**
 * @brief Implementa la funzione di callback per liberare il vettore delle mosse.
 *
 * Questa funzione libera tutta la memoria allocata per le mosse all'interno del
 * vettore dinamico e poi libera il vettore stesso. È essenziale chiamare questa
 * funzione per evitare perdite di memoria dopo aver terminato l'utilizzo delle mosse.
 *
 * @param[in] moves_vec Vettore dinamico contenente le mosse (`chess_move_t*`).
 *
 * @note
 *   - Dopo la chiamata a questa funzione, `moves_vec` non sarà più valido.
 *   - Assicurarsi di non utilizzare il vettore dopo averlo liberato.
 */
void chess_free_moves(dynamic_vector_t *moves_vec) {
    if (!moves_vec) return;
    int num_moves = dv_size(moves_vec);
    for (int i = 0; i < num_moves; i++) {
        chess_move_t *move = (chess_move_t*)dv_get(moves_vec, i);
        if (move) {
            free(move);
        }
    }
    dv_free(moves_vec);
}

/**
 * @brief Implementa la funzione di callback per ottenere il numero di mosse.
 *
 * Questa funzione restituisce il numero totale di mosse presenti nel vettore dinamico.
 * È utile per iterare sulle mosse e accedere a ciascuna di esse.
 *
 * @param[in] moves_vec Vettore dinamico contenente le mosse (`chess_move_t*`).
 * @return Numero di mosse presenti nel vettore.
 *
 * @note
 *   - Se `moves_vec` è `NULL`, la funzione potrebbe restituire un valore non valido.
 *   - È responsabilità dell'utente verificare che il vettore non sia vuoto prima di accedere alle mosse.
 */
int chess_get_num_moves(const dynamic_vector_t *moves_vec) {
    if (!moves_vec) return 0;
    return dv_size(moves_vec);
}

/**
 * @brief Implementa la funzione di callback per accedere a una mossa specifica.
 *
 * Questa funzione permette di accedere a una specifica mossa all'interno del
 * vettore dinamico utilizzando l'indice fornito. Se l'indice è valido, viene
 * restituito un puntatore alla mossa richiesta; altrimenti, viene restituito `NULL`.
 *
 * @param[in] moves_vec Vettore dinamico contenente le mosse (`chess_move_t*`).
 * @param[in] index Indice della mossa da accedere (da 0 a n-1).
 * @return Puntatore alla mossa richiesta (`chess_move_t*`), oppure `NULL` se l'indice è fuori range.
 *
 * @note
 *   - Gli indici validi vanno da 0 a `chess_get_num_moves(moves_vec) - 1`.
 *   - È responsabilità dell'utente gestire correttamente i casi in cui l'indice è fuori range.
 */
void* chess_get_move_at(const dynamic_vector_t *moves_vec, int index) {
    if (!moves_vec) return NULL;
    return dv_get(moves_vec, index);
}

/**
 * @brief Implementa la funzione di callback per copiare una mossa.
 *
 * Questa funzione crea una copia indipendente di una mossa esistente, allocando
 * dinamicamente la memoria necessaria. È utile quando si desidera duplicare
 * una mossa per modificarla senza alterare l'originale.
 *
 * @param[in] move_void Mossa da copiare (`chess_move_t*`).
 * @return Puntatore a una nuova mossa copiata (`chess_move_t*`), allocata dinamicamente, oppure `NULL` in caso di errore.
 *
 * @note
 *   - È responsabilità dell'utente liberare la memoria della mossa copiata utilizzando `chess_free_move`.
 *   - Assicurarsi che il puntatore `move_void` sia valido e punti a una mossa correttamente inizializzata.
 */
void* chess_copy_move(const void *move_void) {
    if (!move_void) return NULL;
    const chess_move_t *move = (const chess_move_t*)move_void;
    chess_move_t *new_move = (chess_move_t*)malloc(sizeof(chess_move_t));
    if (!new_move) return NULL;
    memcpy(new_move, move, sizeof(chess_move_t));
    return new_move;
}

/**
 * @brief Implementa la funzione di callback per liberare una mossa.
 *
 * Questa funzione libera la memoria allocata per una singola mossa. È essenziale
 * chiamare questa funzione per evitare perdite di memoria dopo aver terminato
 * l'utilizzo di una mossa specifica.
 *
 * @param[in] move_void Mossa da liberare (`chess_move_t*`).
 *
 * @note
 *   - Dopo la chiamata a questa funzione, il puntatore `move_void` non sarà più valido.
 *   - Assicurarsi di non utilizzare la mossa dopo averla liberata.
 */
void chess_free_move(void *move_void) {
    if (move_void) {
        free(move_void);
    }
}

/* --------------------------------------------------------------------------
 * DEFINIZIONE DELLE ALTRE FUNZIONI DI GENERAZIONE DELLE MOSSE
 * --------------------------------------------------------------------------
 * Nota: Le funzioni generate_white_bishop_moves, generate_black_bishop_moves,
 * generate_white_rook_moves, generate_black_rook_moves, generate_white_queen_moves,
 * generate_black_queen_moves, generate_white_king_moves e generate_black_king_moves
 * sono implementate nei rispettivi file chess_moves_white.c e chess_moves_black.c.
 * Queste funzioni devono calcolare le mosse legali per ogni tipo di pezzo,
 * considerando gli ostacoli sulla scacchiera e le regole specifiche del pezzo.
 * --------------------------------------------------------------------------
 */

/**
 * @brief Aggiunge una nuova mossa al vettore delle mosse.
 *
 * @param[in] moves       Vettore dinamico dove aggiungere la mossa.
 * @param[in] from        Casella di partenza (0-63).
 * @param[in] to          Casella di arrivo (0-63).
 * @param[in] promotion   Tipo di promozione (0 = nessuna, 1 = Cavallo, 2 = Alfiere, 3 = Torre, 4 = Regina).
 * @param[in] is_castling Flag per arrocco (1 = sì, 0 = no).
 * @param[in] is_en_passant Flag per en passant (1 = sì, 0 = no).
 */
void add_move(dynamic_vector_t *moves, uint8_t from, uint8_t to, 
                     uint8_t promotion, uint8_t is_castling, uint8_t is_en_passant)
{
    chess_move_t *move = (chess_move_t*)malloc(sizeof(chess_move_t));
    if (!move) {
        // Possibile log errore
        return;
    }
    move->from = from;
    move->to = to;
    move->promotion = promotion;
    move->is_castling = is_castling;
    move->is_en_passant = is_en_passant;
    dv_push_back(moves, move);
}

/* --------------------------------------------------------------------------
 * FUNZIONI DI ESPLORAZIONE "A RAGGIO"
 * (utili a Alfieri, Torri e Regine)
 * --------------------------------------------------------------------------
 */

/**
 * @brief Esplora una direzione (ray) spostandosi di `shift` bit a ogni step,
 *        finché non incontra un pezzo o esce dalla scacchiera.
 *
 * @param[in] from_bit   Bitboard con un singolo bit “di partenza” già acceso
 * @param[in] from       Indice (0-63) della casella di partenza
 * @param[in] shift      Movimento in termini di bit (±1, ±7, ±8, ±9)
 * @param[in] block_mask [IGNORATO in questa versione] maschera eventuale
 * @param[in] own_occ    Bitboard dei pezzi propri (che bloccano la continuazione)
 * @param[in] opp_occ    Bitboard dei pezzi avversari (catturabili, poi stop)
 * @param[in,out] moves  Vettore dinamico in cui aggiungere le mosse
 */
void explore_ray(uint64_t from_bit, int from, int shift,
                 uint64_t block_mask,
                 uint64_t own_occ, uint64_t opp_occ,
                 dynamic_vector_t *moves)
{
    // Calcola la colonna e la riga di partenza
    int col = from % 8;   // 0..7
    int row = from / 8;   // 0..7

    // Determina di quanto varia colonna e riga
    // in base al "shift". Se la tua enumerazione bitboard è classica
    // "a1=0, b1=1,... h8=63" (row-major, bottom-up), e i movimenti:
    //   +1 = est, -1 = ovest, +8 = nord, -8 = sud,
    //   +7 = nord-est, +9 = nord-ovest, -7 = sud-ovest, -9 = sud-est
    // allora puoi mappare shift -> (dc, dr) come sotto.
    // (Se la tua enumerazione differisce, regola i valori di dc, dr.)

    int dc = 0, dr = 0; 
    switch (shift) {
        case +1:  dc = +1; dr =  0; break;  // Est
        case -1:  dc = -1; dr =  0; break;  // Ovest
        case +8:  dc =  0; dr = +1; break;  // Nord
        case -8:  dc =  0; dr = -1; break;  // Sud

        case +7:  dc = +1; dr = +1; break;  // Nord-Est
        case +9:  dc = -1; dr = +1; break;  // Nord-Ovest
        case -7:  dc = -1; dr = -1; break;  // Sud-Ovest
        case -9:  dc = +1; dr = -1; break;  // Sud-Est

        default:
            // Se arriva un "shift" inaspettato, esci subito
            return;
    }

    while (1) {
        // Avanza di una casella nella direzione (dc, dr)
        col += dc;
        row += dr;

        // Se usciamo dai confini (0..7) => stop
        if (col < 0 || col > 7 || row < 0 || row > 7) {
            break;
        }

        // Calcola l'indice "to" (0..63) della nuova casella
        int to = row * 8 + col;
        uint64_t to_bit = (1ULL << to);

        // Se c'è un pezzo nostro, blocco subito (nessuna mossa).
        if (to_bit & own_occ) {
            break;
        }
        // Se c'è un pezzo avversario, catturo e stop
        if (to_bit & opp_occ) {
            add_move(moves, (uint8_t)from, (uint8_t)to, 0, 0, 0);
            break;
        }
        // Casella vuota => aggiungiamo la mossa e continuiamo
        add_move(moves, (uint8_t)from, (uint8_t)to, 0, 0, 0);
    }
}


