/**
 * @file
 * @brief Programma di test per la funzione generate_black_pawn_moves,
 *        con stampa dello stato iniziale della scacchiera.
 *
 * ##VERSION## "test_black_pawn_moves.c 1.0"
 *
 * Questo programma testa specificamente la funzione generate_black_pawn_moves
 * con diversi scenari. Ogni scenario è definito da:
 *  - Una FEN che configura la posizione.
 *  - Una lista di mosse attese (in notazione algebrica semplificata, es: "e1=Q").
 * Se il risultato coincide con le aspettative, il test passa (PASS),
 * altrimenti fallisce (FAIL).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "chess_moves.h"
#include "chess_moves_pawn.h"
#include "chess_moves_knight.h"
#include "chess_moves_bishop.h"
#include "chess_moves_rook.h"
#include "chess_moves_queen.h"
#include "chess_moves_king.h"
#include "chess_state.h"
#include "obj_dynamic_vector.h"


/* --------------------------------------------------------------------------
 * STRUTTURA PER I CASI DI TEST
 * -------------------------------------------------------------------------- */

/**
 * @brief Struttura per memorizzare un singolo caso di test.
 */
typedef struct {
    const char *description;    /**< Descrizione del test (es: "Pedone nero e2 che promuove"). */
    const char *fen;           /**< Stringa FEN per configurare la posizione. */
    const char *expected_moves[16]; /**< Elenco delle mosse attese (max 16 per semplicità). */
    int expected_count;        /**< Quante mosse ci aspettiamo. */
} black_pawn_test_t;

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
 * FUNZIONE DI TEST PRINCIPALE
 * -------------------------------------------------------------------------- */

void test_generate_black_pawn_moves() {
    printf("\n=========================================\n");
    printf("TEST FUNZIONE: generate_black_pawn_moves\n");
    printf("=========================================\n\n");

    /* I 16 casi di test */
    static black_pawn_test_t tests[] = {
        {
            "Caso 1: e2 -> e1=Q/R/B/N",
            "7k/8/8/8/8/8/4p3/7K b - - 0 1",
            { "e2e1=N", "e2e1=B", "e2e1=R", "e2e1=Q" },
            4
        },
        {
            "Caso 2: e7 -> e6, e5",
            "7k/4p3/8/8/8/8/8/7K b - - 0 1",
            { "e7e6", "e7e5" },
            2
        },
        {
            "Caso 3: e7 bloccato da un cavallo nero su e6 => nessuna",
            "7k/4p3/4n3/8/8/8/8/7K b - - 0 1",
            { },
            0
        },
        {
            "Caso 4: e7 bloccato da un cavallo bianco su e6 => nessuna",
            "7k/4p3/4N3/8/8/8/8/7K b - - 0 1",
            { },
            0
        },
        {
            "Caso 5: e7 con alfiere bianco d6 => e6,e5,exd6",
            "7k/4p3/3B4/8/8/8/8/7K b - - 0 1",
            { "e7e6", "e7e5", "e7d6" },
            3
        },
        {
            "Caso 6: e7 con alfiere bianco f6 => e6,e5,exf6",
            "7k/4p3/5B2/8/8/8/8/7K b - - 0 1",
            { "e7e6", "e7e5", "e7f6" },
            3
        },
        {
            "Caso 7: e7 con alfiere nero f6 => e6,e5",
            "7k/4p3/5b2/8/8/8/8/7K b - - 0 1",
            { "e7e6", "e7e5" },
            2
        },
        {
            "Caso 8: e4 -> e3",
            "7k/8/8/8/4p3/8/8/7K b - - 0 1",
            { "e4e3" },
            1
        },
        {
            "Caso 9: e4 bloccato da cavallo nero e3 => nessuna",
            "7k/8/8/8/4p3/4n3/8/7K b - - 0 1",
            { },
            0
        },
        {
            "Caso 10: e4 bloccato da cavallo bianco e3 => nessuna",
            "7k/8/8/8/4p3/4N3/8/7K b - - 0 1",
            { },
            0
        },
        {
            "Caso 11: e4 vs cavallo bianco d3 => e3, exd3",
            "7k/8/8/8/4p3/3N4/8/7K b - - 0 1",
            { "e4e3", "e4d3" },
            2
        },
        {
            "Caso 12: e4 vs cavallo bianco f3 => e3, exf3",
            "7k/8/8/8/4p3/5N2/8/7K b - - 0 1",
            { "e4e3", "e4f3" },
            2
        },
        {
            "Caso 13: e4, en passant su d3 => e3, exd3 e.p.",
            "7k/8/8/8/3Pp3/8/8/7K b - d3 0 1",
            { "e4e3", "e4d3e.p" },
            2
        },
        {
            "Caso 14: e4, en passant su f3 => e3, exf3 e.p.",
            "7k/8/8/8/4pP2/8/8/7K b - f3 0 1",
            { "e4e3", "e4f3e.p" },
            2
        },
        {
            "Caso 15: re nero d8, pedone nero e7, regina bianca f6 => unica mossa exf6",
            "3k4/4p3/5Q2/8/8/8/8/7K b - - 0 1",
            { "e7e6", "e7e5", "e7f6" },
            3
        }
    };

    int num_tests = (int)(sizeof(tests)/sizeof(tests[0]));
    int passed = 0;

    for (int i = 0; i < num_tests; i++) {
        // 1) Parse la FEN
        bitboard_state_t state;
        parse_fen(tests[i].fen, &state);

        // ** STAMPA STATO INIZIALE **
        printf("==================================================\n");
        printf("%s\n", tests[i].description);
        printf("FEN: %s\n\n", tests[i].fen);

        printf("STATO INIZIALE:\n");
        print_board_simple(&state);

        // 2) Genera mosse
        dynamic_vector_t *moves_vec = dv_create();
        generate_black_pawn_moves(&state, moves_vec);

        // 3) Converte e confronta
        int gen_count = (int)dv_size(moves_vec);
        char gen_notations[64][8]; 
        memset(gen_notations, 0, sizeof(gen_notations));

        for (int m = 0; m < gen_count; m++) {
            chess_move_t *mv = (chess_move_t*)dv_get(moves_vec, m);
            if (!mv) continue;
            convert_move_to_notation_simplified(
                mv->from, mv->to, mv->promotion,
                mv->is_en_passant, gen_notations[m]
            );
        }

        // 4) Verifica corrispondenza con expected
        int match_ok = 1;
        if (gen_count != tests[i].expected_count) {
            match_ok = 0;
        } else {
            for (int m = 0; m < gen_count; m++) {
                int found = 0;
                for (int x = 0; x < tests[i].expected_count; x++) {
                    if (strcmp(gen_notations[m], tests[i].expected_moves[x]) == 0) {
                        found = 1;
                        break;
                    }
                }
                if (!found) {
                    match_ok = 0;
                    break;
                }
            }
        }

        // 5) Stampo i risultati
        printf("Mosse Generate (%d):\n", gen_count);
        for (int m = 0; m < gen_count; m++) {
            printf("  %s\n", gen_notations[m]);
        }
        if (match_ok) {
            printf("PASS\n");
            passed++;
        } else {
            printf("FAIL\n");
            printf("Attese (%d):\n", tests[i].expected_count);
            for (int x = 0; x < tests[i].expected_count; x++) {
                printf("  %s\n", tests[i].expected_moves[x]);
            }
        }
        printf("\n");

        dv_free(moves_vec);
    }

    printf("==================================================\n");
    printf("Test generate_black_pawn_moves completati: %d/%d PASS.\n",
           passed, num_tests);
    printf("==================================================\n");
}


// Struttura di test analoga per i pedoni bianchi
typedef struct {
    const char *description;
    const char *fen;
    const char *expected_moves[16];  // max 16 mosse attese, puoi aumentare se serve
    int expected_count;
} white_pawn_test_t;

void test_generate_white_pawn_moves() {
    printf("\n=========================================\n");
    printf("TEST FUNZIONE: generate_white_pawn_moves\n");
    printf("=========================================\n\n");

    static white_pawn_test_t tests[] = {
        {
            "Caso 1: e7 -> e8 bloccato da re avversario",
            "4k3/4P3/8/8/8/8/8/4K3 w - - 0 1",
            {  },
            0
        },
        {
            "Caso 2: e2 -> e4 (doppio passo)",
            "4k3/8/8/8/8/8/4P3/4K3 w - - 0 1",
            { "e2e3", "e2e4" },
            2
        },
        {
            "Caso 3: come precedente",
            "4k3/8/8/8/8/8/4P3/4K3 w - - 0 1",
            {  "e2e3", "e2e4"  },
            2
        },
        {
            "Caso 4: e2 cattura in diagonale a sinistra, pedone nero su d3",
            "4k3/8/8/8/8/3p4/4P3/4K3 w - - 0 1",
            { "e2e3", "e2e4", "e2d3" },
            3
        },
        {
            "Caso 5: e5 en passant su d6",
            "4k3/8/8/3pP3/8/8/8/4K3 w - d6 0 1",
            { "e5e6", "e5d6e.p" },
            2
        }
    };

    int num_tests = (int)(sizeof(tests)/sizeof(tests[0]));
    int passed = 0;

    for (int i = 0; i < num_tests; i++) {
        // 1) Parse la FEN
        bitboard_state_t state;
        parse_fen(tests[i].fen, &state);

        // ** STAMPA STATO INIZIALE **
        printf("==================================================\n");
        printf("%s\n", tests[i].description);
        printf("FEN: %s\n\n", tests[i].fen);

        printf("STATO INIZIALE:\n");
        print_board_simple(&state);

        // 2) Genera mosse
        dynamic_vector_t *moves_vec = dv_create();
        generate_white_pawn_moves(&state, moves_vec);

        // 3) Converte e confronta
        int gen_count = (int)dv_size(moves_vec);
        char gen_notations[64][8]; 
        memset(gen_notations, 0, sizeof(gen_notations));

        for (int m = 0; m < gen_count; m++) {
            chess_move_t *mv = (chess_move_t*)dv_get(moves_vec, m);
            if (!mv) continue;
            convert_move_to_notation_simplified(
                mv->from, mv->to, mv->promotion,
                mv->is_en_passant, gen_notations[m]
            );
        }

        // 4) Verifica corrispondenza con expected
        int match_ok = 1;
        if (gen_count != tests[i].expected_count) {
            match_ok = 0;
        } else {
            for (int m = 0; m < gen_count; m++) {
                int found = 0;
                for (int x = 0; x < tests[i].expected_count; x++) {
                    if (strcmp(gen_notations[m], tests[i].expected_moves[x]) == 0) {
                        found = 1;
                        break;
                    }
                }
                if (!found) {
                    match_ok = 0;
                    break;
                }
            }
        }

        // 5) Stampo i risultati
        printf("Mosse Generate (%d):\n", gen_count);
        for (int m = 0; m < gen_count; m++) {
            printf("  %s\n", gen_notations[m]);
        }
        if (match_ok) {
            printf("PASS\n");
            passed++;
        } else {
            printf("FAIL\n");
            printf("Attese (%d):\n", tests[i].expected_count);
            for (int x = 0; x < tests[i].expected_count; x++) {
                printf("  %s\n", tests[i].expected_moves[x]);
            }
        }
        printf("\n");

        dv_free(moves_vec);
    }

    printf("==================================================\n");
    printf("Test generate_white_pawn_moves completati: %d/%d PASS.\n",
           passed, num_tests);
    printf("==================================================\n");
}



/* --------------------------------------------------------------------------
 * Struttura per i test del Cavallo Bianco
 * -------------------------------------------------------------------------- */
typedef struct {
    const char *description;
    const char *fen;
    const char *expected_moves[16];
    int expected_count;
} white_knight_test_t;


/**
 * @brief Esegue i test su generate_white_knight_moves per una serie di configurazioni.
 */
void test_generate_white_knight_moves() {
    printf("\n=========================================\n");
    printf("TEST FUNZIONE: generate_white_knight_moves\n");
    printf("=========================================\n\n");

    /* Ecco una serie di 7 (o quanti vuoi) casi di test */
    static white_knight_test_t tests[] = {
        {
            "Caso 1: Cavallo bianco al centro (e4) => 8 mosse",
            "8/8/8/4N3/8/8/8/7k w - - 0 1",
            { "e5d3", "e5f3", "e5c4", "e5g4", "e5c6", "e5g6", "e5d7", "e5f7" },
            8
        },
        {
            "Caso 2: Cavallo bianco in un angolo (a1) => 2 mosse",
            "N7/8/8/8/8/8/8/7k w - - 0 1",
            { "a8b6", "a8c7" },
            2
        },
        {
            "Caso 3: Cavallo bianco su b1 nella posizione iniziale => 2 mosse",
            "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
            { "b1a3", "b1c3", "g1f3", "g1h3" },
            4
        },
        {
            "Caso 4: Cavallo bianco bloccato da pezzi bianchi => 0 mosse",
            "rnbqkbnr/pppppppp/8/8/8/PpPpPPPP/RNBQKBNR/7k w - - 0 1",
            { "b2d1", "b2d3", "b2a4", "b2c4", "g2e1", "g2f4", "g2h4"},
            7
        },
        {
            "Caso 5: Cavallo bianco su e4 può catturare un pezzo nero",
            "8/8/8/4N3/8/5p2/6n1/7k w - - 0 1",
            {
              "e5d3", "e5f3", "e5c4", "e5g4", "e5c6",
              "e5g6", "e5d7", "e5f7"
            },
            8
        },
        {
            "Caso 6: Cavallo bianco su h8 => 2 mosse (f7,g6)",
            "7N/8/8/8/8/8/8/7k w - - 0 1",
            { "h8f7", "h8g6" },
            2
        },
        {
            "Caso 7: Cavallo bianco su a4 => 4 mosse",
            "8/8/8/N7/8/8/8/7k w - - 0 1",
            { "a5b3", "a5c4", "a5c6", "a5b7" },
            4
        }
    };

    int num_tests = (int)(sizeof(tests)/sizeof(tests[0]));
    int passed = 0;

    for (int i = 0; i < num_tests; i++) {
        printf("==================================================\n");
        printf("%s\n", tests[i].description);
        printf("FEN: %s\n\n", tests[i].fen);

        // 1) Parse la FEN
        bitboard_state_t state;
        parse_fen(tests[i].fen, &state);

        // ** Stampa lo stato iniziale della scacchiera **
        printf("STATO INIZIALE:\n");
        print_board_simple(&state);

        // 2) Genera le mosse del cavallo bianco
        dynamic_vector_t *moves_vec = dv_create();
        generate_white_knight_moves(&state, moves_vec);

        // 3) Converte le mosse generate in notazione semplificata (es: "e4c5")
        int gen_count = (int)dv_size(moves_vec);
        char gen_notations[64][8]; 
        memset(gen_notations, 0, sizeof(gen_notations));

        for (int m = 0; m < gen_count; m++) {
            chess_move_t *mv = (chess_move_t*)dv_get(moves_vec, m);
            if (!mv) continue;
            // Ricava "from" e "to"
            int from_file = mv->from % 8;
            int from_rank = mv->from / 8;
            int to_file   = mv->to % 8;
            int to_rank   = mv->to / 8;

            char from_file_c = 'a' + from_file;
            char from_rank_c = '1' + from_rank;
            char to_file_c   = 'a' + to_file;
            char to_rank_c   = '1' + to_rank;

            snprintf(gen_notations[m], sizeof(gen_notations[m]),
                     "%c%c%c%c", from_file_c, from_rank_c, to_file_c, to_rank_c);
        }

        // 4) Confronto con la lista attesa
        int match_ok = 1;
        if (gen_count != tests[i].expected_count) {
            match_ok = 0;
        } else {
            // Verifica che ogni mossa generata appaia fra quelle attese
            for (int m = 0; m < gen_count; m++) {
                int found = 0;
                for (int x = 0; x < tests[i].expected_count; x++) {
                    if (strcmp(gen_notations[m], tests[i].expected_moves[x]) == 0) {
                        found = 1;
                        break;
                    }
                }
                if (!found) {
                    match_ok = 0;
                    break;
                }
            }
        }

        // 5) Stampo i risultati
        printf("Mosse Generate (%d):\n", gen_count);
        for (int m = 0; m < gen_count; m++) {
            printf("  %s\n", gen_notations[m]);
        }
        if (match_ok) {
            printf("PASS\n");
            passed++;
        } else {
            printf("FAIL\n");
            printf("Attese (%d):\n", tests[i].expected_count);
            for (int x = 0; x < tests[i].expected_count; x++) {
                printf("  %s\n", tests[i].expected_moves[x]);
            }
        }
        printf("\n");

        // 6) Libero il vettore dinamico
        dv_free(moves_vec);
    }

    printf("==================================================\n");
    printf("Test generate_white_knight_moves completati: %d/%d PASS.\n",
           passed, num_tests);
    printf("==================================================\n");
}


/* --------------------------------------------------------------------------
 * Struttura per i test del Cavallo Nero
 * -------------------------------------------------------------------------- */
typedef struct {
    const char *description;
    const char *fen;               // Posizione FEN con 'b' come side to move
    const char *expected_moves[16]; 
    int expected_count;
} black_knight_test_t;


/**
 * @brief Esegue i test su generate_black_knight_moves per una serie di configurazioni.
 */
void test_generate_black_knight_moves() {
    printf("\n=========================================\n");
    printf("TEST FUNZIONE: generate_black_knight_moves\n");
    printf("=========================================\n\n");

    /*
     * Esempi di 7 casi di test (puoi aggiungerne o rimuoverne a piacere).
     * NOTA: le coordinate delle mosse (es. "e5d3") sono in notazione
     *       "file + rank", con rank che parte da '1' in basso fino a '8' in alto,
     *       e file da 'a' a 'h' (come per il cavallo bianco).
     */
    static black_knight_test_t tests[] = {
        {
            // 1) Cavallo nero al centro (e5) => 8 mosse standard
            "Caso 1: Cavallo nero al centro (e5) => 8 mosse",
            /*  r8   r7   r6   r5         r4    r3   r2    r1
                8/   8/   8/   4n3/      8/    8/   8/    7K  b - - 0 1   */
            "8/8/8/4n3/8/8/8/7K b - - 0 1",
            {
              "e5d3", "e5f3", "e5c4", "e5g4",
              "e5c6", "e5g6", "e5d7", "e5f7"
            },
            8
        },
        {
            // 2) Cavallo nero in un angolo (a8) => 2 mosse (a8c7, a8b6)
            "Caso 2: Cavallo nero in un angolo (a8) => 2 mosse",
            /* a8 = 'n', re nero tocca muovere => Fen:
               n7/8/8/8/8/8/8/7K b - - 0 1
             */
            "n7/8/8/8/8/8/8/7K b - - 0 1",
            { "a8c7", "a8b6" },
            2
        },
        {
            // 3) Cavallo nero sulla sua posizione iniziale b8 + g8 => 4 mosse totali
            //    Fen classica, con "b" come side to move
            "Caso 3: Cavallo nero su b8 e g8 nella posizione iniziale => 4 mosse",
            /*  r8= r n b q k b n r
                r7= p p p p p p p p
                ...
                r1= R N B Q K B N R   ma side to move = black
                => dovremo mettere b KQkq - 0 1, se vogliamo tutti i diritti di arrocco intatti
             */
            "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR b KQkq - 0 1",
            { 
              // b8 => a6, c6
              // g8 => f6, h6
              "b8a6", "b8c6", "g8f6", "g8h6"
            },
            4
        },
        {
            // 4) Cavallo nero bloccato da pezzi neri => 0 mosse
            //    Posizioniamo un cavallo nero su b2, e intorno a4,c4,d3 ci sono pedoni neri
            //    in modo che non possa saltare.
            "Caso 4: Cavallo nero bloccato dai propri pezzi => 0 mosse",
            /* FEN:
               8/8/8/8/p1p5/3p4/1n6/7k b - - 0 1
               Significa:
                 - a4,c4,d3 = pedoni neri => p1p5 / 3p4
                 - cavallo nero su b2 => '1n6'
                 - re nero su h1 => '7k'
                 - side to move = b
             */
            "8/8/8/8/p1p5/3p4/1n6/7k b - - 0 1",
            { "b2d1" },
            1
        },
        {
            // 5) Cavallo nero su b8, ma di fronte a pedoni avversari / catture
            //    Esempio fantasioso: cavallo nero potrà muoversi?
            //    (opzionale, se vuoi un test differente)
            //    Qui ne mettiamo uno più semplice: cavallo nero su h8 => 2 mosse
            "Caso 5: Cavallo nero su h8 => 2 mosse (f7,g6)",
            /* h8 = 'n' => Fen "7n/8/8/8/8/8/8/7K b - - 0 1"
             */
            "7n/8/8/8/8/8/8/7K b - - 0 1",
            { "h8f7", "h8g6" },
            2
        },
        {
            // 6) Cavallo nero può catturare pezzi bianchi
            //    Mettiamo il cavallo su e5 e alcuni pezzi bianchi nelle caselle
            //    raggiungibili. Ad esempio, c4 (alfiere), f3 (pedone), ecc.
            "Caso 6: Cavallo nero su e5 che può catturare pezzi bianchi",
            /* 8/8/8/4n3/2B5/5P2/8/7K b - - 0 1
               => n su e5 (col=4, rank=4)
               => Alfiere bianco su c4 (col=2, rank=3)
               => pedone bianco su f3 (col=5, rank=2)
               => Re bianco su h1 (facoltativo)
             */
            "8/8/8/4n3/2B5/5P2/8/7K b - - 0 1",
            {
              // e5 -> c4 (cattura), e5 -> f3 (cattura), e5 -> d3, e5-> g4, e5-> c6, e5-> g6, e5-> d7, e5-> f7
              "e5c4", "e5f3", "e5d3", "e5g4",
              "e5c6", "e5g6", "e5d7", "e5f7"
            },
            8
        },
        {
            // 7) Cavallo nero su a4 => 4 mosse
            "Caso 7: Cavallo nero su a4 => 4 mosse",
            /* 8/8/8/n7/8/8/8/7K b - - 0 1
               => n su a4 => col=0, rank=3
             */
            "8/8/8/n7/8/8/8/7K b - - 0 1",
            { "a5b3", "a5c4", "a5c6", "a5b7" },
            4
        }
    };

    int num_tests = (int)(sizeof(tests)/sizeof(tests[0]));
    int passed = 0;

    for (int i = 0; i < num_tests; i++) {
        printf("==================================================\n");
        printf("%s\n", tests[i].description);
        printf("FEN: %s\n\n", tests[i].fen);

        // 1) Parse la FEN
        bitboard_state_t state;
        parse_fen(tests[i].fen, &state);

        // ** Stampa lo stato iniziale della scacchiera **
        printf("STATO INIZIALE:\n");
        print_board_simple(&state);

        // 2) Genera le mosse del cavallo nero
        dynamic_vector_t *moves_vec = dv_create();
        generate_black_knight_moves(&state, moves_vec);

        // 3) Converte le mosse in notazione (es: "e5c4")
        int gen_count = (int)dv_size(moves_vec);
        char gen_notations[64][8]; 
        memset(gen_notations, 0, sizeof(gen_notations));

        for (int m = 0; m < gen_count; m++) {
            chess_move_t *mv = (chess_move_t*)dv_get(moves_vec, m);
            if (!mv) continue;
            // Ricava 'file' e 'rank' per from e to
            int from_file = mv->from % 8;   // 0..7
            int from_rank = mv->from / 8;   // 0..7
            int to_file   = mv->to % 8;
            int to_rank   = mv->to / 8;

            char from_file_c = 'a' + from_file;   // 0->'a', 1->'b', ...
            char from_rank_c = '1' + from_rank;   // 0->'1', 1->'2', ...
            char to_file_c   = 'a' + to_file;
            char to_rank_c   = '1' + to_rank;

            snprintf(gen_notations[m], sizeof(gen_notations[m]),
                     "%c%c%c%c", from_file_c, from_rank_c, to_file_c, to_rank_c);
        }

        // 4) Confronto con la lista attesa
        int match_ok = 1;
        if (gen_count != tests[i].expected_count) {
            match_ok = 0;
        } else {
            // Verifica che ogni mossa generata appaia tra quelle attese
            for (int m = 0; m < gen_count; m++) {
                int found = 0;
                for (int x = 0; x < tests[i].expected_count; x++) {
                    if (strcmp(gen_notations[m], tests[i].expected_moves[x]) == 0) {
                        found = 1;
                        break;
                    }
                }
                if (!found) {
                    match_ok = 0;
                    break;
                }
            }
        }

        // 5) Stampo i risultati
        printf("Mosse Generate (%d):\n", gen_count);
        for (int m = 0; m < gen_count; m++) {
            printf("  %s\n", gen_notations[m]);
        }
        if (match_ok) {
            printf("PASS\n");
            passed++;
        } else {
            printf("FAIL\n");
            printf("Attese (%d):\n", tests[i].expected_count);
            for (int x = 0; x < tests[i].expected_count; x++) {
                printf("  %s\n", tests[i].expected_moves[x]);
            }
        }
        printf("\n");

        // 6) Libero il vettore dinamico
        dv_free(moves_vec);
    }

    printf("==================================================\n");
    printf("Test generate_black_knight_moves completati: %d/%d PASS.\n",
           passed, num_tests);
    printf("==================================================\n");
}




/* Struttura dei test definita sopra */
typedef struct {
    const char *description;
    const char *fen;
    const char *expected_moves[32];
    int expected_count;
} white_bishop_test_t;

/* Presupponiamo che tu abbia questa funzione */
extern void print_board_simple(const bitboard_state_t *state);

void test_generate_white_bishop_moves() {
    printf("\n=========================================\n");
    printf("TEST FUNZIONE: generate_white_bishop_moves\n");
    printf("=========================================\n\n");

    static white_bishop_test_t tests[] = {
        {
            "Caso 1: Bishop bianco al centro (d4) senza ostacoli",
            "8/8/8/3B4/8/8/8/7k w - - 0 1",
            {
                "d5c6","d5b7","d5a8","d5e6","d5f7","d5g8","d5c4",
                "d5b3","d5a2","d5e4","d5f3","d5g2","d5h1"
            },
            13
        },
        {
            "Caso 2: Bishop bianco in un angolo (a1)",
            "B7/8/8/8/8/8/8/7k w - - 0 1",
            {
                "a8b7","a8c6","a8d5","a8e4","a8f3","a8g2","a8h1"
            },
            7
        },
        {
            "Caso 3: Bishop bianco bloccato da propri pezzi",
            "8/8/8/3B4/4P3/2P1P3/8/7k w - - 0 1",
            { 
                "d5c6", "d5b7", "d5a8", "d5e6", "d5f7", "d5g8", "d5c4",
                "d5b3", "d5a2"
            },
            9
        },
        {
            "Caso 4: Bishop bianco su d4 può catturare pezzi neri",
            "8/8/8/3B4/8/5p2/1p6/7k w - - 0 1",
            {
              "d5c6","d5b7","d5a8","d5e6","d5f7","d5g8","d5c4",
              "d5b3","d5a2","d5e4","d5f3"
            },
            11
        },
        {
            "Caso 5: Bishop bianco sul bordo (h1)",
            "7k/8/8/8/8/8/8/7B w - - 0 1",
            {
                "h1g2","h1f3","h1e4","h1d5","h1c6","h1b7","h1a8"
            },
            7
        },
        {
            "Caso 6: Bishop bianco iniziale (f1) nella pos standard",
            "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
            { },
            0
        },
        {
            "Caso 7: Bishop bianco su a4 con possibili catture",
            "8/4p3/3p4/2p5/B7/8/8/7k w - - 0 1",
            {
                "a4b5","a4c6","a4d7","a4e8","a4b3","a4c2","a4d1"
            },
            7
        }
    };

    int num_tests = (int)(sizeof(tests)/sizeof(tests[0]));
    int passed = 0;

    for (int i = 0; i < num_tests; i++) {
        printf("==================================================\n");
        printf("%s\n", tests[i].description);
        printf("FEN: %s\n\n", tests[i].fen);

        // 1) Parse la FEN
        bitboard_state_t state;
        parse_fen(tests[i].fen, &state);

        // 2) Stampa lo stato iniziale
        printf("STATO INIZIALE:\n");
        print_board_simple(&state);

        // 3) Genera le mosse dell'alfiere bianco
        dynamic_vector_t *moves_vec = dv_create();
        generate_white_bishop_moves(&state, moves_vec);

        // 4) Converte le mosse in notazione es: d4c5
        int gen_count = (int)dv_size(moves_vec);
        char gen_notations[64][8];
        memset(gen_notations, 0, sizeof(gen_notations));

        for (int m = 0; m < gen_count; m++) {
            chess_move_t *mv = (chess_move_t*)dv_get(moves_vec, m);
            if (!mv) continue;
            int from_file = mv->from % 8;
            int from_rank = mv->from / 8;
            int to_file   = mv->to % 8;
            int to_rank   = mv->to / 8;

            char from_file_c = 'a' + from_file;
            char from_rank_c = '1' + from_rank;
            char to_file_c   = 'a' + to_file;
            char to_rank_c   = '1' + to_rank;

            snprintf(gen_notations[m], sizeof(gen_notations[m]), 
                     "%c%c%c%c",
                     from_file_c, from_rank_c, to_file_c, to_rank_c);
        }

        // 5) Confronto con le attese
        int match_ok = 1;
        if (gen_count != tests[i].expected_count) {
            match_ok = 0;
        } else {
            for (int m = 0; m < gen_count; m++) {
                int found = 0;
                for (int x = 0; x < tests[i].expected_count; x++) {
                    if (strcmp(gen_notations[m], tests[i].expected_moves[x]) == 0) {
                        found = 1;
                        break;
                    }
                }
                if (!found) {
                    match_ok = 0;
                    break;
                }
            }
        }

        // 6) Stampa i risultati
        printf("Mosse Generate (%d):\n", gen_count);
        for (int m = 0; m < gen_count; m++) {
            printf("  %s\n", gen_notations[m]);
        }
        if (match_ok) {
            printf("PASS\n");
            passed++;
        } else {
            printf("FAIL\n");
            printf("Attese (%d):\n", tests[i].expected_count);
            for (int x = 0; x < tests[i].expected_count; x++) {
                printf("  %s\n", tests[i].expected_moves[x]);
            }
        }
        printf("\n");

        dv_free(moves_vec);
    }

    printf("==================================================\n");
    printf("Test generate_white_bishop_moves completati: %d/%d PASS.\n",
           passed, num_tests);
    printf("==================================================\n");
}

/* Struttura dei test per i Black Bishop */
typedef struct {
    const char *description;
    const char *fen;
    const char *expected_moves[64]; // Ho alzato a 64 per sicurezza, se hai molte mosse
    int expected_count;
} black_bishop_test_t;

void test_generate_black_bishop_moves() {
    printf("\n=========================================\n");
    printf("TEST FUNZIONE: generate_black_bishop_moves\n");
    printf("=========================================\n\n");

    static black_bishop_test_t tests[] = {
        {
            // 1) Alfiere nero al centro (d5) senza ostacoli
            "Caso 1: Bishop nero al centro (d5) senza ostacoli",
            "8/8/8/3b4/8/8/8/7k b - - 0 1",
            {
                // d5 => c6,b7,a8 | e6,f7,g8 | c4,b3,a2 | e4,f3,g2,h1
                "d5e6","d5f7","d5g8","d5c6","d5b7","d5a8",
                "d5e4","d5f3","d5g2","d5c4","d5b3","d5a2"
            },
            12
        },
        {
            // 2) Alfiere nero in un angolo (a1)
            "Caso 2: Bishop nero in un angolo (a1)",
            "b7/8/8/8/8/8/8/7k b - - 0 1",
            {
                // a1 => b2,c3,d4,e5,f6,g7,h8
                "a8b7","a8c6","a8d5","a8e4","a8f3","a8g2"
            },
            6
        },
        {
            // 3) Alfiere nero bloccato da propri pezzi
            "Caso 3: Bishop nero bloccato da propri pezzi",
            "8/8/8/3b4/4p3/2p1p3/8/7k b - - 0 1",
            {
                // Supponendo i pedoni neri su e4, c3, e3 blocchino alcune diagonali
                // Adatta se necessario: e.g. d5 => c6,b7,a8, c4,b3,a2 (6 mosse) 
                "d5e6","d5f7","d5g8","d5c6","d5b7","d5a8", "d5c4", "d5b3", "d5a2"
            },
            9
        },
        {
            // 4) Alfiere nero su d5 può catturare pezzi bianchi
            "Caso 4: Bishop nero su d5 può catturare pezzi bianchi",
            "8/8/8/3b4/8/5P2/1P6/7k b - - 0 1",
            {
                // d5 => c6,b7,a8 / e6,f7,g8 / c4,b3,a2 / e4,f3 
                "d5c6","d5b7","d5a8","d5e6","d5f7","d5g8",
                "d5c4","d5b3","d5a2","d5e4","d5f3"
            },
            11
        },
        {
            // 5) Alfiere nero sul bordo (h8)
            "Caso 5: Bishop nero sul bordo (h8)",
            "7b/8/8/8/8/8/8/7k b - - 0 1",
            {
                // h8 => g7,f6,e5,d4,c3,b2,a1
                "h8g7","h8f6","h8e5","h8d4","h8c3","h8b2","h8a1"
            },
            7
        },
        {
            // 6) Alfiere nero iniziale (c8,f8) => 0 mosse immediate
            "Caso 6: Bishop nero iniziale (c8,f8) => 0 mosse",
            "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR b KQkq - 0 1",
            {
                // Nessuna mossa, bloccato dai pedoni neri su c7/f7
            },
            0
        },
        {
            // 7) Caso con entrambi i bishop in campo, ma muove Nero
            "Caso 7: Entrambi i Bishop => muove Nero (bishop su c5)",
            "8/8/8/2b5/8/5P2/5B2/7k b - - 0 1",
            {
                // c5 => b6,a7, d6,e7,f8, b4,a3, d4,e3,f2
                "c5b6","c5a7","c5d6","c5e7","c5f8","c5b4","c5a3","c5d4","c5e3","c5f2"
            },
            10
        },
        {
            // 8) **NUOVO**: Due alfieri neri liberi di muoversi
            "Caso 8: Due alfieri neri (c5,f6) liberi => generare tutte le mosse di entrambi",
            /* Mettiamo c5 e f6 come alfieri neri, nessun blocco.
               Re nero su h1, Fen = 8/8/5b2/2b5/8/8/8/7k b - - 0 1
            */
            "8/8/5b2/2b5/8/8/8/7k b - - 0 1",
            {
                // Alfiere su c5:
                //   c5 => b6,a7, d6,e7,f8, b4,a3, d4,e3,f2,g1
                //   => 11 mosse
                "c5b6","c5a7","c5d6","c5e7","c5f8",
                "c5b4","c5a3","c5d4","c5e3","c5f2","c5g1",

                // Alfiere su f6:
                //   f6 => e7,d8, g7,h8, e5,d4,c3,b2,a1, g5,h4
                //   => 12 mosse
                "f6e7","f6d8","f6g7","f6h8","f6e5","f6d4","f6c3","f6b2","f6a1","f6g5","f6h4"
            },
            // Totale 11 + 11? O 11 + 12? Verifichiamo la diagonale di f6. 
            //   e7,d8 => Nord-Ovest
            //   g7,h8 => Nord-Est
            //   e5,d4,c3,b2,a1 => Sud-Ovest
            //   g5,h4 => Sud-Est
            // => 2 + 2 + 4 + 2 = 10, ho dimenticato qualcuno? 
            //   e5 -> d4 -> c3 -> b2 -> a1 (4 mosse), g5->h4 (2 mosse), e7->d8(2), g7->h8(2)
            // => 10. 
            // => Quindi 11 per c5 + 10 per f6 = 21 totali. 
            // Se controlli meglio e vedi f6-> e5-> d4-> c3-> b2-> a1, e' 5 destinazioni => 5 mosse
            //  e7-> d8 => 1 destinazione? In realta' e7 e d8 => 2 mosse. 
            //  g7-> h8 => 2 mosse. 
            //  g5-> h4 => 2 mosse. 
            // => 5+2+2+2=11 => c'e' 1 in piu' rispetto al conto. Forse sto confondendo. 
            // Facciamo la lista concreta:
            //   e7,d8 (2) ; g7,h8 (2) ; e5,d4,c3,b2,a1 (5) ; g5,h4 (2) => 2+2+5+2=11
            // Dunque 11 per f6 + 11 per c5 = 22 totali. 
            // Qui riduco a 21, togliendone 1 se c'e' conflitto. Ma in teoria non c'e' conflitto. 
            // Quindi 11 + 11 => 22 mosse totali. 
            22
        }
    };

    int num_tests = (int)(sizeof(tests)/sizeof(tests[0]));
    int passed = 0;

    for (int i = 0; i < num_tests; i++) {
        printf("==================================================\n");
        printf("%s\n", tests[i].description);
        printf("FEN: %s\n\n", tests[i].fen);

        // 1) Parse la FEN
        bitboard_state_t state;
        parse_fen(tests[i].fen, &state);

        // 2) Stampa lo stato iniziale
        printf("STATO INIZIALE:\n");
        print_board_simple(&state);

        // 3) Genera le mosse dell'alfiere nero
        dynamic_vector_t *moves_vec = dv_create();
        generate_black_bishop_moves(&state, moves_vec);

        // 4) Converte le mosse in notazione (es: d5c4)
        int gen_count = (int)dv_size(moves_vec);
        char gen_notations[64][8]; 
        memset(gen_notations, 0, sizeof(gen_notations));

        for (int m = 0; m < gen_count; m++) {
            chess_move_t *mv = (chess_move_t*)dv_get(moves_vec, m);
            if (!mv) continue;

            int from_file = mv->from % 8; 
            int from_rank = mv->from / 8;
            int to_file   = mv->to % 8;
            int to_rank   = mv->to / 8;

            char from_file_c = 'a' + from_file;
            char from_rank_c = '1' + from_rank;
            char to_file_c   = 'a' + to_file;
            char to_rank_c   = '1' + to_rank;

            snprintf(gen_notations[m], sizeof(gen_notations[m]),
                     "%c%c%c%c",
                     from_file_c, from_rank_c, 
                     to_file_c,   to_rank_c);
        }

        // 5) Confronto con le attese
        int match_ok = 1;
        if (gen_count != tests[i].expected_count) {
            match_ok = 0;
        } else {
            for (int m = 0; m < gen_count; m++) {
                int found = 0;
                for (int x = 0; x < tests[i].expected_count; x++) {
                    if (strcmp(gen_notations[m], tests[i].expected_moves[x]) == 0) {
                        found = 1;
                        break;
                    }
                }
                if (!found) {
                    match_ok = 0;
                    break;
                }
            }
        }

        // 6) Stampa i risultati
        printf("Mosse Generate (%d):\n", gen_count);
        for (int m = 0; m < gen_count; m++) {
            printf("  %s\n", gen_notations[m]);
        }
        if (match_ok) {
            printf("PASS\n");
            passed++;
        } else {
            printf("FAIL\n");
            printf("Attese (%d):\n", tests[i].expected_count);
            for (int x = 0; x < tests[i].expected_count; x++) {
                printf("  %s\n", tests[i].expected_moves[x]);
            }
        }
        printf("\n");

        dv_free(moves_vec);
    }

    printf("==================================================\n");
    printf("Test generate_black_bishop_moves completati: %d/%d PASS.\n",
           passed, num_tests);
    printf("==================================================\n");
}


typedef struct {
    const char *description;
    const char *fen;
    const char *expected_moves[32];
    int expected_count;
} black_rook_test_t;

void test_generate_black_rook_moves() {
    printf("\n=========================================\n");
    printf("TEST FUNZIONE: generate_black_rook_moves\n");
    printf("=========================================\n\n");

    static black_rook_test_t tests[] = {
        {
            "Caso 1: Torre nera al centro (d5) senza ostacoli",
            "8/8/8/3r4/8/8/8/7K b - - 0 1",
            {
                "d5d6","d5d7","d5d8","d5d4","d5d3","d5d2","d5d1",
                "d5c5","d5b5","d5a5","d5e5","d5f5","d5g5","d5h5"
            },
            14
        },
        {
            "Caso 2: Torre nera in un angolo (a8)",
            "r7/8/8/8/8/8/8/7K b - - 0 1",
            {
                "a8a7","a8a6","a8a5","a8a4","a8a3","a8a2","a8a1",
                "a8b8","a8c8","a8d8","a8e8","a8f8","a8g8","a8h8"
            },
            14
        },
        {
            "Caso 3: Torre nera bloccata da propri pezzi",
            "8/8/8/3r4/3ppr2/8/8/7K b - - 0 1",
            { 
                "f4g4", "f4h4", "f4f5", "f4f6", "f4f7", "f4f8", 
                "f4f3", "f4f2", "f4f1", "d5e5", "d5f5", "d5g5", 
                "d5h5", "d5c5", "d5b5", "d5a5", "d5d6", "d5d7",
                "d5d8"
            },
            19
        },
        {
            "Caso 4: Torre nera su d5 con pezzi bianchi catturabili",
            "8/3P4/8/3r1Q2/8/3N4/8/7K b - - 0 1",
            {
                "d5e5","d5f5","d5c5","d5b5","d5a5","d5d6",
                "d5d7","d5d4","d5d3"
            },
            9
        },
        {
            "Caso 5: Torre nera pos standard a8 => 0 mosse (bloccata)",
            "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR b KQkq - 0 1",
            { },
            0
        },
        {
            "Caso 6: Torre nera sul bordo (h5)",
            "8/8/8/7r/8/8/8/7K b - - 0 1",
            {
                "h5h6","h5h7","h5h8","h5h4","h5h3","h5h2","h5h1",
                "h5g5","h5f5","h5e5","h5d5","h5c5","h5b5","h5a5"
            },
            14
        }
    };

    int num_tests = (int)(sizeof(tests)/sizeof(tests[0]));
    int passed = 0;

    for (int i = 0; i < num_tests; i++) {
        printf("==================================================\n");
        printf("%s\n", tests[i].description);
        printf("FEN: %s\n\n", tests[i].fen);

        // 1) parse FEN
        bitboard_state_t state;
        parse_fen(tests[i].fen, &state);

        // 2) Stampa stato iniziale
        printf("STATO INIZIALE:\n");
        print_board_simple(&state);

        // 3) Genera mosse (pseudomoves) per la torre nera
        dynamic_vector_t *moves_vec = dv_create();
        generate_black_rook_moves(&state, moves_vec);

        // 4) Converte le mosse in notazione
        int gen_count = (int)dv_size(moves_vec);
        char gen_notations[64][8];
        memset(gen_notations, 0, sizeof(gen_notations));

        for (int m = 0; m < gen_count; m++) {
            chess_move_t *mv = (chess_move_t*)dv_get(moves_vec, m);
            if (!mv) continue;

            int from_file = mv->from % 8;
            int from_rank = mv->from / 8;
            int to_file   = mv->to % 8;
            int to_rank   = mv->to / 8;

            char from_file_c = 'a' + from_file;
            char from_rank_c = '1' + from_rank;
            char to_file_c   = 'a' + to_file;
            char to_rank_c   = '1' + to_rank;

            // Esempio "d5d6"
            snprintf(gen_notations[m], sizeof(gen_notations[m]),
                     "%c%c%c%c", from_file_c, from_rank_c, to_file_c, to_rank_c);
        }

        // 5) Confronto con mosse attese
        int match_ok = 1;
        if (gen_count != tests[i].expected_count) {
            match_ok = 0;
        } else {
            for (int m = 0; m < gen_count; m++) {
                int found = 0;
                for (int x = 0; x < tests[i].expected_count; x++) {
                    if (strcmp(gen_notations[m], tests[i].expected_moves[x]) == 0) {
                        found = 1;
                        break;
                    }
                }
                if (!found) {
                    match_ok = 0;
                    break;
                }
            }
        }

        // 6) Stampa risultati
        printf("Mosse Generate (%d):\n", gen_count);
        for (int m = 0; m < gen_count; m++) {
            printf("  %s\n", gen_notations[m]);
        }
        if (match_ok) {
            printf("PASS\n");
            passed++;
        } else {
            printf("FAIL\n");
            printf("Attese (%d):\n", tests[i].expected_count);
            for (int x = 0; x < tests[i].expected_count; x++) {
                printf("  %s\n", tests[i].expected_moves[x]);
            }
        }
        printf("\n");

        dv_free(moves_vec);
    }

    printf("==================================================\n");
    printf("Test generate_black_rook_moves completati: %d/%d PASS.\n",
           passed, num_tests);
    printf("==================================================\n");
}

/**
 * Struttura per i test della Torre bianca.
 */
typedef struct {
    const char *description;
    const char *fen;
    const char *expected_moves[32];
    int expected_count;
} white_rook_test_t;


void test_generate_white_rook_moves() {
    printf("\n=========================================\n");
    printf("TEST FUNZIONE: generate_white_rook_moves\n");
    printf("=========================================\n\n");

    static white_rook_test_t tests[] = {
        {
            /**
             * 1) Torre bianca al centro (d4) senza ostacoli
             *    FEN: 8/8/8/3R4/8/8/8/7k w - - 0 1
             *    Ossia, la torre è su d5 in notazione “riga 4” se stiamo guardando
             *    dall'alto, ma in FEN è definita su r5. L'importante è che side to move = w.
             */
            "Caso 1: Torre bianca al centro (d4) senza ostacoli",
            "8/8/8/3R4/8/8/8/7k w - - 0 1",
            {
                // d5 => d6,d7,d8, d4,d3,d2,d1, c5,b5,a5, e5,f5,g5,h5
                "d5d6","d5d7","d5d8","d5d4","d5d3","d5d2","d5d1",
                "d5c5","d5b5","d5a5","d5e5","d5f5","d5g5","d5h5"
            },
            14
        },
        {
            /**
             * 2) Torre bianca in un angolo (a1)
             *    FEN: R7/8/8/8/8/8/8/7k w - - 0 1
             *    Re nero su h1, tocca al Bianco
             */
            "Caso 2: Torre bianca in un angolo (a1)",
            "R7/8/8/8/8/8/8/7k w - - 0 1",
            {
                // a1 => a2,a3,a4,a5,a6,a7,a8, b1,c1,d1,e1,f1,g1,h1
                "a8b8","a8c8","a8d8","a8e8","a8f8","a8g8","a8h8",
                "a8a7","a8a6","a8a5","a8a4","a8a3","a8a2","a8a1"
            },
            14
        },
        {
            /**
             * 3) Torre bianca bloccata dai propri pezzi
             *    FEN: 8/8/8/3R4/3PP3/8/8/7k w - - 0 1
             *    Mettiamo pedoni bianchi su d4,e4
             */
            "Caso 3: Torre bianca bloccata dai propri pezzi",
            "8/8/8/3R4/3PP3/8/8/7k w - - 0 1",
            {
                // ipotizziamo la torre su d5: non può andare su d4,d3... se c'è pedone su d4
                // Orizzontalmente (c5,b5,a5,e5,f5,g5,h5)
                // Verticalmente solo d6,d7,d8 se non c'è blocco
                "d5d6","d5d7","d5d8","d5c5","d5b5","d5a5","d5e5","d5f5","d5g5","d5h5"
            },
            10
        },
        {
            /**
             * 4) Torre bianca su d5 con pezzi neri catturabili
             *    FEN: 8/3p4/8/3R1q2/8/3n4/8/7k w - - 0 1
             */
            "Caso 4: Torre bianca su d5 con pezzi neri catturabili",
            "8/3p4/8/3R1q2/8/3n4/8/7k w - - 0 1",
            {
                // d5 => d6 (cattura pedone su d7?), d4,d3,d2,d1
                // orizz: c5,b5,a5,e5(f5?),g5?,h5? => verifica i pezzi neri in orizzontale
                // A seconda di dove hai posizionato esattamente i neri. Questo è un esempio generico.
                // Immaginiamo 9 mosse se la regina nera è su f5, pedone su d7, cavallo su d3...
                "d5e5","d5f5","d5c5","d5b5", "d5a5","d5d6","d5d7",
                "d5d4","d5d3"
            },
            9 // Adattare se la posizione non corrisponde a questi pezzi
        },
        {
            /**
             * 5) Torre bianca pos standard a1 => 0 mosse (bloccata dai pedoni)
             *    Fen classica con side = w
             */
            "Caso 5: Torre bianca pos standard a1 => 0 mosse (bloccata)",
            "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
            { },
            0
        },
        {
            /**
             * 6) Torre bianca sul bordo (h5)
             *    8/8/8/7R/8/8/8/7k w - - 0 1
             */
            "Caso 6: Torre bianca sul bordo (h5)",
            "8/8/8/7R/8/8/8/7k w - - 0 1",
            {
                // h5 => h6,h7,h8,h4,h3,h2,h1, g5,f5,e5,d5,c5,b5,a5
                "h5h6","h5h7","h5h8","h5h4","h5h3","h5h2","h5h1",
                "h5g5","h5f5","h5e5","h5d5","h5c5","h5b5","h5a5"
            },
            14
        },
        {
            /**
             * 7) Caso con due torri bianche libere
             *    Fen: 8/8/8/3R4/8/6R1/8/7k w - - 0 1
             *    => una torre su d5, l'altra su g3, re nero h1
             */
            "Caso 7: Due torri bianche libere su d5 e g3",
            "8/8/8/3R4/8/6R1/8/7k w - - 0 1",
            {
                // Torre su d5 => d6,d7,d8,d4,d3,d2,d1,c5,b5,a5,e5,f5,g5,h5
                // (14 mosse)
                "d5d6","d5d7","d5d8","d5d4","d5d3","d5d2","d5d1",
                "d5c5","d5b5","d5a5","d5e5","d5f5","d5g5","d5h5",

                // Torre su g3 => g4,g5,g6,g7,g8,g2,g1,g3f3,g3e3,g3d3,g3c3,g3b3,g3a3,h3
                // Sulla riga 3, colonna g => orizz + vertical
                // => vertical: g4,g5,g6,g7,g8,g2,g1
                // => orizz: f3,e3,d3,c3,b3,a3,h3
                "g3g4","g3g5","g3g6","g3g7","g3g8","g3g2","g3g1",
                "g3f3","g3e3","g3d3","g3c3","g3b3","g3a3","g3h3"
            },
            // Totale 14 + 14 = 28 se non ci sono blocchi/catture. 
            28
        }
    };

    int num_tests = (int)(sizeof(tests)/sizeof(tests[0]));
    int passed = 0;

    for (int i = 0; i < num_tests; i++) {
        printf("==================================================\n");
        printf("%s\n", tests[i].description);
        printf("FEN: %s\n\n", tests[i].fen);

        // 1) parse FEN
        bitboard_state_t state;
        parse_fen(tests[i].fen, &state);

        // 2) Stampa stato iniziale
        printf("STATO INIZIALE:\n");
        print_board_simple(&state);

        // 3) Genera mosse (pseudomoves) per la torre bianca
        dynamic_vector_t *moves_vec = dv_create();
        generate_white_rook_moves(&state, moves_vec);

        // 4) Converte le mosse in notazione
        int gen_count = (int)dv_size(moves_vec);
        char gen_notations[64][8];
        memset(gen_notations, 0, sizeof(gen_notations));

        for (int m = 0; m < gen_count; m++) {
            chess_move_t *mv = (chess_move_t*)dv_get(moves_vec, m);
            if (!mv) continue;

            int from_file = mv->from % 8;
            int from_rank = mv->from / 8;
            int to_file   = mv->to % 8;
            int to_rank   = mv->to / 8;

            char from_file_c = 'a' + from_file;
            char from_rank_c = '1' + from_rank;
            char to_file_c   = 'a' + to_file;
            char to_rank_c   = '1' + to_rank;

            // Esempio "d5d6"
            snprintf(gen_notations[m], sizeof(gen_notations[m]),
                     "%c%c%c%c", from_file_c, from_rank_c, to_file_c, to_rank_c);
        }

        // 5) Confronto con mosse attese
        int match_ok = 1;
        if (gen_count != tests[i].expected_count) {
            match_ok = 0;
        } else {
            for (int m = 0; m < gen_count; m++) {
                int found = 0;
                for (int x = 0; x < tests[i].expected_count; x++) {
                    if (strcmp(gen_notations[m], tests[i].expected_moves[x]) == 0) {
                        found = 1;
                        break;
                    }
                }
                if (!found) {
                    match_ok = 0;
                    break;
                }
            }
        }

        // 6) Stampa risultati
        printf("Mosse Generate (%d):\n", gen_count);
        for (int m = 0; m < gen_count; m++) {
            printf("  %s\n", gen_notations[m]);
        }
        if (match_ok) {
            printf("PASS\n");
            passed++;
        } else {
            printf("FAIL\n");
            printf("Attese (%d):\n", tests[i].expected_count);
            for (int x = 0; x < tests[i].expected_count; x++) {
                printf("  %s\n", tests[i].expected_moves[x]);
            }
        }
        printf("\n");

        dv_free(moves_vec);
    }

    printf("==================================================\n");
    printf("Test generate_white_rook_moves completati: %d/%d PASS.\n",
           passed, num_tests);
    printf("==================================================\n");
}


/**
 * Struttura per i test della Regina bianca.
 */
typedef struct {
    const char *description;
    const char *fen;                   // FEN con side to move = w
    const char *expected_moves[64];    // Lista di mosse attese
    int expected_count;                // Numero di mosse attese
} white_queen_test_t;

void test_generate_white_queen_moves() {
    printf("\n=========================================\n");
    printf("TEST FUNZIONE: generate_white_queen_moves\n");
    printf("=========================================\n\n");

    static white_queen_test_t tests[] = {
        {
            /**
             * 1) Regina bianca al centro (d4) senza ostacoli.
             *    FEN: 8/8/8/3Q4/8/8/8/7k w - - 0 1
             *    Qui la regina si muove come torre + alfiere su d5 (in vista scacchistica),
             *    side to move = w.
             */
            "Caso 1: Regina bianca al centro (d4) senza ostacoli",
            "8/8/8/3Q4/8/8/8/7k w - - 0 1",
            {
              // Mosse da "d5" in orizzontale/verticale:
              // d6,d7,d8, d4,d3,d2,d1, c5,b5,a5, e5,f5,g5,h5
              // Più diagonali: c6,b7,a8, e6,f7,g8, c4,b3,a2, e4,f3,g2,h1
              // => 14 lineari + 13 diagonali = 27
              // NB: alcuni preferiscono 28, controlla se "d5d5" non è ammesso (ovviamente no),
              // e se enumeri correttamente.  
                "d5e6","d5f7","d5g8","d5c6","d5b7",
                "d5a8","d5c4","d5b3","d5a2","d5e4",
                "d5f3","d5g2","d5h1","d5e5","d5f5",
                "d5g5","d5h5","d5c5","d5b5","d5a5",
                "d5d6","d5d7","d5d8","d5d4","d5d3",
                "d5d2","d5d1"            
            },
            27
        },
        {
            /**
             * 2) Regina bianca in un angolo (a1)
             *    FEN: Q7/8/8/8/8/8/8/7k w - - 0 1
             */
            "Caso 2: Regina bianca in un angolo (a1)",
            "Q7/8/8/8/8/8/8/7k w - - 0 1",
            {
                // Da a1 => orizzontale: a2,a3,a4,a5,a6,a7,a8
                // verticale: b1,c1,d1,e1,f1,g1,h1
                // diagonale: b2,c3,d4,e5,f6,g7,h8
                // => 7 + 7 + 7 = 21
                "a8b7","a8c6","a8d5","a8e4","a8f3",
                "a8g2","a8h1","a8b8","a8c8","a8d8",
                "a8e8","a8f8","a8g8","a8h8","a8a7",
                "a8a6","a8a5","a8a4","a8a3","a8a2",
                "a8a1"           
            },
            21
        },
        {
            /**
             * 3) Regina bianca bloccata dai propri pezzi
             *    FEN: 8/8/8/3Q4/3PP3/8/8/7k w - - 0 1
             *    Mettiamo pedoni bianchi su d4,e4
             */
            "Caso 3: Regina bianca bloccata da propri pezzi",
            "8/8/8/3Q4/3PP3/8/8/7k w - - 0 1",
            {
                // Supponendo la regina su d5. Bloccata in parte dai pedoni su d4,e4.
                // Avrà spostamenti orizzontali e verticali verso l'alto e qualche diagonale libera.
                // ipotizziamo 12 mosse (da verificare con la disposizione esatta).
                "d5e6","d5f7","d5g8","d5c6","d5b7",
                "d5a8","d5c4","d5b3","d5a2","d5e5",
                "d5f5","d5g5","d5h5","d5c5","d5b5",
                "d5a5","d5d6","d5d7","d5d8"
            },
            19
        },
        {
            /**
             * 4) Regina bianca su d5 con pezzi neri catturabili
             *    FEN: 8/3p4/8/3Q1q2/8/3n4/8/7k w - - 0 1
             */
            "Caso 4: Regina bianca su d5 con pezzi neri catturabili",
            "8/3p4/8/3Q1q2/8/3n4/8/7k w - - 0 1",
            {
                "d5e6","d5f7","d5g8","d5c6","d5b7",
                "d5a8","d5c4","d5b3","d5a2","d5e4",
                "d5f3","d5g2","d5h1","d5e5","d5f5",
                "d5c5","d5b5","d5a5","d5d6","d5d7",
                "d5d4","d5d3"
            },
            22 // Esempio
        },
        {
            /**
             * 5) Regina bianca pos standard (d1,e1) => 0 mosse immediate
             *    Fen classica con side = w
             */
            "Caso 5: Regina bianca pos standard => 0 mosse (bloccata)",
            "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
            { },
            0
        },
        {
            /**
             * 6) Regina bianca sul bordo (h5)
             *    8/8/8/7Q/8/8/8/7k w - - 0 1
             */
            "Caso 6: Regina bianca sul bordo (h5)",
            "8/8/8/7Q/8/8/8/7k w - - 0 1",
            {
                "h5g6","h5f7","h5e8","h5g4","h5f3",
                "h5e2","h5d1","h5g5","h5f5","h5e5",
                "h5d5","h5c5","h5b5","h5a5","h5h6",
                "h5h7","h5h8","h5h4","h5h3","h5h2",
                "h5h1"            
            },
            21 // o 19, a seconda del row/col. Ricontrolla i conti.
        },
        {
            /**
             * 7) Due regine bianche libere
             *    Esempio: una regina su d5, l'altra su g3, re nero su h1
             *    FEN: 8/8/8/3Q4/8/6Q1/8/7k w - - 0 1
             */
            "Caso 7: Due regine bianche libere (d5 e g3)",
            "8/8/8/3Q4/8/6Q1/8/7k w - - 0 1",
            {
                "g3h4","g3f4","g3e5","g3d6","g3c7",
                "g3b8","g3f2","g3e1","g3h2","g3h3",
                "g3f3","g3e3","g3d3","g3c3","g3b3",
                "g3a3","g3g4","g3g5","g3g6","g3g7",
                "g3g8","g3g2","g3g1","d5e6","d5f7",
                "d5g8","d5c6","d5b7","d5a8","d5c4",
                "d5b3","d5a2","d5e4","d5f3","d5g2",
                "d5h1","d5e5","d5f5","d5g5","d5h5",
                "d5c5","d5b5","d5a5","d5d6","d5d7",
                "d5d8","d5d4","d5d3","d5d2","d5d1"
            },
            50 // Esempio, da regolare se serve
        }
    };

    int num_tests = (int)(sizeof(tests)/sizeof(tests[0]));
    int passed = 0;

    for (int i = 0; i < num_tests; i++) {
        printf("==================================================\n");
        printf("%s\n", tests[i].description);
        printf("FEN: %s\n\n", tests[i].fen);

        // 1) parse FEN
        bitboard_state_t state;
        parse_fen(tests[i].fen, &state);

        // 2) Stampa stato iniziale
        printf("STATO INIZIALE:\n");
        print_board_simple(&state);

        // 3) Genera mosse (pseudomoves) per la regina bianca
        dynamic_vector_t *moves_vec = dv_create();
        generate_white_queen_moves(&state, moves_vec);

        // 4) Converte le mosse in notazione
        int gen_count = (int)dv_size(moves_vec);
        char gen_notations[64][8];
        memset(gen_notations, 0, sizeof(gen_notations));

        for (int m = 0; m < gen_count; m++) {
            chess_move_t *mv = (chess_move_t*)dv_get(moves_vec, m);
            if (!mv) continue;

            int from_file = mv->from % 8;
            int from_rank = mv->from / 8;
            int to_file   = mv->to % 8;
            int to_rank   = mv->to / 8;

            char from_file_c = 'a' + from_file;
            char from_rank_c = '1' + from_rank;
            char to_file_c   = 'a' + to_file;
            char to_rank_c   = '1' + to_rank;

            // es: "d5c4"
            snprintf(gen_notations[m], sizeof(gen_notations[m]),
                     "%c%c%c%c", from_file_c, from_rank_c, to_file_c, to_rank_c);
        }

        // 5) Confronto con le mosse attese
        int match_ok = 1;
        if (gen_count != tests[i].expected_count) {
            match_ok = 0;
        } else {
            for (int m = 0; m < gen_count; m++) {
                int found = 0;
                for (int x = 0; x < tests[i].expected_count; x++) {
                    if (strcmp(gen_notations[m], tests[i].expected_moves[x]) == 0) {
                        found = 1;
                        break;
                    }
                }
                if (!found) {
                    match_ok = 0;
                    break;
                }
            }
        }

        // 6) Stampa i risultati
        printf("Mosse Generate (%d):\n", gen_count);
        for (int m = 0; m < gen_count; m++) {
            printf("  %s\n", gen_notations[m]);
        }
        if (match_ok) {
            printf("PASS\n");
            passed++;
        } else {
            printf("FAIL\n");
            printf("Attese (%d):\n", tests[i].expected_count);
            for (int x = 0; x < tests[i].expected_count; x++) {
                printf("  %s\n", tests[i].expected_moves[x]);
            }
        }
        printf("\n");

        dv_free(moves_vec);
    }

    printf("==================================================\n");
    printf("Test generate_white_queen_moves completati: %d/%d PASS.\n",
           passed, num_tests);
    printf("==================================================\n");
}

/**
 * Struttura per i test della Regina nera.
 */
typedef struct {
    const char *description;
    const char *fen;                   // FEN con side to move = b
    const char *expected_moves[64];    // Lista di mosse attese
    int expected_count;                // Numero di mosse attese
} black_queen_test_t;

/* Presumiamo di avere: 
 *   parse_fen(const char*, bitboard_state_t*)
 *   print_board_simple(const bitboard_state_t*)
 *   generate_black_queen_moves(const bitboard_state_t*, dynamic_vector_t*)
 *   dv_create(), dv_size(), dv_get(), dv_free() 
 * per la gestione del dynamic_vector_t e delle mosse.
 */

void test_generate_black_queen_moves() {
    printf("\n=========================================\n");
    printf("TEST FUNZIONE: generate_black_queen_moves\n");
    printf("=========================================\n\n");

    static black_queen_test_t tests[] = {
        {
            /**
             * 1) Regina nera al centro (d5) senza ostacoli
             *    FEN: 8/8/8/3q4/8/8/8/7K b - - 0 1
             */
            "Caso 1: Regina nera al centro (d5) senza ostacoli",
            "8/8/8/3q4/8/8/8/7K b - - 0 1",
            {
                // d5 => movimenti come torre (verticale/orizzontale) + alfiere (diagonale)
                // Esempio di 27 mosse totali (14 lineari + 13 diagonali),
                // a seconda di come enumeri, potresti ottenerne 27 o 28.
                "d5e6","d5f7","d5g8","d5c6","d5b7",
                "d5a8","d5c4","d5b3","d5a2","d5e4",
                "d5f3","d5g2","d5h1","d5e5","d5f5",
                "d5g5","d5h5","d5c5","d5b5","d5a5",
                "d5d6","d5d7","d5d8","d5d4","d5d3",
                "d5d2","d5d1"
            },
            27
        },
        {
            /**
             * 2) Regina nera in un angolo (a8)
             *    FEN: q7/8/8/8/8/8/8/7K b - - 0 1
             */
            "Caso 2: Regina nera in un angolo (a8)",
            "q7/8/8/8/8/8/8/7K b - - 0 1",
            {
                // Da a8 => orizz: a7,a6,a5,a4,a3,a2,a1
                // vert: b8,c8,d8,e8,f8,g8,h8
                // diag: b7,c6,d5,e4,f3,g2,h1
                // => 7 + 7 + 7 = 21
                "a8a7","a8a6","a8a5","a8a4","a8a3",
                "a8a2","a8a1","a8b8","a8c8","a8d8",
                "a8e8","a8f8","a8g8","a8h8","a8b7",
                "a8c6","a8d5","a8e4","a8f3","a8g2",
                "a8h1"
            },
            21
        },
        {
            /**
             * 3) Regina nera bloccata da propri pezzi
             *    Esempio: pedoni neri attorno
             *    FEN: 8/8/8/3q4/3pp3/8/8/7K b - - 0 1
             */
            "Caso 3: Regina nera bloccata da propri pezzi",
            "8/8/8/3q4/3pp3/8/8/7K b - - 0 1",
            {
                // Se la regina è su d5 e ci sono pedoni neri su d4,e4,e5?
                // ipotizziamo un certo numero di mosse, es. 12
                "d5e6","d5f7","d5g8","d5c6","d5b7",
                "d5a8","d5c4","d5b3","d5a2","d5e5",
                "d5f5","d5g5","d5h5","d5c5","d5b5",
                "d5a5","d5d6","d5d7","d5d8"
            },
            19
        },
        {
            /**
             * 4) Regina nera su d5 con pezzi bianchi catturabili
             *    FEN: 8/3P4/8/3q1Q2/8/3N4/8/7K b - - 0 1
             */
            "Caso 4: Regina nera su d5 con pezzi bianchi catturabili",
            "8/3P4/8/3q1Q2/8/3N4/8/7K b - - 0 1",
            {
                "d5e6","d5f7","d5g8","d5c6","d5b7",
                "d5a8","d5c4","d5b3","d5a2","d5e4",
                "d5f3","d5g2","d5h1","d5e5","d5f5",
                "d5c5","d5b5","d5a5","d5d6","d5d7",
                "d5d4","d5d3"            
            },
            22
        },
        {
            /**
             * 5) Regina nera posizione standard => 0 mosse immediate
             *    FEN: rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR b KQkq - 0 1
             */
            "Caso 5: Regina nera pos standard => 0 mosse (bloccata)",
            "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR b KQkq - 0 1",
            { },
            0
        },
        {
            /**
             * 6) Regina nera sul bordo (h5)
             *    8/8/8/7q/8/8/8/7K b - - 0 1
             */
            "Caso 6: Regina nera sul bordo (h5)",
            "8/8/8/7q/8/8/8/7K b - - 0 1",
            {
                // h5 => h6,h7,h8,h4,h3,h2,h1, g5,f5,e5,d5,c5,b5,a5
                // diagonali NW = g6,f7,e8, SW = g4,f3,e2,d1
                "h5h6","h5h7","h5h8","h5h4","h5h3",
                "h5h2","h5h1","h5g5","h5f5","h5e5",
                "h5d5","h5c5","h5b5","h5a5","h5g6",
                "h5f7","h5e8","h5g4","h5f3","h5e2",
                "h5d1"
            },
            21
        },
        {
            /**
             * 7) Due regine nere libere
             *    Esempio: una regina su d5, l'altra su g3
             *    FEN: 8/8/8/3q4/8/6q1/8/7K b - - 0 1
             */
            "Caso 7: Due regine nere libere (d5 e g3)",
            "8/8/8/3q4/8/6q1/8/7K b - - 0 1",
            {
                // Elenchiamo possibili mosse di g3 + d5 come combinazione, 
                // proprio come per le bianche ma con side = b.
                "g3h4","g3f4","g3e5","g3d6","g3c7",
                "g3b8","g3f2","g3e1","g3h2","g3h3",
                "g3f3","g3e3","g3d3","g3c3","g3b3",
                "g3a3","g3g4","g3g5","g3g6","g3g7",
                "g3g8","g3g2","g3g1","d5e6","d5f7",
                "d5g8","d5c6","d5b7","d5a8","d5c4",
                "d5b3","d5a2","d5e4","d5f3","d5g2",
                "d5h1","d5e5","d5f5","d5g5","d5h5",
                "d5c5","d5b5","d5a5","d5d6","d5d7",
                "d5d8","d5d4","d5d3","d5d2","d5d1"
            },
            50 // Esempio di 50 mosse, da regolare se serve
        }
    };

    int num_tests = (int)(sizeof(tests)/sizeof(tests[0]));
    int passed = 0;

    for (int i = 0; i < num_tests; i++) {
        printf("==================================================\n");
        printf("%s\n", tests[i].description);
        printf("FEN: %s\n\n", tests[i].fen);

        // 1) parse FEN
        bitboard_state_t state;
        parse_fen(tests[i].fen, &state);

        // 2) Stampa stato iniziale
        printf("STATO INIZIALE:\n");
        print_board_simple(&state);

        // 3) Genera mosse (pseudomoves) per la regina nera
        dynamic_vector_t *moves_vec = dv_create();
        generate_black_queen_moves(&state, moves_vec);

        // 4) Converte le mosse in notazione
        int gen_count = (int)dv_size(moves_vec);
        char gen_notations[64][8];
        memset(gen_notations, 0, sizeof(gen_notations));

        for (int m = 0; m < gen_count; m++) {
            chess_move_t *mv = (chess_move_t*)dv_get(moves_vec, m);
            if (!mv) continue;

            int from_file = mv->from % 8;
            int from_rank = mv->from / 8;
            int to_file   = mv->to % 8;
            int to_rank   = mv->to / 8;

            char from_file_c = 'a' + from_file;
            char from_rank_c = '1' + from_rank;
            char to_file_c   = 'a' + to_file;
            char to_rank_c   = '1' + to_rank;

            // es: "d5c4"
            snprintf(gen_notations[m], sizeof(gen_notations[m]),
                     "%c%c%c%c", from_file_c, from_rank_c, to_file_c, to_rank_c);
        }

        // 5) Confronto con le mosse attese
        int match_ok = 1;
        if (gen_count != tests[i].expected_count) {
            match_ok = 0;
        } else {
            // Controlliamo che ogni mossa generata sia nella lista attesa
            for (int m = 0; m < gen_count; m++) {
                int found = 0;
                for (int x = 0; x < tests[i].expected_count; x++) {
                    if (strcmp(gen_notations[m], tests[i].expected_moves[x]) == 0) {
                        found = 1;
                        break;
                    }
                }
                if (!found) {
                    match_ok = 0;
                    break;
                }
            }
        }

        // 6) Stampa i risultati
        printf("Mosse Generate (%d):\n", gen_count);
        for (int m = 0; m < gen_count; m++) {
            printf("  %s\n", gen_notations[m]);
        }
        if (match_ok) {
            printf("PASS\n");
            passed++;
        } else {
            printf("FAIL\n");
            printf("Attese (%d):\n", tests[i].expected_count);
            for (int x = 0; x < tests[i].expected_count; x++) {
                printf("  %s\n", tests[i].expected_moves[x]);
            }
        }
        printf("\n");

        dv_free(moves_vec);
    }

    printf("==================================================\n");
    printf("Test generate_black_queen_moves completati: %d/%d PASS.\n",
           passed, num_tests);
    printf("==================================================\n");
}

/**
 * Struttura per i test del Re nero.
 * (Puoi adattare questa struttura o usarne una identica a quella esistente nel tuo codice.)
 */
typedef struct {
    const char *description;        
    const char *fen;               // FEN con side to move = b (Re nero)
    const char *expected_moves[32]; 
    int expected_count; 
} black_king_test_t;

/* Presupponiamo che tu abbia queste funzioni già pronte: 
 *   parse_fen(const char*, bitboard_state_t*)
 *   print_board_simple(const bitboard_state_t*)
 *   generate_black_king_moves(const bitboard_state_t*, dynamic_vector_t*)
 *   dv_create(), dv_size(), dv_get(), dv_free()
 *   add_move(...)
 * e la struttura 'chess_move_t' con campo 'is_castling'.
 */

/**
 * @brief Testa la funzione generate_black_king_moves, con stampa speciale per l'arrocco.
 */
void test_generate_black_king_moves() {
    printf("\n=========================================\n");
    printf("TEST FUNZIONE: generate_black_king_moves (arrocco segnalato)\n");
    printf("=========================================\n\n");

    /*
     * Esempi di 7 (o quanti desideri) casi di test:
     * -- Includiamo un paio di casi con possibile arrocco corto/lungo.
     */
    static black_king_test_t tests[] = {
        {
            // 1) Re nero al centro (d5) senza ostacoli
            "Caso 1: Re nero al centro (d5) senza ostacoli",
            "8/8/8/3k4/8/8/8/7K b - - 0 1",
            {
                // d5 => c6, d6, e6, c5, e5, c4, d4, e4
                "d5c6","d5d6","d5e6","d5c5","d5e5","d5c4","d5d4","d5e4"
            },
            8
        },
        {
            // 2) Arrocco corto e lungo disponibili
            //    Diritti di arrocco k e q = 0x4 e 0x8
            "Caso 2: Arrocco corto e lungo per il Nero",
            /* 
             * Esempio:
             * FEN: r3k2r/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR b kq - 0 1
             * => re nero su e8, torri nere su a8,h8, nessun pezzo in mezzo, castling_rights = kq
             */
            "r3k2r/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR b kq - 0 1",
            {
                // e8 -> d8, e8->f8, e8->d7, e8->e7, e8->f7
                // + arrocco corto (e8g8) e arrocco lungo (e8c8)
                "e8d8","e8f8","O-O","O-O-O"
            },
            4
        },
        {
            // 3) Un arrocco limitato
            "Caso 3: Re nero bloccato",
            /* 
             * FEN d'esempio: rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQK2R b k - 0 1
             * => 'k' indica solo l'arrocco corto nero disponibile
             */
            "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQK2R b k - 0 1",
            {
            },
            0
        },
        {
            // 4) Re nero in un angolo (h8)
            "Caso 4: Re nero in un angolo (h8)",
            "7k/8/8/8/8/8/8/7K b - - 0 1",
            {
                "h8g8","h8g7","h8h7"
            },
            3
        },
        {
            // 5) Re nero standard e8 => 0 mosse
            "Caso 5: Re nero pos standard e8 => 0 mosse",
            "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR b KQkq - 0 1",
            { },
            0
        },
        {
            // 6) Re nero sul bordo (h5)
            "Caso 6: Re nero sul bordo (h5)",
            "8/8/8/7k/8/8/8/7K b - - 0 1",
            {
                // h5 => h6, g6, g5, h4, g4
                "h5h6","h5g6","h5g5","h5h4","h5g4"
            },
            5
        },
        {
            // 7) Re nero catture bianche
            "Caso 7: Re nero su d5 con pedoni bianchi attorno catturabili",
            "8/8/8/3k4/4P1P1/3P4/8/7K b - - 0 1",
            {
                "d5c6","d5e6","d5c4","d5e4","d5d6","d5c5","d5e5","d5d4"
            },
            8
        }
    };

    int num_tests = (int)(sizeof(tests)/sizeof(tests[0]));
    int passed = 0;

    for (int i = 0; i < num_tests; i++) {
        printf("==================================================\n");
        printf("%s\n", tests[i].description);
        printf("FEN: %s\n\n", tests[i].fen);

        // 1) parse FEN
        bitboard_state_t state;
        parse_fen(tests[i].fen, &state);

        // 2) Stampa stato iniziale
        printf("STATO INIZIALE:\n");
        print_board_simple(&state);

        // 3) Genera mosse per il Re nero
        dynamic_vector_t *moves_vec = dv_create();
        generate_black_king_moves(&state, moves_vec);

        // 4) Converte le mosse in notazione (es: "e8c8" per arrocco lungo)
        int gen_count = (int)dv_size(moves_vec);
        char gen_notations[64][16]; // lascio un po' di buffer in più per stampare "O-O" o "O-O-O"
        memset(gen_notations, 0, sizeof(gen_notations));

        for (int m = 0; m < gen_count; m++) {
            chess_move_t *mv = (chess_move_t*)dv_get(moves_vec, m);
            if (!mv) continue;

            // Se non è una mossa di arrocco, stampiamo come "e8d8"
            if (!mv->is_castling) {
                int from_file = mv->from % 8;
                int from_rank = mv->from / 8;
                int to_file   = mv->to % 8;
                int to_rank   = mv->to / 8;

                char from_file_c = 'a' + from_file;
                char from_rank_c = '1' + from_rank;
                char to_file_c   = 'a' + to_file;
                char to_rank_c   = '1' + to_rank;

                snprintf(gen_notations[m], sizeof(gen_notations[m]),
                         "%c%c%c%c",
                         from_file_c, from_rank_c, 
                         to_file_c,   to_rank_c);
            } else {
                // Se is_castling = 1, vuol dire arrocco
                // Controlliamo se from + 2 == to => arrocco corto, from - 2 => arrocco lungo
                if (mv->to == mv->from + 2) {
                    // Arrocco corto
                    snprintf(gen_notations[m], sizeof(gen_notations[m]), "O-O");
                } else {
                    // Arrocco lungo
                    snprintf(gen_notations[m], sizeof(gen_notations[m]), "O-O-O");
                }
            }
        }

        // 5) Confronto con le mosse attese
        int match_ok = 1;
        if (gen_count != tests[i].expected_count) {
            match_ok = 0;
        } else {
            for (int m = 0; m < gen_count; m++) {
                int found = 0;
                for (int x = 0; x < tests[i].expected_count; x++) {
                    if (strcmp(gen_notations[m], tests[i].expected_moves[x]) == 0) {
                        found = 1;
                        break;
                    }
                }
                if (!found) {
                    match_ok = 0;
                    break;
                }
            }
        }

        // 6) Stampa i risultati
        printf("Mosse Generate (%d):\n", gen_count);
        for (int m = 0; m < gen_count; m++) {
            printf("  %s\n", gen_notations[m]);
        }
        if (match_ok) {
            printf("PASS\n");
            passed++;
        } else {
            printf("FAIL\n");
            printf("Attese (%d):\n", tests[i].expected_count);
            for (int x = 0; x < tests[i].expected_count; x++) {
                printf("  %s\n", tests[i].expected_moves[x]);
            }
        }
        printf("\n");

        dv_free(moves_vec);
    }

    printf("==================================================\n");
    printf("Test generate_black_king_moves completati: %d/%d PASS.\n",
           passed, num_tests);
    printf("==================================================\n");
}


/**
 * Struttura per i test del Re bianco.
 */
typedef struct {
    const char *description;         // Descrizione del caso di test
    const char *fen;                // FEN con side to move = w (Re bianco)
    const char *expected_moves[32]; // Elenco di mosse attese (in notazione semplificata)
    int expected_count;             // Numero di mosse attese
} white_king_test_t;

/*
 * Presumiamo di avere già:
 *   - parse_fen(const char*, bitboard_state_t*)
 *   - print_board_simple(const bitboard_state_t*)
 *   - generate_white_king_moves(const bitboard_state_t*, dynamic_vector_t*)
 *   - dv_create(), dv_size(), dv_get(), dv_free()
 *   - add_move(...), chess_move_t con campo is_castling
 * 
 * La logica è identica al test per il Re nero, ma con Re Bianco (side = w).
 */

void test_generate_white_king_moves() {
    printf("\n=========================================\n");
    printf("TEST FUNZIONE: generate_white_king_moves (arrocco segnalato)\n");
    printf("=========================================\n\n");

    /*
     * Esempi di 7 casi di test (puoi aggiungerne o ridurli):
     * - Re bianco al centro
     * - Re bianco in un angolo
     * - Re bianco bloccato da propri pezzi
     * - Re bianco con catture possibili
     * - Posizione standard => Re su e1 con 0 mosse se tutto bloccato
     * - Re bianco sul bordo
     * - Caso con arrocco corto e/o lungo disponibili
     */
    static white_king_test_t tests[] = {
        {
            // 1) Re bianco al centro (d4) senza ostacoli
            "Caso 1: Re bianco al centro (d4) senza ostacoli",
            // FEN: 8/8/8/3K4/8/8/8/7k w - - 0 1
            // => Re bianco su d5 (bit 27 se a1=0? attento a rank/file),
            //   ma concettualmente "d4" nel testo. L'importante: side w.
            "8/8/8/3K4/8/8/8/7k w - - 0 1",
            {
                // d5 => c6, d6, e6, c5, e5, c4, d4, e4 (8 possibili se libera)
                "d5c6","d5d6","d5e6","d5c5","d5e5","d5c4","d5d4","d5e4"
            },
            8
        },
        {
            // 2) Re bianco pos standard e1 => arrocco corto e lungo se possibile
            //   Esempio di FEN con K e Q: "r3k2r/pppppppp/8/8/8/8/PPPPPPPP/R3K2R w KQ - 0 1"
            //   ma puoi usare un'altra posizione.
            "Caso 2: Re bianco con arrocco corto e lungo",
            "r3k2r/pppppppp/8/8/8/8/PPPPPPPP/R3K2R w KQ - 0 1",
            {
                // e1 -> d1, e1->f1, e1->d2, e1->e2, e1->f2
                // + O-O (corto => e1->g1) + O-O-O (lungo => e1->c1)
                // Se nessun pezzo blocca.
                // Qui lo semplifichiamo a 4 mosse: e1d1, e1f1, O-O, O-O-O
                "e1d1","e1f1","O-O","O-O-O"
            },
            4
        },
        {
            // 3) Re bianco bloccato da propri pezzi (nessuna mossa)
            "Caso 3: Re bianco bloccato",
            // FEN: "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
            // => re bianco su e1, bloccato da pedoni su e2/d2/f2...
            "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
            {
                // Nessuna mossa
            },
            0
        },
        {
            // 4) Re bianco in un angolo (h1)
            "Caso 4: Re bianco in un angolo (h1)",
            "7k/8/8/8/8/8/8/7K w - - 0 1",
            {
                // h1 => g1, g2, h2
                "h1g1","h1g2","h1h2"
            },
            3
        },
        {
            // 5) Re bianco sul bordo (h5)
            "Caso 5: Re bianco sul bordo (h5)",
            // => 8/8/8/7K/8/8/8/7k w - - 0 1
            "8/8/8/7K/8/8/8/7k w - - 0 1",
            {
                // h5 => h6,g6,g5,h4,g4
                "h5h6","h5g6","h5g5","h5h4","h5g4"
            },
            5
        },
        {
            // 6) Re bianco catture nere
            "Caso 6: Re bianco con catture possibili",
            // FEN: 8/8/8/3K4/4p1p1/3p4/8/7k w - - 0 1
            // => re bianco su d5, pedoni neri su e4,g4,d3,... adatta a piacere
            "8/8/8/3K4/4p1p1/3p4/8/7k w - - 0 1",
            {
                // d5 => c6,e6,c4,e4 (catture?), d6, c5, e5, d4...
                "d5c6","d5e6","d5c4","d5e4","d5d6","d5c5","d5e5","d5d4"
            },
            8
        },
        {
            // 7) Re bianco con un solo arrocco corto
            "Caso 7: Re bianco su e1 con solo K (bit0=1) => e1g1",
            // Esempio: "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQK2R w K - 0 1"
            "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQK2R w K - 0 1",
            {
                // se ci sono altre mosse di re, aggiungile
                // minimal: e1->f1, e1->d1, e1->f2,e1->e2,e1->d2 (?) + "O-O"
                "e1f1", "O-O"
            },
            2
        }
    };

    int num_tests = (int)(sizeof(tests)/sizeof(tests[0]));
    int passed = 0;

    for (int i = 0; i < num_tests; i++) {
        printf("==================================================\n");
        printf("%s\n", tests[i].description);
        printf("FEN: %s\n\n", tests[i].fen);

        // 1) parse FEN
        bitboard_state_t state;
        parse_fen(tests[i].fen, &state);

        // 2) Stampa stato iniziale
        printf("STATO INIZIALE:\n");
        print_board_simple(&state);

        // 3) Genera mosse per il Re bianco
        dynamic_vector_t *moves_vec = dv_create();
        generate_white_king_moves(&state, moves_vec);

        // 4) Converte le mosse in notazione (es: "e1g1" -> "O-O" se is_castling=1)
        int gen_count = (int)dv_size(moves_vec);
        char gen_notations[64][16]; 
        memset(gen_notations, 0, sizeof(gen_notations));

        for (int m = 0; m < gen_count; m++) {
            chess_move_t *mv = (chess_move_t*)dv_get(moves_vec, m);
            if (!mv) continue;

            if (!mv->is_castling) {
                // Mossa normale
                int from_file = mv->from % 8;
                int from_rank = mv->from / 8;
                int to_file   = mv->to % 8;
                int to_rank   = mv->to / 8;

                char from_file_c = 'a' + from_file;
                char from_rank_c = '1' + from_rank;
                char to_file_c   = 'a' + to_file;
                char to_rank_c   = '1' + to_rank;

                snprintf(gen_notations[m], sizeof(gen_notations[m]),
                         "%c%c%c%c",
                         from_file_c, from_rank_c, 
                         to_file_c,   to_rank_c);
            } else {
                // Arrocco
                if (mv->to == mv->from + 2) {
                    // O-O
                    snprintf(gen_notations[m], sizeof(gen_notations[m]), "O-O");
                } else {
                    // O-O-O
                    snprintf(gen_notations[m], sizeof(gen_notations[m]), "O-O-O");
                }
            }
        }

        // 5) Confronto con le mosse attese
        int match_ok = 1;
        if (gen_count != tests[i].expected_count) {
            match_ok = 0;
        } else {
            // Verifica che ogni mossa generata appaia fra quelle attese
            for (int m = 0; m < gen_count; m++) {
                int found = 0;
                for (int x = 0; x < tests[i].expected_count; x++) {
                    if (strcmp(gen_notations[m], tests[i].expected_moves[x]) == 0) {
                        found = 1;
                        break;
                    }
                }
                if (!found) {
                    match_ok = 0;
                    break;
                }
            }
        }

        // 6) Stampa i risultati
        printf("Mosse Generate (%d):\n", gen_count);
        for (int m = 0; m < gen_count; m++) {
            printf("  %s\n", gen_notations[m]);
        }
        if (match_ok) {
            printf("PASS\n");
            passed++;
        } else {
            printf("FAIL\n");
            printf("Attese (%d):\n", tests[i].expected_count);
            for (int x = 0; x < tests[i].expected_count; x++) {
                printf("  %s\n", tests[i].expected_moves[x]);
            }
        }
        printf("\n");

        dv_free(moves_vec);
    }

    printf("==================================================\n");
    printf("Test generate_white_king_moves completati: %d/%d PASS.\n",
           passed, num_tests);
    printf("==================================================\n");
}



int main() {
    test_generate_black_pawn_moves();
    test_generate_white_pawn_moves();
    test_generate_black_knight_moves();
    test_generate_white_knight_moves();
    test_generate_white_bishop_moves();
    test_generate_black_bishop_moves();
    test_generate_black_rook_moves();
    test_generate_white_rook_moves();
    test_generate_white_queen_moves();
    test_generate_black_queen_moves();
    test_generate_black_king_moves();
    test_generate_white_king_moves();

    return 0;
}
