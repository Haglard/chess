// chess to gui interface

#ifndef CHESS_TO_GUI_INTERFACE_H
#define CHESS_TO_GUI_INTERFACE_H

#include <SDL2/SDL.h>
#include <stdbool.h>


/**
 * @enum ChessGuiPieces_t
 * @brief Elenco di tutti i pezzi di scacchi (GUI), incluso uno "vuoto".
 */
typedef enum ChessGuiPieces_e {
    CHESS_GUI_EMPTY = 0,

    CHESS_GUI_PAWN_WHITE,
    CHESS_GUI_ROOK_WHITE,
    CHESS_GUI_KNIGHT_WHITE,
    CHESS_GUI_BISHOP_WHITE,
    CHESS_GUI_QUEEN_WHITE,
    CHESS_GUI_KING_WHITE,
    CHESS_GUI_PAWN_BLACK,
    CHESS_GUI_ROOK_BLACK,
    CHESS_GUI_KNIGHT_BLACK,
    CHESS_GUI_BISHOP_BLACK,
    CHESS_GUI_QUEEN_BLACK,
    CHESS_GUI_KING_BLACK,

    CHESS_GUI_PIECE_COUNT  
} ChessGuiPieces_t;

// ----------------------------------------------------------------------
// Nuova parte: dead pieces (2×N = [colore][slot])
// ----------------------------------------------------------------------

/** @brief Costanti per identificare il colore/giocatore. */
#define GUI_WHITE_PLAYER 0
#define GUI_BLACK_PLAYER 1

/** @brief Numero massimo di “dead pieces” tracciabili per ogni colore. */
#define GUI_MAX_DEAD_PIECES 16

/**
 * @brief Numero massimo di mosse registrabili nella storia
 */
#define MAX_HISTORY_MOVES 999

/**
 * @struct move_record_t
 * @brief Rappresenta una singola mossa nella storia
 */
typedef struct move_record_s {
    int move_id;               /**< Indice 1-based (se 0 => verrà impostato automaticamente). */

    ChessGuiPieces_t moved_piece;   /**< Pezzo mosso (es. CHESS_GUI_PAWN_WHITE). */
    int player_who_moved;           /**< CHESS_GUI_WHITE o CHESS_GUI_BLACK. */

    int from_c;  /**< Colonna di partenza [0..7]. */
    int from_r;  /**< Riga di partenza    [0..7]. */
    int to_c;    /**< Colonna di arrivo   [0..7]. */
    int to_r;    /**< Riga di arrivo      [0..7]. */

    bool castling_right;     
    bool castling_left;      
    bool en_passant;         
    bool opponent_under_check; 
    bool checkmate;          
    bool draw;               

    ChessGuiPieces_t captured_piece; /**< Se != CHESS_GUI_EMPTY, pezzo catturato. */
    int board_status_evaluation;      /**< Valutazione board (intero). */
} move_record_t;


/**
 * @struct game_history_log_t
 * @brief Struttura che contiene fino a 999 mosse e l'indice dell'ultima.
 */
typedef struct game_history_log_s {
    int last_move;  /**< -1 se vuota, altrimenti indice 0..(n-1). */
    move_record_t records[MAX_HISTORY_MOVES];
} game_history_log_t;


/**
 * @brief Inizializza la storia delle mosse (last_move = -1, azzera records).
 */
void game_history_init(void);


/**
 * @brief Restituisce il numero di mosse attualmente in storia (0 se vuota).
 */
int game_history_length(void);


/**
 * @brief Aggiunge una nuova mossa in coda.
 *
 * Se rec->move_id == 0, viene impostato a (last_move+2) -> 1-based.
 *
 * @param rec Puntatore a move_record_t da inserire
 * @return true se inserita, false se array pieno
 */
bool game_history_add_move(const move_record_t* rec);


/**
 * @brief Legge la mossa in posizione index (0..length-1).
 * @param index indice 0-based
 * @param outRecord puntatore dove copiare la mossa
 * @return true se esiste, false altrimenti
 */
bool game_history_get_move(int index, move_record_t* outRecord);


/**
 * @typedef ChessGuiBoard_t
 * @brief Matrice 8×8 di ChessGuiPieces_t.
 */
typedef ChessGuiPieces_t ChessGuiBoard_t[8][8];

/**
 * @brief Inizializza la scacchiera \p board con la disposizione standard (pedoni, re, ecc.).
 */
void ChessGuiBoard_InitStandardArrangement(ChessGuiBoard_t board);


/**
 * @struct ChessGuiDeadPieces_t
 * @brief Struttura che contiene i pezzi "morti" per i due colori (white, black).
 *
 * Matrice 2×N: 
 * - dimensione 0 => bianchi
 * - dimensione 1 => neri
 * - dimensione orizzontale => slot (fino a 16).
 */
typedef struct ChessGuiDeadPieces_s {
    ChessGuiPieces_t dead[2][GUI_MAX_DEAD_PIECES];
} ChessGuiDeadPieces_t;


/**
 * @brief Inizializza tutti i dead pieces a CHESS_GUI_EMPTY.
 *
 * @param dp Struttura da inizializzare.
 */
void ChessGuiDeadPieces_InitEmpty(ChessGuiDeadPieces_t *dp);


/**
 * @brief Inizializza la struttura dei dead pieces come se TUTTI i pezzi 
 *        fossero stati catturati (16 per i bianchi, 16 per i neri).
 *
 * @param dp Puntatore alla struttura ChessGuiDeadPieces_t da riempire.
 */
void initDeadPiecesAllCaptured(ChessGuiDeadPieces_t *dp);


/**
 * @brief Ritorna il pezzo "morto" in posizione \p pos per il colore \p color.
 *
 * @param dp     Struttura ChessGuiDeadPieces_t.
 * @param color  \c GUI_WHITE_PLAYER o \c GUI_BLACK_PLAYER.
 * @param pos    Indice dello slot [0..GUI_MAX_DEAD_PIECES-1].
 * @return       Il pezzo in dp->dead[color][pos].
 */
ChessGuiPieces_t ChessGuiDeadPieces_GetPiece(const ChessGuiDeadPieces_t *dp, int color, int pos);

/**
 * @brief Imposta in \p dp->dead[color][pos] il pezzo \p piece.
 *
 * @param dp     Struttura dei dead pieces.
 * @param color  \c GUI_WHITE_PLAYER o \c GUI_BLACK_PLAYER.
 * @param pos    Indice slot [0..GUI_MAX_DEAD_PIECES-1].
 * @param piece  Un \c ChessGuiPieces_t (es. CHESS_GUI_PAWN_WHITE) o CHESS_GUI_EMPTY per "vuoto".
 */
void ChessGuiDeadPieces_SetPiece(ChessGuiDeadPieces_t *dp, int color, int pos, ChessGuiPieces_t piece);


/**
 * @struct ChessGameEngine
 * @brief Raccoglie i riferimenti necessari al motore di gioco scacchistico,
 *        inclusi lo stato, il game_descriptor e la cache per il minimax.
 */
typedef struct ChessGameEngine_s {
    bitboard_state_t* current_state;      /**< Stato corrente (bitboard). */
    game_descriptor_t* gd;               /**< Descrittore di gioco scacchistico. */
    generic_hash_table_t* my_cache;      /**< Cache per memorizzare gli stati già analizzati. */
    int depth;                           /**< Profondità di ricerca per minimax. */
} ChessGameEngine;

/**
 * @brief Inizializza il motore di gioco scacchistico (bitboard, game_descriptor, cache, depth).
 *
 * Esegue i passi:
 * 1. Alloca e inizializza current_state (board standard).
 * 2. Inizializza il descrittore di gioco (con callback minimax).
 * 3. Crea la cache (hash table) per il minimax.
 * 4. Imposta la profondità di ricerca a 5.
 *
 * @return Un puntatore a ChessGameEngine se tutto ok, NULL se errore.
 */
ChessGameEngine* ChessGui_init_game_engine(void);

/**
 * @brief Rilascia le risorse del motore di gioco scacchistico.
 *
 * @param engine Puntatore al ChessGameEngine creato da ChessGui_init_game_engine.
 */
void ChessGui_free_game_engine(ChessGameEngine* engine);


#endif /* CHESS_TO_GUI_INTERFACE_H */
