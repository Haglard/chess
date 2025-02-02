#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "chess_moves.h"          // Contiene la definizione delle API da testare
#include "chess_state.h"          // Presumibilmente definisce bitboard_state_t, parse_fen, ecc.
#include "obj_dynamic_vector.h"   // Per dv_create, dv_size, dv_get, dv_free
#include "obj_trace.h"
#include "chess_game_dynamics.h"


/* --------------------------------------------------------------------------
 * HELPER PER LA CONVERSIONE DELLE MOSSE
 * -------------------------------------------------------------------------- */

/**
 * @brief Converte una mossa generata (from, to, ecc.) in notazione algebrica semplificata
 *        (es: "e2e1=Q", "exd3e.p."), e la scrive in out_str.
 */
static void convert_move_to_notation_simplified(
    int from, int to, 
    int promotion, 
    int is_en_passant,
    char *out_str
) {
    int from_file = from % 8;
    int from_rank = from / 8;
    int to_file   = to % 8;
    int to_rank   = to / 8;

    char from_file_char = 'a' + from_file;
    char from_rank_char = '1' + from_rank;
    char to_file_char   = 'a' + to_file;
    char to_rank_char   = '1' + to_rank;

    // Base: "e2e1"
    snprintf(out_str, 8, "%c%c%c%c", from_file_char, from_rank_char, to_file_char, to_rank_char);

    // Se c'è promozione
    if (promotion != 0) {
        char promo = '?';
        switch (promotion) {
            case 1: promo = 'N'; break;
            case 2: promo = 'B'; break;
            case 3: promo = 'R'; break;
            case 4: promo = 'Q'; break;
        }
        size_t len = strlen(out_str);
        snprintf(out_str + len, 8 - len, "=%c", promo);
    }
    // Se en passant
    if (is_en_passant) {
        // Aggiunge "e.p."
        size_t len = strlen(out_str);
        snprintf(out_str + len, 8 - len, "e.p.");
    }
}

/**
 * @brief Confronta la notazione di una mossa generata con la lista di mosse attese.
 *
 * @param move_notation Mossa generata in notazione (es: "e1=Q").
 * @param expected Array di stringhe attese.
 * @param expected_count Numero di stringhe attese.
 * @return 1 se la mossa è presente in expected, 0 altrimenti.
 */
static int is_move_in_expected(const char *move_notation, const char **expected, int expected_count) {
    for (int i = 0; i < expected_count; i++) {
        if (strcmp(move_notation, expected[i]) == 0) {
            return 1; // Trovata
        }
    }
    return 0;
}

/* --------------------------------------------------------------------------
 * parse_castling_rights
 * -------------------------------------------------------------------------- */

/**
 * @brief Interpreta i caratteri di un castling right e setta i bit corrispondenti:
 *        bit0 = White K, bit1 = White Q, bit2 = Black k, bit3 = Black q.
 *
 * @param castling_str Esempio: "KQkq", "-", "KQ", "kq", ...
 * @param out_castling Puntatore a un uint8_t dove impostare i bit corrispondenti.
 */
static void parse_castling_rights(const char *castling_str, uint8_t *out_castling) {
    *out_castling = 0;
    if (castling_str[0] == '-' || castling_str[0] == '\0') {
        // Nessun diritto
        return;
    }

    for (int i = 0; castling_str[i] != '\0'; i++) {
        switch (castling_str[i]) {
            case 'K':
                // White short (bit0)
                *out_castling |= 0x1; 
                break;
            case 'Q':
                // White long (bit1)
                *out_castling |= 0x2;
                break;
            case 'k':
                // Black short (bit2)
                *out_castling |= 0x4;
                break;
            case 'q':
                // Black long (bit3)
                *out_castling |= 0x8;
                break;
            default:
                break;
        }
    }
}

/* --------------------------------------------------------------------------
 * place_piece_on_bitboard
 * -------------------------------------------------------------------------- */

/**
 * @brief Converte un carattere di pezzo in minuscolo/maiuscolo in un bit
 *        all'interno del bitboard corrispondente del \c state. 
 *        Esempio: 'p' => black_pawns, 'P' => white_pawns, ...
 *
 * @param c Carattere del pezzo (es: 'p','n','b','r','q','k','P','N','B','R','Q','K')
 * @param sq Indice 0..63 della scacchiera (0=a1, 1=b1, ..., 63=h8).
 * @param state Struttura da popolare.
 */
static void place_piece_on_bitboard(char c, int sq, bitboard_state_t *state) {
    switch (c) {
        // Pezzi neri
        case 'p': state->black_pawns   |= (1ULL << sq); break;
        case 'n': state->black_knights |= (1ULL << sq); break;
        case 'b': state->black_bishops |= (1ULL << sq); break;
        case 'r': state->black_rooks   |= (1ULL << sq); break;
        case 'q': state->black_queens  |= (1ULL << sq); break;
        case 'k':
            // Re nero
            state->black_kings  |= (1ULL << sq);
            break;

        // Pezzi bianchi
        case 'P': state->white_pawns   |= (1ULL << sq); break;
        case 'N': state->white_knights |= (1ULL << sq); break;
        case 'B': state->white_bishops |= (1ULL << sq); break;
        case 'R': state->white_rooks   |= (1ULL << sq); break;
        case 'Q': state->white_queens  |= (1ULL << sq); break;
        case 'K':
            // Re bianco
            state->white_kings  |= (1ULL << sq);
            break;

        default:
            // Carattere sconosciuto o '.' => nessun pezzo
            break;
    }
}

/* --------------------------------------------------------------------------
 * parse_fen_placement
 * -------------------------------------------------------------------------- */

/**
 * @brief Analizza la parte "Piece Placement" di una FEN, 
 *        ad esempio "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR",
 *        e imposta i bit nei rispettivi bitboard del \c state.
 *
 * @param fen_placement Stringa corrispondente alla sezione dei pezzi (prima parte della FEN).
 * @param state Out: i bitboard popolati.
 */
static void parse_fen_placement(const char *fen_placement, bitboard_state_t *state) {
    // Dividiamo in 8 righe separate da '/'
    char copy[128];
    strncpy(copy, fen_placement, sizeof(copy) - 1);
    copy[sizeof(copy) - 1] = '\0';

    char *rows[8] = {0};
    int row_count = 0;

    {
        char *saveptr = NULL;
        char *token = strtok_r(copy, "/", &saveptr);
        while (token && row_count < 8) {
            rows[row_count++] = token;
            token = strtok_r(NULL, "/", &saveptr);
        }
    }

    // row_count dovrebbe essere 8 (una per ogni rank 8..1).
    // rows[0] => rank8, rows[1] => rank7, ...
    // rank che stiamo interpretando = 7 - i (0 => rank8, 7 => rank1)
    for (int i = 0; i < 8; i++) {
        const char *row_str = rows[i];
        int rank = 7 - i; // rank 8..1
        int file = 0;     // 0..7

        for (int idx = 0; row_str[idx] != '\0'; idx++) {
            char c = row_str[idx];
            if (isdigit((unsigned char)c)) {
                // Indica un numero di caselle vuote
                int empty = c - '0';
                file += empty;  // Salta 'empty' colonne
            } else {
                // E' un pezzo (es: 'p', 'k', 'P', 'K', ecc.)
                int sq = rank * 8 + file;
                place_piece_on_bitboard(c, sq, state);
                file++;
            }
            if (file >= 8) break; 
        }
    }
}

/* --------------------------------------------------------------------------
 * parse_fen
 * -------------------------------------------------------------------------- */

/**
 * @brief parse_fen
 *
 * @param fen Esempio: "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
 * @param state (out) bitboard_state_t da valorizzare
 */
void parse_fen(const char *fen, bitboard_state_t *state) {
    // 1) Puliamo lo state
    memset(state, 0, sizeof(*state));
    // Default
    state->en_passant = 255;       // di default nessun en passant
    state->current_player = 1;     // default w
    state->castling_rights = 0;
    state->halfmove_clock = 0;
    state->fullmove_number = 1;

    // 2) Suddividiamo la FEN in campi
    //    FEN standard: <placement> <activeColor> <castling> <enPassant> <halfmove> <fullmove>
    //    Esempio: "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
    // Indici:
    //   0 -> piece placement
    //   1 -> active color (w/b)
    //   2 -> castling rights (KQkq / - / ecc.)
    //   3 -> en passant target (es. e3 / -)
    //   4 -> halfmove clock
    //   5 -> fullmove number

    char fen_copy[256];
    strncpy(fen_copy, fen, sizeof(fen_copy) - 1);
    fen_copy[sizeof(fen_copy) - 1] = '\0';

    char *tokens[6] = {0};
    int token_count = 0;

    {
        char *saveptr = NULL;
        char *tok = strtok_r(fen_copy, " \t\r\n", &saveptr);
        while (tok && token_count < 6) {
            tokens[token_count++] = tok;
            tok = strtok_r(NULL, " \t\r\n", &saveptr);
        }
    }

    // 3) Se abbiamo almeno 1 token, parse piece placement
    if (token_count > 0) {
        parse_fen_placement(tokens[0], state);
    }

    // 4) active color
    if (token_count > 1 && tokens[1]) {
        if (strcmp(tokens[1], "w") == 0) {
            state->current_player = 1;  // white
        } else if (strcmp(tokens[1], "b") == 0) {
            state->current_player = -1; // black
        }
    }

    // 5) castling
    if (token_count > 2 && tokens[2]) {
        parse_castling_rights(tokens[2], &state->castling_rights);
    }

    // 6) en passant
    if (token_count > 3 && tokens[3]) {
        if (strcmp(tokens[3], "-") != 0) {
            // es: "e3"
            char file_c = tokens[3][0];
            char rank_c = tokens[3][1];
            int file = file_c - 'a';
            int rank = rank_c - '1';
            if (file >= 0 && file < 8 && rank >= 0 && rank < 8) {
                int ep_sq = rank * 8 + file;
                state->en_passant = (uint8_t)ep_sq;
            }
        }
    }

    // 7) halfmove clock
    if (token_count > 4 && tokens[4]) {
        int val = atoi(tokens[4]);
        if (val >= 0) {
            state->halfmove_clock = (uint8_t)val;
        }
    }

    // 8) fullmove number
    if (token_count > 5 && tokens[5]) {
        int val = atoi(tokens[5]);
        if (val > 0) {
            state->fullmove_number = (uint8_t)val;
        }
    }
}

/* --------------------------------------------------------------------------
 * STAMPA DELLA SCACCHIERA (SEMPLICE)
 * -------------------------------------------------------------------------- */

/**
 * @brief Stampa la scacchiera in ASCII, utilizzando . per caselle vuote,
 *        p n b r q k per i pezzi neri e P N B R Q K per i pezzi bianchi.
 *
 * @param state Puntatore allo stato bitboard da stampare.
 */
static void print_board_simple(const bitboard_state_t *state)
{
    // Inizializza tutte le caselle a '.'
    char board[64];
    for (int i = 0; i < 64; i++) {
        board[i] = '.';
    }

    // Pezzi neri
    for (int i = 0; i < 64; i++) {
        if (state->black_pawns   & (1ULL << i)) board[i] = 'p';
        if (state->black_knights & (1ULL << i)) board[i] = 'n';
        if (state->black_bishops & (1ULL << i)) board[i] = 'b';
        if (state->black_rooks   & (1ULL << i)) board[i] = 'r';
        if (state->black_queens  & (1ULL << i)) board[i] = 'q';
        if (state->black_kings   & (1ULL << i)) board[i] = 'k';  // <-- Aggiunto
    }

    // Pezzi bianchi
    for (int i = 0; i < 64; i++) {
        if (state->white_pawns   & (1ULL << i)) board[i] = 'P';
        if (state->white_knights & (1ULL << i)) board[i] = 'N';
        if (state->white_bishops & (1ULL << i)) board[i] = 'B';
        if (state->white_rooks   & (1ULL << i)) board[i] = 'R';
        if (state->white_queens  & (1ULL << i)) board[i] = 'Q';
        if (state->white_kings   & (1ULL << i)) board[i] = 'K';  // <-- Aggiunto
    }

    // Stampa a video
    printf("  +------------------------+\n");
    for (int rank = 7; rank >= 0; rank--) {
        printf("%d |", rank + 1);
        for (int file = 0; file < 8; file++) {
            int sq = rank * 8 + file;
            printf(" %c", board[sq]);
        }
        printf(" |\n");
    }
    printf("  +------------------------+\n");
    printf("    a b c d e f g h\n\n");
}

//--------------------------------------------------------------------------------------
// Struttura di test per la generazione delle mosse
typedef struct {
    const char *description;      // Descrizione test
    const char *fen;             // Posizione FEN
    int expected_min_moves;      // Range minimo di mosse attese
    int expected_max_moves;      // Range massimo di mosse attese
} moves_test_t;

// Funzione di stampa semplificata di una singola mossa
static void print_move(const chess_move_t *mv) {
    if (!mv) return;
    if (mv->is_castling) {
        // Determiniamo se è arrocco corto o lungo
        if (mv->to == mv->from + 2) {
            printf("  O-O\n");
        } else {
            printf("  O-O-O\n");
        }
    } else {
        // Mossa standard "e2e4" (+ eventuale e.p. o promozione)
        int from_file = mv->from % 8;
        int from_rank = mv->from / 8;
        int to_file   = mv->to % 8;
        int to_rank   = mv->to / 8;

        char from_file_c = 'a' + from_file;
        char from_rank_c = '1' + from_rank;
        char to_file_c   = 'a' + to_file;
        char to_rank_c   = '1' + to_rank;

        printf("  %c%c%c%c", from_file_c, from_rank_c, to_file_c, to_rank_c);

        if (mv->is_en_passant) {
            printf(" e.p.");
        }
        if (mv->promotion != 0) {
            // 1=N, 2=B, 3=R, 4=Q
            char pc = '?';
            switch (mv->promotion) {
                case 1: pc='N'; break;
                case 2: pc='B'; break;
                case 3: pc='R'; break;
                case 4: pc='Q'; break;
            }
            printf("=%c", pc);
        }
        printf("\n");
    }
}

//--------------------------------------------------------------------------------------
// NUOVA FUNZIONE: Applica la mossa su una copia dello stato e stampa la scacchiera ottenuta
//--------------------------------------------------------------------------------------
static void apply_and_print(bitboard_state_t *original_state, const chess_move_t *mv)
{
    // 1) Creiamo una copia profonda dello stato, così da non modificare 'original_state'
    void *state_copy_v = chess_copy_state(original_state);
    if (!state_copy_v) {
        printf("  ERRORE: impossibile copiare lo stato!\n");
        return;
    }

    bitboard_state_t *state_copy = (bitboard_state_t*)state_copy_v;

    // 2) Applichiamo la mossa
    void *new_state_v = chess_apply_move(state_copy, mv);
    if (!new_state_v) {
        // Mossa illegale o altro => stampiamo un avviso
        printf("  => Mossa ILLEGALE (chess_apply_move ritorna NULL)\n");
        chess_free_state(state_copy);
        return;
    }

    // 3) Stampa la scacchiera risultante
    bitboard_state_t *new_state = (bitboard_state_t*)new_state_v;
    printf("  => Scacchiera dopo la mossa:\n");
    print_board_simple(new_state);
    printf("\n");

    // 4) Dealloca i due stati temporanei
    chess_free_state(new_state);   // risultato dopo la mossa
    chess_free_state(state_copy);  // copia originale
}

//--------------------------------------------------------------------------------------
static void test_chess_moves_all() {
    // Definiamo alcuni test su scenari diversi
    static moves_test_t tests[] = {
        {
            "1) Posizione iniziale classica (bianco muove)",
            "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
            20, 20
        },
        {
            "2) Posizione vuota tranne re bianco su e1",
            "8/8/8/8/8/8/8/4K3 w - - 0 1",
            2, 8
        },
        {
            "3) En passant disponibile (bianco to move)",
            "rnbqkbnr/pppppppp/8/8/4pP2/8/PPPP1PPP/RNBQKBNR w KQkq e3 0 2",
            1, 30
        },
        {
            "4) Promozione possibile (pedone bianco su settima traversa, con Donna nera in h8 e Cavallo nero in g8)",
            "6nq/7P/8/8/8/8/8/k6K w - - 0 1",
            4, 20
        },
        {
            "5) Arrocco effettivo (bianco con rocco corto e lungo)",
            "r3k2r/pppppppp/8/8/8/8/PPPPPPPP/R3K2R w KQ - 0 1",
            2, 10
        },
        {
            "6) Stato con mosse minime (solo Re nero)",
            "8/8/8/8/8/8/8/7k b - - 0 1",
            2, 8
        },
        {
            "7) Enorme promozione + en passant + arrocco nero",
            "r3k2r/1P2P2p/8/3n4/4p3/8/p1pp1PPP/RNBQKBNR b kq e3 0 1",
            1, 60
        }
    };

    int num_tests = (int)(sizeof(tests)/sizeof(tests[0]));
    int passed = 0;

    for (int i = 0; i < num_tests; i++) {
        printf("==================================================\n");
        printf("%s\n", tests[i].description);
        printf("FEN: %s\n\n", tests[i].fen);

        // 1) parse fen
        bitboard_state_t state;
        parse_fen(tests[i].fen, &state);

        // 1bis) Stampa la configurazione della scacchiera in grafica a caratteri
        printf("STATO INIZIALE:\n");
        print_board_simple(&state);   // stampa ASCII della board
        printf("\n");

        // 2) Ottiene le mosse col modulo chess_moves (API)
        dynamic_vector_t* moves = chess_get_moves(&state);
        int nmoves = chess_get_num_moves(moves);

        // 3) Stampa le mosse generate
        printf("Mosse Generate (%d):\n", nmoves);
        for (int m = 0; m < nmoves; m++) {
            chess_move_t *mv = (chess_move_t*)chess_get_move_at(moves, m);
            print_move(mv);
        }

        // 3bis) ESEGUIAMO LA MOSSA su copia e stampiamo la scacchiera risultante
        printf("\n--- Applicazione di ciascuna mossa e stampa risultato ---\n");
        for (int m = 0; m < nmoves; m++) {
            chess_move_t *mv = (chess_move_t*)chess_get_move_at(moves, m);
            if (!mv) continue;

            // Stampa la mossa
            printf("[Mossa #%d] ", m+1);
            print_move(mv);

            // Applica e stampa la scacchiera ottenuta
            apply_and_print(&state, mv);
        }
        printf("--------------------------------------------------------\n\n");

        // 4) Confronto con expected range di mosse
        int match_ok = 1;
        if (nmoves < tests[i].expected_min_moves || nmoves > tests[i].expected_max_moves) {
            match_ok = 0;
        }

        // 5) Esegue un test su chess_copy_move e chess_free_move (se c'è almeno 1 mossa)
        if (nmoves > 0) {
            chess_move_t* mv0 = (chess_move_t*)chess_get_move_at(moves, 0);
            if (mv0) {
                chess_move_t* copy_mv = (chess_move_t*)chess_copy_move(mv0);
                if (!copy_mv) {
                    match_ok = 0;
                    printf("Errore: chess_copy_move ha restituito NULL.\n");
                } else {
                    // Controllo i campi
                    if (copy_mv->from != mv0->from ||
                        copy_mv->to != mv0->to ||
                        copy_mv->promotion != mv0->promotion ||
                        copy_mv->is_castling != mv0->is_castling ||
                        copy_mv->is_en_passant != mv0->is_en_passant)
                    {
                        match_ok = 0;
                        printf("Errore: la copia della mossa non combacia con l'originale.\n");
                    }
                    chess_free_move(copy_mv);
                }
            }
        }

        // 6) PASS o FAIL
        if (match_ok) {
            printf("PASS\n");
            passed++;
        } else {
            printf("FAIL: mosse generate = %d, fuori range [%d..%d]\n", 
                   nmoves, tests[i].expected_min_moves, tests[i].expected_max_moves);
        }

        // 7) Libera vettore
        chess_free_moves(moves);
        printf("\n");
    }

    printf("==================================================\n");
    printf("Test chess_moves: %d/%d PASS.\n", passed, num_tests);
    printf("==================================================\n");
}

int main() {
    // Inizializza stdtrace per scrivere su "chess_test.log" con livello DEBUG
    if (trace_open_file_channel(&stdtrace, "stdtrace", "chess_test.log", TRACE_LEVEL_DEBUG, true))
    {
        TRACE_INFO(&stdtrace, "stdtrace is now logging to 'chess_test.log' at DEBUG level");
    }
    else
    {
        fprintf(stderr, "Impossibile aprire il file di log chess_test.log\n");
    }

    printf("=== TEST chess_apply_move con vettori globali di stati e mosse ===\n");

    test_chess_moves_all();
    return 0;
}
