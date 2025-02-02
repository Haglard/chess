// chess_game_dynamics.c
#include "chess_game_dynamics.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include "chess_hash.h"
#include "chess_state.h"
#include "chess_moves.h"
#include "obj_trace.h"

/**
 * @brief Restituisce il bitboard di tutti i pezzi bianchi nello stato \p s.
 *
 * @param s Puntatore allo stato bitboard.
 * @return Bitmask con 1 sulle caselle contenenti pezzi bianchi.
 */
static inline uint64_t compute_white_occ(const bitboard_state_t *s)
{
    return (s->white_pawns   | s->white_knights |
            s->white_bishops | s->white_rooks   |
            s->white_queens  | s->white_kings);
}

/**
 * @brief Restituisce il bitboard di tutti i pezzi neri nello stato \p s.
 *
 * @param s Puntatore allo stato bitboard.
 * @return Bitmask con 1 sulle caselle contenenti pezzi neri.
 */
static inline uint64_t compute_black_occ(const bitboard_state_t *s)
{
    return (s->black_pawns   | s->black_knights |
            s->black_bishops | s->black_rooks   |
            s->black_queens  | s->black_kings);
}

/**
 * @brief Restituisce il bitboard di tutti i pezzi (bianchi o neri).
 *
 * @param s Puntatore allo stato bitboard.
 * @return Bitmask con 1 sulle caselle contenenti pezzi di entrambi i colori.
 */
static inline uint64_t compute_all_occ(const bitboard_state_t *s)
{
    // Usiamo le funzioni inline appena definite
    return compute_white_occ(s) | compute_black_occ(s);
}


/**
 * @brief Restituisce l'indice (0-63) della casella occupata dal Re di `player`.
 *        Restituisce -1 se non lo trova (caso anomalo).
 *
 * @param[in] state   Stato di gioco (bitboard).
 * @param[in] player  1 = bianco, -1 = nero.
 * @return Indice (0-63) della casella del Re, oppure -1 se non presente.
 */
int get_king_position(const bitboard_state_t *state, int player)
{
    uint64_t king_bb = (player == 1)
                       ? state->white_kings
                       : state->black_kings;

    if (king_bb == 0ULL) {
        // Non trovato re? Caso anomalo
        return -1;
    }

    // Trova il primo (e unico) bit a 1.
    // Usando GCC/Clang: __builtin_ctzll(king_bb) conta gli zeri a destra
    // (lowest set bit). In altri compilatori, implementa una funzione analoga.
#ifdef __GNUC__
    int index = __builtin_ctzll(king_bb);
#else
    // Sostituisci con la tua routine di "bit scan forward".
    // Per semplicità, come placeholder:
    int index = 0; 
    while (index < 64 && !(king_bb & (1ULL << index))) {
        index++;
    }
    if (index == 64) index = -1;  // caso se non trovato
#endif

    return index;
}


/**
 * @brief Rimuove (spegne) il bit corrispondente al pezzo del giocatore `player`
 *        in `from_square`. Aggiorna i bitboard dei pezzi bianchi/neri,
 *        ma non abbiamo campi di occupancy (white_occ, black_occ), quindi
 *        non aggiorniamo nulla di simile.
 *
 * @param[in,out] tempState   Stato “copia” da modificare.
 * @param[in]     from_square Indice 0-63 della casella di partenza.
 * @param[in]     player      1 = bianco, -1 = nero.
 *
 * @note Se la casella non contiene effettivamente un pezzo di `player`,
 *       potresti implementare un controllo extra e gestire l’anomalia.
 */
void remove_piece_from_bitboards(bitboard_state_t *tempState, int from_square, int player)
{
    uint64_t mask = 1ULL << from_square;

    if (player == 1) {
        // Possibili pezzi bianchi
        if (tempState->white_pawns & mask) {
            tempState->white_pawns &= ~mask;
        } else if (tempState->white_knights & mask) {
            tempState->white_knights &= ~mask;
        } else if (tempState->white_bishops & mask) {
            tempState->white_bishops &= ~mask;
        } else if (tempState->white_rooks & mask) {
            tempState->white_rooks &= ~mask;
        } else if (tempState->white_queens & mask) {
            tempState->white_queens &= ~mask;
        } else if (tempState->white_kings & mask) {
            tempState->white_kings &= ~mask;
        } 
    } else {
        // Possibili pezzi neri
        if (tempState->black_pawns & mask) {
            tempState->black_pawns &= ~mask;
        } else if (tempState->black_knights & mask) {
            tempState->black_knights &= ~mask;
        } else if (tempState->black_bishops & mask) {
            tempState->black_bishops &= ~mask;
        } else if (tempState->black_rooks & mask) {
            tempState->black_rooks &= ~mask;
        } else if (tempState->black_queens & mask) {
            tempState->black_queens &= ~mask;
        } else if (tempState->black_kings & mask) {
            tempState->black_kings &= ~mask;
        }
    }
}


/**
 * @brief Crea una copia dello stato e rimuove il pezzo situato in `move->from`.
 *
 *        L’idea è simulare che il pezzo si “stacchi” dalla sua posizione,
 *        in modo da capire se ciò libera una linea d’attacco verso il Re.
 *
 * @param[in]  state   Stato di gioco originale (bitboard_state_t).
 * @param[in]  move    Mossa (chess_move_t) con la casella di partenza `from`.
 * @return     Puntatore a un nuovo bitboard_state_t con quel pezzo rimosso,
 *             oppure `NULL` in caso di errore (out-of-memory).
 *
 * @note   Il chiamante deve poi chiamare `free(...)` su questo oggetto
 *         quando ha finito il controllo.
 */
bitboard_state_t* simulate_position_without_piece(const bitboard_state_t *state,
                                                         const chess_move_t *move)
{
    // Alloco una nuova struttura
    bitboard_state_t *tempState = (bitboard_state_t*)malloc(sizeof(bitboard_state_t));
    if (!tempState) {
        return NULL; // out-of-memory
    }

    // Copia superficiale
    *tempState = *state;

    // Rimuovo (spegno) il pezzo dal bitboard corrispondente
    remove_piece_from_bitboards(tempState, move->from, state->current_player);

    return tempState;
}


/**
 * @brief Verifica se la casella `square` è attaccata da un pedone di colore `attacker_player`.
 *
 * Per convenzione, `attacker_player == 1` => pedoni bianchi,
 *               `attacker_player == -1` => pedoni neri.
 *
 * Ricorda:
 *  - I pedoni bianchi attaccano (posizione - 7) e (posizione - 9) sulle colonne,
 *    se non stiamo uscendo dai file A/H.
 *  - I pedoni neri attaccano (posizione + 7) e (posizione + 9).
 */
bool is_attacked_by_pawn(const bitboard_state_t *state, int square, int attacker_player)
{
    uint64_t attacker_pawns = (attacker_player == 1)
                              ? state->white_pawns
                              : state->black_pawns;

    // Calcoliamo la maschera per la casella "square"
    const uint64_t mask = 1ULL << square;

    if (attacker_player == 1) {
        // Bianchi: un pedone bianco attacca "square" se "square" è diagonale
        // in alto a sinistra o in alto a destra rispetto al pedone.
        // Invertendo il discorso: se "square" è la destinazione, la posizione
        // di partenza del pedone è "square + 7" (destra) o "square + 9" (sinistra).
        // Attenzione al wrap sugli estremi del file.
        if (((mask << 7) & attacker_pawns & NOT_H_FILE) != 0ULL) return true;
        if (((mask << 9) & attacker_pawns & NOT_A_FILE) != 0ULL) return true;
    } else {
        // Neri: un pedone nero attacca "square" se "square" è diagonale
        // in basso a sinistra (square - 9) o in basso a destra (square - 7).
        if (((mask >> 7) & attacker_pawns & NOT_A_FILE) != 0ULL) return true;
        if (((mask >> 9) & attacker_pawns & NOT_H_FILE) != 0ULL) return true;
    }

    return false;
}

/**
 * @brief Verifica se la casella `square` è attaccata da un cavallo di colore `attacker_player`.
 *
 * I cavalli si muovono a "L": ±1 in colonna e ±2 in riga (o viceversa).
 * Usiamo bitboard shift e maschere NOT_A_FILE, NOT_H_FILE per evitare wrap su file A/H.
 */
bool is_attacked_by_knight(const bitboard_state_t *state, int square, int attacker_player)
{
    uint64_t attacker_knights = (attacker_player == 1)
                                ? state->white_knights
                                : state->black_knights;

    // Costruiamo una maschera con un 1 sul bit "square"
    uint64_t sq_mask = 1ULL << square;

    // Verifichiamo se un cavallo dell'avversario può occupare "square".
    // (In altre parole, spostiamo "sq_mask" nelle posizioni da cui un cavallo
    // arriverebbe a "square" e controlliamo se coincide con un cavallo avversario).
    uint64_t knights_from_positions = 0ULL;

    // Cavalcando "from" su quell square, i possibili "from" sono:
    // shift a sinistra / destra di 6, 10, 15, 17 con maschere di file.

    // 2 verso sinistra e 1 in alto (square + 6)
    if ((sq_mask << 6) & NOT_GH_FILE) {
        knights_from_positions |= (sq_mask << 6);
    }
    // 2 verso sinistra e 1 in basso (square - 10)
    if ((sq_mask >> 10) & NOT_GH_FILE) {
        knights_from_positions |= (sq_mask >> 10);
    }
    // 2 verso destra e 1 in alto (square + 10)
    if ((sq_mask << 10) & NOT_AB_FILE) {
        knights_from_positions |= (sq_mask << 10);
    }
    // 2 verso destra e 1 in basso (square - 6)
    if ((sq_mask >> 6) & NOT_AB_FILE) {
        knights_from_positions |= (sq_mask >> 6);
    }
    // 1 verso sinistra e 2 in alto (square + 15)
    if ((sq_mask << 15) & NOT_H_FILE) {
        knights_from_positions |= (sq_mask << 15);
    }
    // 1 verso sinistra e 2 in basso (square - 17)
    if ((sq_mask >> 17) & NOT_H_FILE) {
        knights_from_positions |= (sq_mask >> 17);
    }
    // 1 verso destra e 2 in alto (square + 17)
    if ((sq_mask << 17) & NOT_A_FILE) {
        knights_from_positions |= (sq_mask << 17);
    }
    // 1 verso destra e 2 in basso (square - 15)
    if ((sq_mask >> 15) & NOT_A_FILE) {
        knights_from_positions |= (sq_mask >> 15);
    }

    // Ora se un cavallo avversario è in una di queste posizioni, la casella è sotto attacco
    if (knights_from_positions & attacker_knights) {
        return true;
    }

    return false;
}

/**
 * @brief Funzione di supporto per l'esplorazione "sliding" (rays).
 *
 * Dato un "starting_bit" (un singolo bit a 1, es. la casella "king_pos"),
 * e un "delta" (es. +1, -1, +8, -8, +9, -9, +7, -7), percorriamo la scacchiera
 * in quella direzione finché:
 *  - usciamo dal bordo (wrap-around di riga/colonna o new_pos fuori da 0..63)
 *  - incontriamo un pezzo
 *
 * Restituisce la maschera dei bit "raggiungibili" lungo quel raggio
 * (inclusa l'eventuale casella occupata su cui ci fermiamo).
 *
 * @param[in] starting_bit Un bitmask con un singolo bit acceso.
 * @param[in] delta        Lo spostamento in termini di bit index (es. +8 = una riga su).
 * @param[in] occupancy    Bitboard con 1 su tutte le caselle occupate (bianche o nere).
 * @return Un uint64_t con 1 su tutte le caselle "visibili" in quella direzione
 *         (inclusa l'eventuale prima casella occupata).
 */
uint64_t ray_moves(uint64_t starting_bit, int delta, uint64_t occupancy)
{
    uint64_t result = 0ULL;

    // Troviamo la posizione di partenza (0..63) del bit "starting_bit"
    // (Assumiamo che starting_bit abbia esattamente 1 bit acceso)
#ifdef __GNUC__
    int current_pos = __builtin_ctzll(starting_bit); // Conta zeri a dx (lowest set bit)
#else
    // Implementazione generica se non usi GCC/Clang:
    int current_pos = 0;
    while (current_pos < 64 && !(starting_bit & (1ULL << current_pos))) {
        current_pos++;
    }
    if (current_pos == 64) {
        // Non abbiamo trovato alcun bit? Restituiamo 0
        return 0ULL;
    }
#endif

    // Estraggo riga e colonna attuali (con riga in [0..7], colonna in [0..7])
    int row = current_pos / 8;
    int col = current_pos % 8;

    // Avvio un ciclo per spostarci di "delta" finché è valido
    while (true) {
        // Calcola la nuova posizione
        int new_pos = current_pos + delta;

        // Se esce dai confini [0..63], interrompo
        if (new_pos < 0 || new_pos > 63) {
            break;
        }

        // Calcolo la riga/colonna attesi con la stessa logica: riga = new_pos / 8, col = new_pos % 8
        int new_row = new_pos / 8;
        int new_col = new_pos % 8;

        // Controllo wrap-around di riga o colonna:
        // Se il delta è ±1, stiamo muovendo orizzontalmente => la differenza di row deve rimanere 0
        // Se delta è ±8, stiamo muovendo verticalmente => la differenza di col deve rimanere 0
        // Se delta è ±7, ±9 => diagonali => la differenza di row e col deve essere ±1 in valore assoluto
        //   (Oppure verifichiamo più genericamente che |new_row - row|=1 e |new_col - col|=1)
        // Per una gestione generica, verifichiamo se new_row-row e new_col-col corrispondono
        // a quello che ci aspettiamo da "delta". E' più semplice un check "locale": se colonna
        // salta da 7 a 0 (o viceversa) in un +1, è un wrap. Ecc.

        // Per evitare troppi switch, usiamo un controllo: se la differenza di riga
        // e colonna non è coerente con delta, usciamo. Altrimenti aggiorniamo row/col.
        // Un modo semplice: se la differenza di row è >1 in valore assoluto, break.
        // (Idem colonna). Ma differenziare esattamente i casi evita falsi positivi.

        // --- CONTROLLO WRAP AROUND ORIZZONTALE / VERTICALE / DIAGONALE
        int row_diff = new_row - row;
        int col_diff = new_col - col;
        // Esempi di "delta -> (row_diff, col_diff)":
        //  +1 => (0, +1)
        //  -1 => (0, -1)
        //  +8 => (+1, 0)
        //  -8 => (-1, 0)
        //  +7 => (+1, -1)
        //  -7 => (-1, +1)
        //  +9 => (+1, +1)
        //  -9 => (-1, -1)

        if (delta == +1 && (row_diff != 0 || col_diff != +1)) break;
        if (delta == -1 && (row_diff != 0 || col_diff != -1)) break;
        if (delta == +8 && (row_diff != +1 || col_diff != 0)) break;
        if (delta == -8 && (row_diff != -1 || col_diff != 0)) break;
        if (delta == +7 && (row_diff != +1 || col_diff != -1)) break;
        if (delta == -7 && (row_diff != -1 || col_diff != +1)) break;
        if (delta == +9 && (row_diff != +1 || col_diff != +1)) break;
        if (delta == -9 && (row_diff != -1 || col_diff != -1)) break;

        // Se siamo qui, non c'è wrap-around. Accettiamo la nuova posizione
        current_pos = new_pos;
        row = new_row;
        col = new_col;

        // Costruiamo il bit corrispondente
        uint64_t new_bit = (1ULL << current_pos);

        // Aggiungiamo al "result"
        result |= new_bit;

        // Se la casella è occupata, ci fermiamo (includendo la casella stessa).
        if (new_bit & occupancy) {
            break;
        }
    }

    return result;
}

/**
 * @brief Verifica se la casella `square` è attaccata da un alfiere/vescovo o regina di `attacker_player`.
 *
 * Se un alfiere o una regina (avversari) possono "vedere" `square` sulle diagonali, allora è sotto attacco.
 */
bool is_attacked_by_bishop_or_queen(const bitboard_state_t *state,
                                           int square,
                                           int attacker_player)
{
    // Costruiamo un singolo bit con "square"
    uint64_t sq_mask = 1ULL << square;

    // Bitboard dei pezzi "sliding" diagonali:
    uint64_t attacker_bishops = (attacker_player == 1)
                                ? state->white_bishops
                                : state->black_bishops;
    uint64_t attacker_queens  = (attacker_player == 1)
                                ? state->white_queens
                                : state->black_queens;

    uint64_t occupancy = compute_all_occ(state);

    // Esplora 4 diagonali: +9, -9, +7, -7.
    // Vediamo se, lungo quel raggio, troviamo un alfiere o una regina avversaria.
    // Se "sq_mask" appare in quell'insieme di raggi di un bishop/queen, indica attacco.
    uint64_t diag_up_right = ray_moves(sq_mask, +9, occupancy);  // spostarsi su-destra
    uint64_t diag_up_left  = ray_moves(sq_mask, +7, occupancy);  // spostarsi su-sinistra
    uint64_t diag_dn_right = ray_moves(sq_mask, -7, occupancy);  // giù-destra
    uint64_t diag_dn_left  = ray_moves(sq_mask, -9, occupancy);  // giù-sinistra

    // Ora controlliamo se in una di queste posizioni c'è un alfiere o una regina
    // che "copre" esattamente quell'angolo di attacco.
    // Esempio: se c'è un bit di "attacker_bishops" in diag_up_right => attacco diagonale
    // (lo includiamo in diag_up_right e stiamo a vedere se coincide).
    // Ma attenzione: "diag_up_right" segna TUTTE le celle che si vedono salendo a dx
    // sino al primo pezzo, inclusa la posizione di quel pezzo. Quindi,
    // se quell'ultimo pezzo è un bishop/queen, => scacco.
    if (diag_up_right & (attacker_bishops | attacker_queens)) return true;
    if (diag_up_left  & (attacker_bishops | attacker_queens)) return true;
    if (diag_dn_right & (attacker_bishops | attacker_queens)) return true;
    if (diag_dn_left  & (attacker_bishops | attacker_queens)) return true;

    return false;
}

/**
 * @brief Verifica se la casella `square` è attaccata da una torre (rook) o regina di `attacker_player`.
 *
 * Analogamente a bishop/queen, ma sulle linee verticali/orizzontali.
 */
bool is_attacked_by_rook_or_queen(const bitboard_state_t *state,
                                         int square,
                                         int attacker_player)
{
    // Bit della casella "square"
    uint64_t sq_mask = 1ULL << square;

    // Bitboard dei pezzi rook e queen
    uint64_t attacker_rooks = (attacker_player == 1)
                              ? state->white_rooks
                              : state->black_rooks;
    uint64_t attacker_queens = (attacker_player == 1)
                               ? state->white_queens
                               : state->black_queens;

    uint64_t occupancy = compute_all_occ(state);

    // Esploriamo 4 direzioni: file su (+8), file giù (-8), rank destra (+1), rank sinistra (-1).
    uint64_t up_moves    = ray_moves(sq_mask, +8, occupancy);
    uint64_t down_moves  = ray_moves(sq_mask, -8, occupancy);
    uint64_t right_moves = ray_moves(sq_mask, +1, occupancy);
    uint64_t left_moves  = ray_moves(sq_mask, -1, occupancy);

    // Se su una di queste direzioni troviamo un rook/queen, => attacco
    if (up_moves    & (attacker_rooks | attacker_queens)) return true;
    if (down_moves  & (attacker_rooks | attacker_queens)) return true;
    if (right_moves & (attacker_rooks | attacker_queens)) return true;
    if (left_moves  & (attacker_rooks | attacker_queens)) return true;

    return false;
}

/**
 * @brief Verifica se la casella `square` è attaccata dal re avversario (colore `attacker_player`).
 *
 * Il re avversario può attaccare "square" se "square" è a una distanza di 1 re
 * in orizzontale/verticale/diagonale.
 */
bool is_attacked_by_king(const bitboard_state_t *state, int square, int attacker_player)
{
    uint64_t attacker_king = (attacker_player == 1)
                             ? state->white_kings
                             : state->black_kings;

    if (!attacker_king) return false;

    // Se l'avversario ha un re in posizione "k", vediamo se "k" è in una delle
    // 8 caselle adiacenti a "square".
    // Qui possiamo semplicemente controllare la distanza in file e rank, oppure
    // costruire le maschere di "king move" e verificare se "square" è incluso.
    // Per semplicità, troviamo la posizione "k_pos" e controlliamo la differenza.

#ifdef __GNUC__
    int k_pos = __builtin_ctzll(attacker_king);
#else
    int k_pos = 0;
    while (k_pos < 64 && !(attacker_king & (1ULL << k_pos))) {
        k_pos++;
    }
    if (k_pos == 64) return false;
#endif

    int king_row = k_pos / 8;
    int king_col = k_pos % 8;

    int sq_row   = square / 8;
    int sq_col   = square % 8;

    // Se la differenza assoluta fra row e col non supera 1, => adiacente
    if (abs(king_row - sq_row) <= 1 && abs(king_col - sq_col) <= 1) {
        return true;
    }

    return false;
}


/**
 * @brief Verifica se il re di `player` è sotto scacco nella posizione fornita.
 *
 * @param[in] state   Stato (bitboard) da analizzare.
 * @param[in] player  Giocatore (1 = bianco, -1 = nero) di cui controllare il re.
 * @return true se il re di `player` è sotto scacco, false altrimenti.
 */
bool is_king_in_check(const bitboard_state_t *state, int player)
{
    // 1) Trova la casella su cui si trova il re
    int king_pos = get_king_position(state, player);
    if (king_pos == -1) {
        // Se non troviamo il re, per robustezza restituiamo "non in scacco" 
        // (oppure potresti segnalare un errore).
        return false;
    }

    // 2) L'avversario è -player
    int attacker = -player;

    // 3) Verifica TUTTI i possibili attacchi

    // a) pedoni avversari
    if (is_attacked_by_pawn(state, king_pos, attacker)) {
        return true;
    }

    // b) cavalli avversari
    if (is_attacked_by_knight(state, king_pos, attacker)) {
        return true;
    }

    // c) alfieri o regine avversari (diagonali)
    if (is_attacked_by_bishop_or_queen(state, king_pos, attacker)) {
        return true;
    }

    // d) torri o regine avversari (dritte, orizz/vert)
    if (is_attacked_by_rook_or_queen(state, king_pos, attacker)) {
        return true;
    }

    // e) re avversario
    if (is_attacked_by_king(state, king_pos, attacker)) {
        return true;
    }

    // Se arriviamo qui, nessun attacco trovato => re non in scacco
    return false;
}


/**
 * @brief Verifica se il pezzo che vogliamo muovere è "pinned".
 *
 * Significato di “pinned”:
 *  - Se sposto il pezzo da `move->from`, il Re (del giocatore che muove) rimane (o diventa) in scacco.
 *
 * Implementazione:
 *  - Localizza il Re.
 *  - Crea una copia dello stato in cui il pezzo su `move->from` è rimosso.
 *  - Verifica se il Re risulta in scacco in quella configurazione.
 *
 * @param[in] state Riferimento allo stato attuale (bitboard_state_t).
 * @param[in] move  Mossa che si vuole applicare (chess_move_t).
 * @return `true` se il pezzo è pinned (la mossa è illegale), `false` altrimenti.
 */
bool is_move_pinned(const bitboard_state_t *state, const chess_move_t *move)
{
    // 1) Individua la posizione del Re di chi sta muovendo
    int king_pos = get_king_position(state, state->current_player);
    if (king_pos == -1) {
        // Re non trovato => situazione anomala => assumiamo non pinned
        return false;
    }

    // 2) Crea una copia dello stato in cui “from” è vuoto (il pezzo si è “mosso via”).
    bitboard_state_t *temp = simulate_position_without_piece(state, move);
    if (!temp) {
        // Fallita allocazione => fallback a "non pinned"
        return false;
    }

    // 3) Controlla se in questo stato (senza il pezzo su from) il Re è in scacco
    bool pinned = is_king_in_check(temp, state->current_player);

    // 4) Libera la copia temporanea
    free(temp);

    // 5) Se pinned == true, vuol dire che quel pezzo non può muoversi liberamente
    return pinned;
}


/**
 * @brief Applica una mossa di arrocco al bitboard (sposta Re e Torre).
 *
 * Dettagli:
 *   - Se la mossa è un arrocco corto, sposta il Re di due caselle a destra e la Torre
 *     dal suo angolo fino a fianco del Re.
 *   - Se la mossa è un arrocco lungo, analogo ma sul lato di sinistra.
 *   - Aggiorna i bit di Re e Torre nel bitboard (set/unset).
 *
 * @param[in,out] new_state Stato da modificare (bitboard).
 * @param[in]     move      Mossa di arrocco.
 */
void apply_castling(bitboard_state_t *new_state, const chess_move_t *move)
{
    // Estraggo la casella di partenza (from) e di arrivo (to) del re
    int from = move->from;
    int to   = move->to;

    // Maschera per "from" e "to"
    uint64_t from_mask = 1ULL << from;
    uint64_t to_mask   = 1ULL << to;

    // Per convenzione, verifichiamo se il Re bianco (E1) o Re nero (E8) sta arroccando
    // (Se hai un "current_player" potresti usarlo al posto di scorrere i bitboard.)
    bool is_white_king = (new_state->white_kings & from_mask) != 0ULL;
    bool is_black_king = (new_state->black_kings & from_mask) != 0ULL;

    // Identifichiamo da -> a per la Torre
    // Possibili combinazioni:
    //  - Bianco corto:  E1->G1 => Torre H1->F1
    //  - Bianco lungo:  E1->C1 => Torre A1->D1
    //  - Nero corto:    E8->G8 => Torre H8->F8
    //  - Nero lungo:    E8->C8 => Torre A8->D8

    int rook_from = -1;
    int rook_to   = -1;

    // Se è Bianco...
    if (is_white_king) {
        // Re parte da E1=4
        if (from == 4 && to == 6) {
            // Bianco corto
            rook_from = 7; // H1
            rook_to   = 5; // F1
        }
        else if (from == 4 && to == 2) {
            // Bianco lungo
            rook_from = 0; // A1
            rook_to   = 3; // D1
        }
    }
    // Se è Nero...
    else if (is_black_king) {
        // Re parte da E8=60
        if (from == 60 && to == 62) {
            // Nero corto
            rook_from = 63; // H8
            rook_to   = 61; // F8
        }
        else if (from == 60 && to == 58) {
            // Nero lungo
            rook_from = 56; // A8
            rook_to   = 59; // D8
        }
    }

    // Ora muoviamo il re nel bitboard corrispondente
    // 1) Togliamo il bit "from"
    if (is_white_king) {
        new_state->white_kings &= ~from_mask;
        // Aggiungiamo il bit "to"
        new_state->white_kings |= to_mask;
    }
    else if (is_black_king) {
        new_state->black_kings &= ~from_mask;
        new_state->black_kings |= to_mask;
    }
    else {
        // Casistica anomala: non è un re (o i bitboard non corrispondono).
        // Puoi gestire un errore o return.
        return;
    }

    // Muoviamo la torre
    if (rook_from != -1 && rook_to != -1) {
        uint64_t rook_from_mask = 1ULL << rook_from;
        uint64_t rook_to_mask   = 1ULL << rook_to;

        if (is_white_king) {
            // Rimuovo la torre da "rook_from"
            new_state->white_rooks &= ~rook_from_mask;
            // Aggiungo la torre a "rook_to"
            new_state->white_rooks |= rook_to_mask;
        }
        else {
            // Rimuovo la torre nera
            new_state->black_rooks &= ~rook_from_mask;
            new_state->black_rooks |= rook_to_mask;
        }
    }
    // Se per qualche ragione i valori non sono stati settati correttamente, l'arrocco è anomalo
    // (Potresti gestire un errore / return.)
}


/**
 * @brief Applica la cattura en passant (rimuove il pedone avversario catturato e
 *        sposta il pedone catturante).
 *
 * Dettagli:
 *   - In en passant, il pedone catturato non è sulla casella `to`, ma su `to ± 8` 
 *     (dipende dal colore del pedone che cattura).
 *   - Qui spostiamo anche il pedone catturante da `from` a `to`, così non serve
 *     più chiamare `apply_regular_move`.
 *
 * @param[in,out] new_state Stato da modificare.
 * @param[in]     move      Mossa di en passant (con `is_en_passant == true`).
 */
void apply_en_passant(bitboard_state_t *new_state, const chess_move_t *move)
{
    // Maschere di partenza e arrivo
    uint64_t from_mask = 1ULL << move->from;
    uint64_t to_mask   = 1ULL << move->to;

    // 1) Determina se il pedone catturante è bianco o nero
    bool is_white_pawn = (new_state->white_pawns & from_mask) != 0ULL;
    bool is_black_pawn = (new_state->black_pawns & from_mask) != 0ULL;

    // Se nessuno dei due, c'è un'anomalia
    if (!is_white_pawn && !is_black_pawn) {
        // Caso anomalo: en passant dichiarato, ma su 'from' non c'è un pedone
        return;
    }

    // 2) Spostiamo il pedone catturante sul bitboard (da from -> to)
    //    - Rimuoviamo il bit "from"
    //    - Aggiungiamo il bit "to"
    if (is_white_pawn) {
        new_state->white_pawns &= ~from_mask; // spegni il bit di partenza
        new_state->white_pawns |=  to_mask;   // accendi il bit di arrivo
    } else {
        new_state->black_pawns &= ~from_mask;
        new_state->black_pawns |=  to_mask;
    }

    // 3) Calcoliamo la posizione del pedone avversario catturato
    //    (pedone bianco cattura un nero su "to - 8", oppure pedone nero cattura un bianco su "to + 8")
    int captured_square;
    if (is_white_pawn) {
        captured_square = move->to - 8;  // pedone nero dietro di 8
    } else {
        captured_square = move->to + 8;  // pedone bianco avanti di 8
    }

    uint64_t captured_mask = 1ULL << captured_square;

    // 4) Rimuovi il pedone avversario
    if (is_white_pawn) {
        // Pedone avversario = pedone nero
        new_state->black_pawns &= ~captured_mask;
    } else {
        // Pedone avversario = pedone bianco
        new_state->white_pawns &= ~captured_mask;
    }
}


/**
 * @brief Applica una mossa regolare (non arrocco, non en passant, non promozione).
 *        Gestisce eventuali catture (rimuovendo il pezzo avversario).
 *
 * @param[in,out] new_state Stato da modificare.
 * @param[in]     move      Mossa da applicare.
 * @return \c true se questa mossa ha coinvolto un pedone o è stata una cattura;
 *         \c false in caso di mossa regolare con pezzo diverso dal pedone e senza cattura.
 */
bool apply_regular_move(bitboard_state_t *new_state, const chess_move_t *move)
{
    // Questo flag diventerà true se muoviamo un pedone
    // o se catturiamo un pezzo avversario.
    bool was_pawn_or_capture = false;

    // Maschere per from e to
    uint64_t from_mask = 1ULL << move->from;
    uint64_t to_mask   = 1ULL << move->to;

    // 1) DETERMINA QUALE PEZZO SI STA MUOVENDO

    bool is_white = false;
    uint64_t *piece_bb_to_update = NULL; // Puntatore al bitboard del pezzo in movimento

    // -- Pezzi bianchi --
    if (new_state->white_pawns & from_mask) {
        is_white = true;
        piece_bb_to_update = &new_state->white_pawns;
        was_pawn_or_capture = true;  // è un pedone
    }
    else if (new_state->white_knights & from_mask) {
        is_white = true;
        piece_bb_to_update = &new_state->white_knights;
    }
    else if (new_state->white_bishops & from_mask) {
        is_white = true;
        piece_bb_to_update = &new_state->white_bishops;
    }
    else if (new_state->white_rooks & from_mask) {
        is_white = true;
        piece_bb_to_update = &new_state->white_rooks;
    }
    else if (new_state->white_queens & from_mask) {
        is_white = true;
        piece_bb_to_update = &new_state->white_queens;
    }
    else if (new_state->white_kings & from_mask) {
        is_white = true;
        piece_bb_to_update = &new_state->white_kings;
    }
    // -- Pezzi neri --
    else if (new_state->black_pawns & from_mask) {
        is_white = false;
        piece_bb_to_update = &new_state->black_pawns;
        was_pawn_or_capture = true;  // è un pedone
    }
    else if (new_state->black_knights & from_mask) {
        is_white = false;
        piece_bb_to_update = &new_state->black_knights;
    }
    else if (new_state->black_bishops & from_mask) {
        is_white = false;
        piece_bb_to_update = &new_state->black_bishops;
    }
    else if (new_state->black_rooks & from_mask) {
        is_white = false;
        piece_bb_to_update = &new_state->black_rooks;
    }
    else if (new_state->black_queens & from_mask) {
        is_white = false;
        piece_bb_to_update = &new_state->black_queens;
    }
    else if (new_state->black_kings & from_mask) {
        is_white = false;
        piece_bb_to_update = &new_state->black_kings;
    }
    else {
        // Nessun pezzo trovato su 'from' => situazione anomala
        // Potresti gestirla con un assert o simile; qui
        // ci limitiamo a restituire false (mossa "non riuscita").
        return false;
    }

    // 2) SPEGNE IL BIT NELLA CASELLA DI PARTENZA
    //    (rimuove il pezzo dalla posizione 'from')
    *piece_bb_to_update &= ~from_mask;

    // 3) EVENTUALE CATTURA: RIMUOVI PEZZO AVVERSARIO SU 'to'
    //    Controlla se su 'to' c'è un pezzo avversario e rimuovilo.
    bool captured = false;
    if (is_white) {
        // Rimuovo dai pezzi neri se presente
        if (new_state->black_pawns & to_mask) {
            new_state->black_pawns &= ~to_mask;
            captured = true;
        } else if (new_state->black_knights & to_mask) {
            new_state->black_knights &= ~to_mask;
            captured = true;
        } else if (new_state->black_bishops & to_mask) {
            new_state->black_bishops &= ~to_mask;
            captured = true;
        } else if (new_state->black_rooks & to_mask) {
            new_state->black_rooks &= ~to_mask;
            captured = true;
        } else if (new_state->black_queens & to_mask) {
            new_state->black_queens &= ~to_mask;
            captured = true;
        } else if (new_state->black_kings & to_mask) {
            // In una posizione regolare, il Re nero non dovrebbe mai
            // essere "catturabile" qui, ma per robustezza potresti rimuoverlo
            new_state->black_kings &= ~to_mask;
            captured = true;
        }
    } else {
        // Rimuovo dai pezzi bianchi se presente
        if (new_state->white_pawns & to_mask) {
            new_state->white_pawns &= ~to_mask;
            captured = true;
        } else if (new_state->white_knights & to_mask) {
            new_state->white_knights &= ~to_mask;
            captured = true;
        } else if (new_state->white_bishops & to_mask) {
            new_state->white_bishops &= ~to_mask;
            captured = true;
        } else if (new_state->white_rooks & to_mask) {
            new_state->white_rooks &= ~to_mask;
            captured = true;
        } else if (new_state->white_queens & to_mask) {
            new_state->white_queens &= ~to_mask;
            captured = true;
        } else if (new_state->white_kings & to_mask) {
            // Anche qui, normalmente non si cattura il Re bianco in scacchi regolari
            new_state->white_kings &= ~to_mask;
            captured = true;
        }
    }

    // Se c'è stata cattura, segnalalo
    if (captured) {
        was_pawn_or_capture = true;
    }

    // 4) ACCENDI IL BIT NELLA CASELLA 'to'
    //    (il pezzo che si è mosso va sulla destinazione)
    *piece_bb_to_update |= to_mask;

    // Restituisce true se abbiamo mosso un pedone o catturato un pezzo,
    // false in caso di mossa regolare di altri pezzi senza cattura.
    return was_pawn_or_capture;
}



/**
 * @brief Gestisce la promozione del pedone (sostituisce il pedone con Cavallo/Alfiere/Torre/Regina).
 *
 * @param[in,out] new_state Stato da modificare.
 * @param[in]     move      Mossa di promozione (move->promotion != 0).
 */
void handle_promotion(bitboard_state_t *new_state, const chess_move_t *move)
{
    // 1) Calcoliamo la maschera corrispondente alla casella di arrivo (dove il pedone è approdato)
    uint64_t to_mask = 1ULL << move->to;

    // 2) Verifica se il pedone promosso è bianco o nero,
    //    controllando se la casella "to" appartiene a white_pawns o black_pawns.
    bool is_white_pawn = ((new_state->white_pawns & to_mask) != 0ULL);
    bool is_black_pawn = ((new_state->black_pawns & to_mask) != 0ULL);

    // Se per qualche motivo non è in alcun pedone, c'è un'incoerenza nella posizione
    // (in un motore robusto potresti gestire l'errore, qui ci limitiamo a un check)
    if (!is_white_pawn && !is_black_pawn) {
        // Caso anomalo: la casella di arrivo non contiene un pedone
        // => Niente da fare, oppure potresti gestire come errore.
        return;
    }

    // 3) Rimuovi il pedone dalla casella "to"
    if (is_white_pawn) {
        new_state->white_pawns &= ~to_mask;
    } else {
        new_state->black_pawns &= ~to_mask;
    }

    // 4) In base al valore di move->promotion, accendi il bit sul pezzo corrispondente
    //    (Cavallo=1, Alfiere=2, Torre=3, Regina=4). 
    //    Dividiamo i due rami: White, Black.
    switch (move->promotion) {
        case 1: // Cavallo
            if (is_white_pawn) {
                new_state->white_knights |= to_mask;
            } else {
                new_state->black_knights |= to_mask;
            }
            break;

        case 2: // Alfiere
            if (is_white_pawn) {
                new_state->white_bishops |= to_mask;
            } else {
                new_state->black_bishops |= to_mask;
            }
            break;

        case 3: // Torre
            if (is_white_pawn) {
                new_state->white_rooks |= to_mask;
            } else {
                new_state->black_rooks |= to_mask;
            }
            break;

        case 4: // Regina
        default: 
            // "default" = 4 in caso di valori non standard; 
            // gestiamo come regina per sicurezza
            if (is_white_pawn) {
                new_state->white_queens |= to_mask;
            } else {
                new_state->black_queens |= to_mask;
            }
            break;
    }
}


/**
 * @brief Aggiorna i diritti di arrocco in base alla mossa appena effettuata.
 *
 * Dettagli:
 *   - Se muovo il Re (bianco o nero), perdo entrambi i diritti di arrocco di quel colore.
 *   - Se muovo una torre in un angolo, perdo il diritto corrispondente (lungo o corto).
 *   - Se catturo una torre avversaria in un angolo, l’avversario perde il diritto corrispondente.
 *
 * @param[in,out] new_state Stato da modificare.
 * @param[in]     move      Mossa che potrebbe influire sui diritti di arrocco.
 */
void update_castling_rights(bitboard_state_t *new_state, const chess_move_t *move)
{
    // Per riferimento (convenzione a1=0, b1=1,...h1=7, a8=56,...,h8=63):
    // - White King in E1=4
    // - White Rooks in A1=0, H1=7
    // - Black King in E8=60
    // - Black Rooks in A8=56, H8=63

    // (Adatta i valori se la tua convenzione differisce)

    // 1) Se la mossa sposta il Re (bianco o nero), azzera i diritti per quel colore
    //    Identifichiamo se la casella 'from' contiene un re bianco o nero
    //    (in new_state->white_kings / new_state->black_kings).

    uint64_t from_mask = 1ULL << move->from;
    uint64_t to_mask   = 1ULL << move->to;

    // Esempio di bitmask per i diritti (1,2,4,8). 
    //  White kingside = 0x1
    //  White queenside = 0x2
    //  Black kingside = 0x4
    //  Black queenside = 0x8

    // Salviamo la maschera attuale
    uint8_t rights = new_state->castling_rights;

    // --- 1) SE MUOVO IL RE ---
    bool is_white_king = (new_state->white_kings & from_mask) != 0ULL;
    bool is_black_king = (new_state->black_kings & from_mask) != 0ULL;

    if (is_white_king) {
        // Rimuovo i bit di White kingside e White queenside
        // => (0x1 e 0x2) => 0x3 = b11
        rights &= ~0x3; 
    }
    if (is_black_king) {
        // Rimuovo i bit di Black kingside e Black queenside
        // => (0x4 e 0x8) => 0xC = b1100
        rights &= ~0xC; 
    }

    // --- 2) SE MUOVO UNA TORRE BIANCA O CATTURO UNA TORRE BIANCA ---
    //    A1 = 0 => White queenside
    //    H1 = 7 => White kingside

    // 2a) Mossa di una torre bianca?
    bool is_white_rook_from = ((new_state->white_rooks & from_mask) != 0ULL);

    // 2b) Cattura di una torre bianca? => se 'to_mask' e' su white_rooks => to conteneva una torre bianca
    bool is_white_rook_to   = ((new_state->white_rooks & to_mask) != 0ULL);

    // Idem per torri nere
    bool is_black_rook_from = ((new_state->black_rooks & from_mask) != 0ULL);
    bool is_black_rook_to   = ((new_state->black_rooks & to_mask) != 0ULL);

    // Se muovo una torre, "from" era su un angolo => perdo il diritto corrispondente
    // Oppure se catturo una torre avversaria in un angolo, quell'avversario perde il diritto.

    // --- Torre bianca su A1=0 => rimuove white queenside (0x2)
    if (is_white_rook_from && move->from == 0) {
        rights &= ~0x2;
    }
    // --- Torre bianca su H1=7 => rimuove white kingside (0x1)
    if (is_white_rook_from && move->from == 7) {
        rights &= ~0x1;
    }
    // --- Cattura di una torre bianca su A1=0 => rimuove white queenside
    if (is_black_rook_from && move->to == 0 && is_white_rook_to) {
        rights &= ~0x2;
    }
    // --- Cattura di una torre bianca su H1=7 => rimuove white kingside
    if (is_black_rook_from && move->to == 7 && is_white_rook_to) {
        rights &= ~0x1;
    }

    // --- Torre nera su A8=56 => rimuove black queenside (0x8)
    if (is_black_rook_from && move->from == 56) {
        rights &= ~0x8;
    }
    // --- Torre nera su H8=63 => rimuove black kingside (0x4)
    if (is_black_rook_from && move->from == 63) {
        rights &= ~0x4;
    }
    // --- Cattura di una torre nera su A8=56 => rimuove black queenside
    if (is_white_rook_from && move->to == 56 && is_black_rook_to) {
        rights &= ~0x8;
    }
    // --- Cattura di una torre nera su H8=63 => rimuove black kingside
    if (is_white_rook_from && move->to == 63 && is_black_rook_to) {
        rights &= ~0x4;
    }

    // Aggiorna i diritti nel nuovo stato
    new_state->castling_rights = rights;
}


/**
 * @brief Aggiorna il campo en_passant.
 *
 * Dettagli:
 *   - Se un pedone si muove in avanti di 2 caselle, setta `new_state->en_passant` 
 *     alla casella "intermedia" (indice 0..63).
 *   - Altrimenti, mette `new_state->en_passant` = 255 (disabilitato).
 *
 * @param[in,out] new_state Stato da modificare.
 * @param[in]     move      Mossa che potrebbe impostare/azzerare l'en passant.
 */
void update_en_passant(bitboard_state_t *new_state, const chess_move_t *move)
{
    // 1) Disabilito en_passant di default
    new_state->en_passant = 255;

    // 2) Verifica se è effettivamente un "pedone" che sta muovendo.
    //    Possiamo controllare se la casella 'from' contiene un pedone
    //    (bianco o nero) nel bitboard.
    //    Altrimenti, non è mossa di pedone => en_passant resta 255.
    uint64_t from_mask = 1ULL << move->from;

    bool is_white_pawn = ((new_state->white_pawns & from_mask) != 0ULL);
    bool is_black_pawn = ((new_state->black_pawns & from_mask) != 0ULL);

    if (!is_white_pawn && !is_black_pawn) {
        // Non è un pedone => non modifichiamo en_passant
        return;
    }

    // 3) Calcolo la differenza to - from, per capire se è uno spostamento di 2 caselle.
    int diff = move->to - move->from;

    // Nota: per un bitboard con a1=0, h1=7, a2=8, ..., a8=56,
    //  - Pedone bianco che fa "2 passi" => diff = +16
    //  - Pedone nero che fa "2 passi" => diff = -16

    if (is_white_pawn && diff == 16) {
        // Significa che il pedone bianco si è mosso di due passi.
        // La casella en passant è la "riga 3" (cioè +8 da from).
        int en_passant_square = move->from + 8; 
        new_state->en_passant = (uint8_t)en_passant_square;
    }
    else if (is_black_pawn && diff == -16) {
        // Pedone nero si è mosso di due passi.
        // La casella en passant è la "riga 6" (cioè -8 da from).
        int en_passant_square = move->from - 8;
        new_state->en_passant = (uint8_t)en_passant_square;
    }
    else {
        // Non è una spinta di 2 caselle (o non dai ranghi iniziali):
        // => disabilitiamo en_passant
        new_state->en_passant = 255;
    }
}


/**
 * @brief Aggiorna i contatori di mezze mosse (halfmove_clock) e mosse totali (fullmove_number).
 *
 * Dettagli:
 *   - Se la mossa è una cattura o è un movimento di pedone, azzera halfmove_clock.
 *   - Altrimenti incrementa halfmove_clock.
 *   - Se muove il Nero, incrementa fullmove_number.
 *
 * @param[in,out] new_state                 Stato da modificare.
 * @param[in]     was_capture_or_pawn_move  `true` se la mossa è una cattura o sposta un pedone.
 */

void update_move_counters(bitboard_state_t *new_state, bool was_capture_or_pawn_move)
{
    // Adesso passiamo &stdtrace invece di stdtrace alle macro TRACE_DEBUG

    TRACE_DEBUG(&stdtrace,
                "[update_move_counters] Inizio. was_capture_or_pawn_move=%d, halfmove_clock prima=%d, fullmove_number=%d, current_player=%d",
                was_capture_or_pawn_move,
                new_state->halfmove_clock,
                new_state->fullmove_number,
                new_state->current_player);

    // Se la mossa riguarda un pedone o cattura, azzeriamo
    if (was_capture_or_pawn_move) {
        TRACE_DEBUG(&stdtrace,
                    "[update_move_counters] Resetto halfmove_clock a 0 (mossa di pedone o cattura).");
        new_state->halfmove_clock = 0;
    } 
    else {
        TRACE_DEBUG(&stdtrace,
                    "[update_move_counters] Incremento halfmove_clock di 1 (mossa ordinaria).");
        new_state->halfmove_clock += 1;
    }

    // Se ha appena mosso il Nero, incrementiamo fullmove_number
    if (new_state->current_player == -1) {
        TRACE_DEBUG(&stdtrace,
                    "[update_move_counters] Muoveva il Nero, incremento fullmove_number da %d a %d.",
                    new_state->fullmove_number,
                    new_state->fullmove_number + 1);
        new_state->fullmove_number += 1;
    }

    TRACE_DEBUG(&stdtrace,
                "[update_move_counters] Fine. halfmove_clock=%d, fullmove_number=%d, current_player=%d",
                new_state->halfmove_clock,
                new_state->fullmove_number,
                new_state->current_player);
}



/**
 * @brief Restituisce la lista delle caselle “in mezzo” (escludendo la destinazione del Re)
 *        tra il Re e la Torre in un arrocco specifico (corto/lungo, bianco/nero).
 *
 * @param[in] from Indice di partenza del Re (es. 4 = E1, 60 = E8).
 * @param[in] to   Indice di arrivo del Re (es. 6 = G1, 2 = C1, 62 = G8, 58 = C8).
 * @param[out] squares Array di dimensione sufficiente (max 3) dove salvare
 *                     le caselle “in mezzo”.
 *
 * @return Numero di caselle effettivamente scritte in \c squares (1, 2 o 0 in caso di errore).
 *         - Restituisce 1 per gli arrocchi corti (F1, F8),
 *         - 2 per gli arrocchi lunghi (B1 & D1, B8 & D8),
 *         - 0 se \c (from, to) non corrispondono a un arrocco riconosciuto.
 *
 * @note Non include la casella di destinazione del Re (G1, C1, G8, C8).
 */
int get_castling_squares(int from, int to, int squares[3])
{
    // Inizializza con -1 (sentinella)
    squares[0] = -1;
    squares[1] = -1;
    squares[2] = -1;

    // ============ BIANCO ============

    // Arrocco corto Bianco: E1 (4) -> G1 (6)
    // "in mezzo": F1 (5)
    if (from == 4 && to == 6) {
        squares[0] = 5;  // F1
        return 1;
    }

    // Arrocco lungo Bianco: E1 (4) -> C1 (2)
    // "in mezzo": B1 (1) e D1 (3) – (C1=2 è la destinazione, la escludiamo)
    if (from == 4 && to == 2) {
        squares[0] = 1;  // B1
        squares[1] = 3;  // D1
        return 2;
    }

    // ============ NERO =============

    // Arrocco corto Nero: E8 (60) -> G8 (62)
    // "in mezzo": F8 (61)
    if (from == 60 && to == 62) {
        squares[0] = 61; // F8
        return 1;
    }

    // Arrocco lungo Nero: E8 (60) -> C8 (58)
    // "in mezzo": B8 (57) e D8 (59) – (C8=58 è la destinazione, la escludiamo)
    if (from == 60 && to == 58) {
        squares[0] = 57; // B8
        squares[1] = 59; // D8
        return 2;
    }

    // Se non corrisponde a questi 4 casi, ritorno 0 (arrocco non riconosciuto)
    return 0;
}


/**
 * @brief Verifica se la casella `square` è sotto attacco da parte di qualsiasi pezzo
 *        del giocatore `attacker_player` (1 = Bianco, -1 = Nero).
 *
 * @param[in] state           Stato di gioco (bitboard_state_t).
 * @param[in] square          Indice 0..63 della casella da controllare.
 * @param[in] attacker_player Giocatore (1 = Bianco, -1 = Nero) che potrebbe attaccare.
 * @return true se la casella è sotto attacco, false altrimenti.
 *
 * Dettagli:
 *  - Verifica nell’ordine:
 *    1) Se un pedone dell’avversario può catturare `square`
 *    2) Se un cavallo dell’avversario può raggiungere `square`
 *    3) Se un alfiere o regina avversaria attacca `square` diagonalmente
 *    4) Se una torre o regina avversaria attacca `square` su file/rank
 *    5) Se il re avversario è a una casella di distanza
 *  - Restituisce true al primo attacco trovato.
 *  - Altrimenti false.
 *
 * Esempi d’uso:
 *    if (is_square_attacked(state, 36, -player)) { ... }
 *    // (controllo se la casella 36 è attaccata dall’avversario)
 */
bool is_square_attacked(const bitboard_state_t *state,
                               int square,
                               int attacker_player)
{
    // 1) Pedoni avversari
    if (is_attacked_by_pawn(state, square, attacker_player)) {
        return true;
    }

    // 2) Cavalli avversari
    if (is_attacked_by_knight(state, square, attacker_player)) {
        return true;
    }

    // 3) Alfieri o Regine (diagonali)
    if (is_attacked_by_bishop_or_queen(state, square, attacker_player)) {
        return true;
    }

    // 4) Torri o Regine (verticale/orizzontale)
    if (is_attacked_by_rook_or_queen(state, square, attacker_player)) {
        return true;
    }

    // 5) Re avversario
    if (is_attacked_by_king(state, square, attacker_player)) {
        return true;
    }

    // Nessun attacco trovato => non è sotto tiro
    return false;
}


/**
 * @brief Verifica se l’arrocco indicato da `ch_move` è legale, controllando
 *        che il re non attraversi caselle sotto attacco, che le caselle di
 *        transito siano libere e che il re non sia in scacco nella posizione
 *        di partenza.
 *
 * @param[in] temp_state Stato corrente di gioco (bitboard). Viene passato
 *                       già “copiato” da chess_apply_move.
 * @param[in] ch_move    Mossa di castling da validare (from->to).
 * @return `true` se l’arrocco è legale, `false` altrimenti.
 */
bool is_legal_castle(bitboard_state_t *temp_state,
                            const chess_move_t *ch_move)
{
    // 1) Identifica i parametri chiave
    int from = ch_move->from;  // Re su E1 (4) o E8(60)
    int to   = ch_move->to;    // G1(6), C1(2), G8(62), C8(58)
    int current_player = temp_state->current_player;  // 1=bianco, -1=nero
    int attacker = -current_player;

    // 2) Controllo di base: se re è in scacco, non possiamo arroccare
    //    (in alcune implementazioni si controlla “non era in scacco all’inizio”
    //     e “non attraversi caselle sotto scacco”).
    //    Verifichiamo se nella posizione attuale (temp_state),
    //    il re è già in scacco.
    if (is_king_in_check(temp_state, current_player)) {
        return false;
    }

    // 3) Determina quali caselle il re attraversa
    //    (inclusa la destinazione), a seconda che sia corto o lungo, bianco o nero
    int squares[3] = { -1, -1, -1 };  // al max 3 caselle
    int n_squares = get_castling_squares(from, to, squares);
    if (n_squares == 0) {
        // Significa che from->to non corrisponde a un arrocco standard
        return false;
    }

    // 4) Verifica che TUTTE queste caselle intermedie/destinazione siano libere
    //    (tranne la casella “to” se decidi che in quell’indice c’è Re? In genere,
    //     la destinazione dev’essere libera comunque prima di spostarci).
    //
    // NON SERVE. Controllato durante la generazione delle mosse.

    // 5) Verifica che TUTTE queste caselle non siano sotto attacco dall’avversario
    //    (il re non può attraversare o finire su caselle controllate).
    for (int i = 0; i < n_squares; i++) {
        int sq = squares[i];
        if (is_square_attacked(temp_state, sq, attacker)) {
            return false;
        }
    }

    // Se tutti i test superati => arrocco legale
    return true;
}


/******************************************************************************
 *                      FUNZIONE PRINCIPALI
 *****************************************************************************/

/**
 * @brief Applica una pseudo-mossa allo stato di gioco corrente utilizzando Bitboards.
 *
 * Viene scartata se risulta illegale (pinned, cattura del Re avversario,
 * lascia/crea scacco al proprio Re).
 *
 * @param[in] state Puntatore costante a uno `bitboard_state_t`.
 * @param[in] move  Puntatore costante a `chess_move_t`.
 * @return Un nuovo stato di gioco (`bitboard_state_t*`) se la mossa è valida,
 *         oppure `NULL` se la mossa è illegale o se `state` / `move` sono NULL.
 *
 * @note Il chiamante deve liberare la memoria del nuovo stato (con `chess_free_state`)
 *       quando non serve più.
 */
void* chess_apply_move(const void *state, const void *move)
{
    if (!state || !move) {
        return NULL; // Parametri non validi
    }

    // Cast dei parametri ai tipi concreti
    const bitboard_state_t *old_state = (const bitboard_state_t*)state;
    const chess_move_t     *ch_move   = (const chess_move_t*)move;

    // 1) Controllo se il pezzo da muovere è pinned: se sì, mossa illegale.
    if (is_move_pinned(old_state, ch_move)) {
        return NULL;
    }

    // 2) Controllo se la mossa cattura il Re avversario (caso pseudo-mossa che va sul re)
    {
        uint64_t opp_king_bb = (old_state->current_player == 1)
                               ? old_state->black_kings
                               : old_state->white_kings;
        uint64_t to_mask = 1ULL << ch_move->to;
        if (opp_king_bb & to_mask) {
            // Mossa che cattura il re avversario: illegale.
            return NULL;
        }
    }

    // 3) Alloco un nuovo stato e ne copio il contenuto
    bitboard_state_t *new_state = (bitboard_state_t*)malloc(sizeof(bitboard_state_t));
    if (!new_state) {
        return NULL; // Fallimento allocazione
    }
    *new_state = *old_state; // Copia superficiale (bit a bit)

    // 4) Applicazione della mossa
    bool was_capture_or_pawn_move = false;

    if (ch_move->is_castling) {
        // A) Eseguiamo i controlli specifici per l’arrocco
        if (!is_legal_castle(new_state, ch_move)) {
            // Se la funzione di validazione fallisce,
            // liberiamo lo stato e ritorniamo NULL.
            free(new_state);
            return NULL;
        }
        // B) Se la validazione ha successo, spostiamo Re e Torre
        apply_castling(new_state, ch_move);
    }
    else if (ch_move->is_en_passant) {
        apply_en_passant(new_state, ch_move);
        was_capture_or_pawn_move = true;
    }
    else {
        // Promozione?
        if (ch_move->promotion != 0) {
            apply_regular_move(new_state, ch_move);
            handle_promotion(new_state, ch_move);
            was_capture_or_pawn_move = true;  // è un pedone
        } else {
            // Mossa regolare
            was_capture_or_pawn_move = apply_regular_move(new_state, ch_move); // PATCH: per gestire halfmove in apply regular move
        }
    }

    // 5) Aggiorno diritti di arrocco, en_passant e contatori di mosse
    update_castling_rights(new_state, ch_move);
    update_en_passant(new_state, ch_move);
    update_move_counters(new_state, was_capture_or_pawn_move);

    // 6) Cambio giocatore
    new_state->current_player = -(old_state->current_player);

    // 7) Verifico se il Re del “vecchio giocatore” (chi ha mosso) è in scacco dopo la mossa
    if (is_king_in_check(new_state, -(new_state->current_player))) {
        free(new_state);
        return NULL;
    }

    // 8) Se tutto ok, restituisco il nuovo stato
    return new_state;
}


/**
 * @brief Verifica se uno stato di gioco è terminale (scacco matto, stallo, patta, ecc.).
 *
 * @param[in] state Stato di gioco da verificare (rappresentato da `bitboard_state_t`).
 * @return 1 se lo stato è terminale, 0 altrimenti.
 */
int chess_is_terminal(const void *state)
{
    if (!state) {
        // Stato non valido => potresti gestire come terminale o errore;
        // qui scegliamo di restituire 1 (stato "non valido") oppure 0.
        return 1;  // assumiamo terminale per sicurezza
    }

    // Cast del parametro generico allo stato bitboard
    const bitboard_state_t *bit_state = (const bitboard_state_t*)state;

    // 1) Genera TUTTE le pseudo-mosse (incluso cattura Re avversario, en passant, ecc.)
    dynamic_vector_t *moves = chess_get_moves(bit_state);
    if (!moves) {
        // Se per qualche ragione non riusciamo a generare le mosse,
        // potremmo considerare lo stato come terminale o generare errore.
        return 1;
    }

    int num_moves = chess_get_num_moves(moves);
    if (num_moves == 0) {
        // Non ci sono pseudo-mosse nemmeno prima del filtraggio:
        // Se il Re è in scacco => scacco matto, se non in scacco => stallo.
        chess_free_moves(moves);

        return 1; // Stato terminale

        // Controlla se il re del giocatore corrente è in scacco
        // (chi deve muovere si trova in scacco ma non ha mosse => checkmate)
        //if (is_king_in_check(bit_state, bit_state->current_player)) {
        //    return 1;  // scacco matto => terminale
        //} else {
        //    return 1;  // stallo => terminale
        //}
    };

    // 2) FASE DI FILTRAGGIO: controlliamo per ciascuna pseudo-mossa se è effettivamente legale.
    //    Se troviamo almeno una mossa LEGITTIMA, lo stato non è terminale => 0.

    // Strategia:
    //  - Per ogni pseudo-mossa, applichiamo "chess_apply_move" su una copia dello stato
    //    (se la mossa è pinned, cattura re avversario, o lascia in scacco, "chess_apply_move" 
    //    dovrebbe restituire NULL).
    //  - Se chess_apply_move restituisce un nuovo stato NON NULL => c'è una mossa legale => non terminale.

    for (int i = 0; i < num_moves; i++) {
        const chess_move_t *mv = (const chess_move_t*)chess_get_move_at(moves, i);
        if (!mv) continue;  // per sicurezza

        // Proviamo ad applicare la mossa: se è illegale, tornerà NULL
        void *new_state = chess_apply_move(bit_state, mv);
        if (new_state != NULL) {
            // sign. la mossa è legale => esiste almeno una mossa valida => non terminale
            chess_free_state(new_state);
            chess_free_moves(moves);
            return 0;  // esiste una mossa => stato NON terminale
        }
    }

    // 3) Se nessuna pseudo-mossa è sopravvissuta (cioè tutte illegali),
    //    allora siamo in stallo o scacco matto:
    //    - Se il re è in scacco => scacco matto => terminale
    //    - Altrimenti stallo => terminale

    chess_free_moves(moves);
    return 1;

    //if (is_king_in_check(bit_state, bit_state->current_player)) {
    //    // scacco matto
    //    return 1;
    //} else {
    //    // stallo
    //    return 1;
    //}
}


/**
 * @brief Determina quale giocatore deve muovere.
 *
 * @param[in] state Stato corrente del gioco (rappresentato da `bitboard_state_t`).
 * @return Identificatore del giocatore da muovere (1 = Bianco, -1 = Nero).
 */
int chess_player_to_move(const void *state)
{
    if (!state) {
        // Se lo stato è NULL, puoi decidere come gestire l'errore:
        // ad esempio, restituiamo 0 come "non definito"
        return 0;
    }

    // Cast del puntatore generico al tipo concreto
    const bitboard_state_t *bit_state = (const bitboard_state_t*)state;

    // Restituisce il valore di `current_player`, che è 1 (Bianco) o -1 (Nero).
    return bit_state->current_player;
}


/**
 * @brief Valuta uno stato di gioco.
 *
 * @param[in] state Stato di gioco da valutare (rappresentato da `bitboard_state_t`).
 * @return Valore euristico dello stato (positivo per vantaggio dei bianchi, negativo per vantaggio dei neri).
 */
int chess_evaluate(const void *state)
{
    if (!state) {
        // Per sicurezza, stato nullo => valutazione neutra (o errore).
        return 0;
    }

    const bitboard_state_t *bs = (const bitboard_state_t*)state;

    // 1) SE LO STATO È TERMINALE, potremmo avere un punteggio “±∞”
    //    (oppure un valore alto/ basso a scelta). Esempio:
    if (chess_is_terminal(bs)) {
        // Se lo stato è terminale, e se è scacco matto per Bianco => grande punteggio positivo.
        // Se è scacco matto per Nero => grande punteggio negativo.
        // Se è stallo/patta => 0.
        // Per semplicità ipotizziamo:
        // - Se il re del “current_player” è in scacco e non ci sono mosse => matto contro di lui.
        // - Altrimenti stallo/patta => 0.

        // Un modo è: se current_player ha 0 mosse legali e re in scacco => matto a suo sfavore.
        // Lo verifichiamo in base a scacco e current_player, oppure
        // più semplicemente diamo un punteggio:
        // +99999 se Bianco ha matto,
        // -99999 se Nero ha matto,
        // 0 se patta.

        // (Questo è un esempio "rapido": in un vero motore potresti avere una routine
        //  che analizza chi è sotto scacco, ecc.)

        // Per brevità, assegniamo:
        // se bs->current_player == 1 => Bianco muove
        //   e se matto => punteggio MOLTO negativo (nero ha dato matto)
        // se bs->current_player == -1 => Nero muove
        //   e se matto => punteggio MOLTO positivo (bianco ha dato matto)
        // se stallo => 0

        // Qui semplifichiamo: se “is_terminal = 1” e re Bianco è in scacco => punteggio -99999
        //                                     se re Nero è in scacco => punteggio +99999
        //                                     altrimenti patta => 0
        // (La logica reale andrebbe raffinata.)
        bool white_check = is_king_in_check(bs, 1);
        bool black_check = is_king_in_check(bs, -1);

        // se white_check == true => Re bianco in scacco => matto a Bianco => punteggio -∞
        // se black_check == true => Re nero in scacco => matto a Nero => punteggio +∞
        // se nessuno in scacco => stallo => 0
        if (white_check) return -99999; 
        if (black_check) return +99999;
        return 0;  // patta / stallo
    }

    // 2) VALUTAZIONE MATERIALE SEMPLICE
    //    Conta i pezzi bianchi e neri, assegna un punteggio base per ciascun tipo.
    //    Esempio di valori:
    //    - pedone   = 100
    //    - cavallo  = 320
    //    - alfiere  = 330 (oppure 320, dipende dalle preferenze)
    //    - torre    = 500
    //    - regina   = 900
    //    - re       = 20000 (opzionale, di solito non si conta il re,
    //      ma puoi dare un valore enorme per impedire cattura del re in stime).
    //    Qui omettiamo il re o gli diamo un valore altissimo per
    //    completare la differenza.

    // 2a) Conta i bit a 1 in ciascun bitboard con una popcount
    //     (oppure un loop). Qui assumiamo di avere __builtin_popcountll per semplicità.
    //     Se il tuo compilatore non la supporta, puoi implementare una popcount a mano.

    int white_pawns   = __builtin_popcountll(bs->white_pawns);
    int white_knights = __builtin_popcountll(bs->white_knights);
    int white_bishops = __builtin_popcountll(bs->white_bishops);
    int white_rooks   = __builtin_popcountll(bs->white_rooks);
    int white_queens  = __builtin_popcountll(bs->white_queens);
    // int white_kings   = __builtin_popcountll(bs->white_kings); // di solito 1

    int black_pawns   = __builtin_popcountll(bs->black_pawns);
    int black_knights = __builtin_popcountll(bs->black_knights);
    int black_bishops = __builtin_popcountll(bs->black_bishops);
    int black_rooks   = __builtin_popcountll(bs->black_rooks);
    int black_queens  = __builtin_popcountll(bs->black_queens);
    // int black_kings   = __builtin_popcountll(bs->black_kings); // di solito 1

    // 2b) Calcoliamo il punteggio (material) = (materia bianca - materia nera)
    //    con i valori di base
    int wscore = 0;
    wscore += white_pawns   * 100;
    wscore += white_knights * 320;
    wscore += white_bishops * 330;
    wscore += white_rooks   * 500;
    wscore += white_queens  * 900;
    // wscore += white_kings   * 20000; // se vuoi contare pure il re

    int bscore = 0;
    bscore += black_pawns   * 100;
    bscore += black_knights * 320;
    bscore += black_bishops * 330;
    bscore += black_rooks   * 500;
    bscore += black_queens  * 900;
    // bscore += black_kings   * 20000;

    int material_score = wscore - bscore;

    // 3) BONUS / MALUS SEMPLICI (opzionali)
    //    Ad esempio, +30 se c'è un "bishop pair" (due alfieri) per un colore.
    //    Se vuoi, puoi farlo:
    if (white_bishops >= 2) material_score += 30; 
    if (black_bishops >= 2) material_score -= 30;

    // 4) ALTRE EURISTICHE (centro, sviluppo, etc.) ... (omesse in questa versione)

    // 5) RESTITUISCI IL RISULTATO
    //    Valore positivo => vantaggio Bianco
    //    Valore negativo => vantaggio Nero
    //    (Puoi aggiungere un piccolissimo bonus a "bs->current_player" se vuoi incentivare la mossa del giocatore di turno)
    return material_score;
}

