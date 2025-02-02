// chess_state.h
#ifndef CHESS_STATE_H
#define CHESS_STATE_H

#include <stdint.h>
#include <stdlib.h>

/**
 * @brief Struttura che rappresenta lo stato del gioco degli scacchi utilizzando Bitboards.
 *
 * Ogni Bitboard a 64 bit rappresenta la presenza di un tipo specifico di pezzo
 * per un giocatore (bianco o nero) su ciascuna delle 64 caselle della scacchiera.
 *
 * La disposizione dei pezzi è la seguente:
 * - Pedoni (P)
 * - Cavalli (N)
 * - Alfieri (B)
 * - Torri (R)
 * - Regine (Q)
 * - Re (K)
 *
 * I Bitboards sono organizzati separatamente per i pezzi bianchi e neri.
 * Ad esempio, `white_pawns` rappresenta la posizione di tutti i pedoni bianchi.
 */
typedef struct {
    // Bitboards per i pezzi bianchi
    uint64_t white_pawns;   /**< Pedoni bianchi */
    uint64_t white_knights; /**< Cavalli bianchi */
    uint64_t white_bishops; /**< Alfieri bianchi */
    uint64_t white_rooks;   /**< Torri bianche */
    uint64_t white_queens;  /**< Regine bianche */
    uint64_t white_kings;   /**< Re bianchi */

    // Bitboards per i pezzi neri
    uint64_t black_pawns;   /**< Pedoni neri */
    uint64_t black_knights; /**< Cavalli neri */
    uint64_t black_bishops; /**< Alfieri neri */
    uint64_t black_rooks;   /**< Torri nere */
    uint64_t black_queens;  /**< Regine nere */
    uint64_t black_kings;   /**< Re neri */

    // Informazioni aggiuntive
    uint8_t castling_rights; /**< Diritti di arrocco (bitmask) */
    uint8_t en_passant;      /**< Casella per en passant (0-63, 255 se non disponibile) */
    uint8_t halfmove_clock;  /**< Contatore dei mezzi turni per la regola delle 50 mosse */
    uint8_t fullmove_number; /**< Numero del turno (inizia da 1) */
    int current_player;      /**< Giocatore corrente (1 = bianco, -1 = nero) */
} bitboard_state_t;


/**
 * @brief Maschera che azzera i bit della colonna 'a' (evita wrap su file A).
 */
#define NOT_A_FILE  0xfefefefefefefefeULL

/**
 * @brief Maschera che azzera i bit della colonna 'h' (evita wrap su file H).
 */
#define NOT_H_FILE  0x7f7f7f7f7f7f7f7fULL

/**
 * @brief Maschera che azzera i bit delle colonne G e H (evita wrap per spostamenti di cavalli).
 */
#define NOT_GH_FILE 0x3f3f3f3f3f3f3f3fULL

/**
 * @brief Crea una copia profonda dello stato del gioco.
 *
 * @param[in] state Puntatore allo stato del gioco originale.
 * @return Un puntatore a un nuovo stato del gioco, allocato dinamicamente, oppure NULL in caso di errore.
 */
void* chess_copy_state(const void *state);

/**
 * @brief Libera la memoria allocata per uno stato del gioco.
 *
 * @param[in] state Puntatore allo stato del gioco da liberare.
 */
void chess_free_state(void *state);

/**
 * @brief Inizializza la scacchiera con la posizione di partenza standard.
 *
 * @param[out] state Puntatore allo stato del gioco da inizializzare.
 */
void initialize_board(bitboard_state_t *state);

/**
 * @brief Stampa lo stato della scacchiera in formato testuale (opzionale, per debug).
 *
 * @param[in] state Puntatore allo stato del gioco da stampare.
 */
void print_board(const bitboard_state_t *state);

#endif /* CHESS_STATE_H */

