#include <string.h>
#include <stdio.h>

#include "chess_to_gui_interface.h"

/**
 * @brief Inizializza la scacchiera 8×8 con la disposizione classica dei pezzi bianchi e neri.
 *
 * Riga 0 e 1: pezzi neri,
 * Riga 6 e 7: pezzi bianchi,
 * Righe centrali vuote (CHESS_GUI_EMPTY).
 */
void ChessGuiBoard_InitStandardArrangement(ChessGuiBoard_t board)
{
    // Riga 0 (in alto): TORRE, CAVALLO, ALFIERE, REGINA, RE, ALFIERE, CAVALLO, TORRE (neri)
    board[0][0] = CHESS_GUI_ROOK_BLACK;
    board[0][1] = CHESS_GUI_KNIGHT_BLACK;
    board[0][2] = CHESS_GUI_BISHOP_BLACK;
    board[0][3] = CHESS_GUI_QUEEN_BLACK;
    board[0][4] = CHESS_GUI_KING_BLACK;
    board[0][5] = CHESS_GUI_BISHOP_BLACK;
    board[0][6] = CHESS_GUI_KNIGHT_BLACK;
    board[0][7] = CHESS_GUI_ROOK_BLACK;

    // Riga 1: pedoni neri
    for (int c = 0; c < 8; c++) {
        board[1][c] = CHESS_GUI_PAWN_BLACK;
    }

    // Righe 2..5 vuote
    for (int r = 2; r < 6; r++) {
        for (int c = 0; c < 8; c++) {
            board[r][c] = CHESS_GUI_EMPTY;
        }
    }

    // Riga 6: pedoni bianchi
    for (int c = 0; c < 8; c++) {
        board[6][c] = CHESS_GUI_PAWN_WHITE;
    }

    // Riga 7 (in basso): TORRE, CAVALLO, ALFIERE, REGINA, RE, ALFIERE, CAVALLO, TORRE (bianchi)
    board[7][0] = CHESS_GUI_ROOK_WHITE;
    board[7][1] = CHESS_GUI_KNIGHT_WHITE;
    board[7][2] = CHESS_GUI_BISHOP_WHITE;
    board[7][3] = CHESS_GUI_QUEEN_WHITE;
    board[7][4] = CHESS_GUI_KING_WHITE;
    board[7][5] = CHESS_GUI_BISHOP_WHITE;
    board[7][6] = CHESS_GUI_KNIGHT_WHITE;
    board[7][7] = CHESS_GUI_ROOK_WHITE;
}


void ChessGuiDeadPieces_InitEmpty(ChessGuiDeadPieces_t *dp) {
    for (int color = 0; color < 2; color++) {
        for (int i = 0; i < GUI_MAX_DEAD_PIECES; i++) {
            dp->dead[color][i] = CHESS_GUI_EMPTY;
        }
    }
}

void initDeadPiecesAllCaptured(ChessGuiDeadPieces_t *dp)
{
    // Bianchi: 8 pedoni, 2 torri, 2 cavalli, 2 alfieri, 1 regina, 1 re
    dp->dead[GUI_WHITE_PLAYER][0]  = CHESS_GUI_PAWN_WHITE;
    dp->dead[GUI_WHITE_PLAYER][1]  = CHESS_GUI_PAWN_WHITE;
    dp->dead[GUI_WHITE_PLAYER][2]  = CHESS_GUI_PAWN_WHITE;
    dp->dead[GUI_WHITE_PLAYER][3]  = CHESS_GUI_PAWN_WHITE;
    dp->dead[GUI_WHITE_PLAYER][4]  = CHESS_GUI_PAWN_WHITE;
    dp->dead[GUI_WHITE_PLAYER][5]  = CHESS_GUI_PAWN_WHITE;
    dp->dead[GUI_WHITE_PLAYER][6]  = CHESS_GUI_PAWN_WHITE;
    dp->dead[GUI_WHITE_PLAYER][7]  = CHESS_GUI_PAWN_WHITE;
    dp->dead[GUI_WHITE_PLAYER][8]  = CHESS_GUI_ROOK_WHITE;
    dp->dead[GUI_WHITE_PLAYER][9]  = CHESS_GUI_ROOK_WHITE;
    dp->dead[GUI_WHITE_PLAYER][10] = CHESS_GUI_KNIGHT_WHITE;
    dp->dead[GUI_WHITE_PLAYER][11] = CHESS_GUI_KNIGHT_WHITE;
    dp->dead[GUI_WHITE_PLAYER][12] = CHESS_GUI_BISHOP_WHITE;
    dp->dead[GUI_WHITE_PLAYER][13] = CHESS_GUI_BISHOP_WHITE;
    dp->dead[GUI_WHITE_PLAYER][14] = CHESS_GUI_QUEEN_WHITE;
    dp->dead[GUI_WHITE_PLAYER][15] = CHESS_GUI_KING_WHITE;

    // Neri: 8 pedoni, 2 torri, 2 cavalli, 2 alfieri, 1 regina, 1 re
    dp->dead[GUI_BLACK_PLAYER][0]  = CHESS_GUI_PAWN_BLACK;
    dp->dead[GUI_BLACK_PLAYER][1]  = CHESS_GUI_PAWN_BLACK;
    dp->dead[GUI_BLACK_PLAYER][2]  = CHESS_GUI_PAWN_BLACK;
    dp->dead[GUI_BLACK_PLAYER][3]  = CHESS_GUI_PAWN_BLACK;
    dp->dead[GUI_BLACK_PLAYER][4]  = CHESS_GUI_PAWN_BLACK;
    dp->dead[GUI_BLACK_PLAYER][5]  = CHESS_GUI_PAWN_BLACK;
    dp->dead[GUI_BLACK_PLAYER][6]  = CHESS_GUI_PAWN_BLACK;
    dp->dead[GUI_BLACK_PLAYER][7]  = CHESS_GUI_PAWN_BLACK;
    dp->dead[GUI_BLACK_PLAYER][8]  = CHESS_GUI_ROOK_BLACK;
    dp->dead[GUI_BLACK_PLAYER][9]  = CHESS_GUI_ROOK_BLACK;
    dp->dead[GUI_BLACK_PLAYER][10] = CHESS_GUI_KNIGHT_BLACK;
    dp->dead[GUI_BLACK_PLAYER][11] = CHESS_GUI_KNIGHT_BLACK;
    dp->dead[GUI_BLACK_PLAYER][12] = CHESS_GUI_BISHOP_BLACK;
    dp->dead[GUI_BLACK_PLAYER][13] = CHESS_GUI_BISHOP_BLACK;
    dp->dead[GUI_BLACK_PLAYER][14] = CHESS_GUI_QUEEN_BLACK;
    dp->dead[GUI_BLACK_PLAYER][15] = CHESS_GUI_KING_BLACK;
}

ChessGuiPieces_t ChessGuiDeadPieces_GetPiece(const ChessGuiDeadPieces_t *dp, int color, int pos) {
    if (!dp) return CHESS_GUI_EMPTY;
    if (color != GUI_WHITE_PLAYER && color != GUI_BLACK_PLAYER) return CHESS_GUI_EMPTY;
    if (pos < 0 || pos >= GUI_MAX_DEAD_PIECES) return CHESS_GUI_EMPTY;

    return dp->dead[color][pos];
}

void ChessGuiDeadPieces_SetPiece(ChessGuiDeadPieces_t *dp, int color, int pos, ChessGuiPieces_t piece) {
    if (!dp) return;
    if (color != GUI_WHITE_PLAYER && color != GUI_BLACK_PLAYER) return;
    if (pos < 0 || pos >= GUI_MAX_DEAD_PIECES) return;

    dp->dead[color][pos] = piece;
}

// Variabile statica globale che contiene la storia
static game_history_log_t s_game_history = {
    .last_move = -1
};

/**
 * @brief Inizializza la storia (s_game_history).
 */
void game_history_init(void)
{
    s_game_history.last_move = -1;
    memset(s_game_history.records, 0, sizeof(s_game_history.records));
}

/**
 * @brief Restituisce quante mosse sono presenti (0 se last_move=-1).
 */
int game_history_length(void)
{
    return s_game_history.last_move + 1;
}

/**
 * @brief Aggiunge una nuova mossa, se c'è spazio.
 */
bool game_history_add_move(const move_record_t* rec)
{
    if (s_game_history.last_move >= (MAX_HISTORY_MOVES - 1)) {
        // Log pieno
        return false;
    }
    s_game_history.last_move++;
    int idx = s_game_history.last_move;

    s_game_history.records[idx] = *rec;

    // Se move_id == 0, impostiamo un valore 1-based
    if (s_game_history.records[idx].move_id == 0) {
        s_game_history.records[idx].move_id = idx + 1; 
    }
    return true;
}

/**
 * @brief Legge la mossa in posizione index
 */
bool game_history_get_move(int index, move_record_t* outRecord)
{
    if (index < 0 || index > s_game_history.last_move) {
        return false;
    }
    if (!outRecord) {
        return false;
    }

    *outRecord = s_game_history.records[index];
    return true;
}



