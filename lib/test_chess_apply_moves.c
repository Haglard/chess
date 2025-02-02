#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "chess_state.h"
#include "chess_moves.h"
#include "chess_game_dynamics.h"
#include "obj_trace.h"

#define TEST_BREAKPOINTS

// Numero di test case
#define NUM_TESTS 2

// ============================================================================
// VETTORI GLOBALI (stati iniziali, mosse, stati finali attesi)
// ============================================================================
#define g_init_state1  { \
    .white_pawns       = 0x000000000000FF00ULL, \
    .white_knights     = 0x0000000000000042ULL, \
    .white_bishops     = 0x0000000000000024ULL, \
    .white_rooks       = 0x0000000000000081ULL, \
    .white_queens      = 0x0000000000000008ULL, \
    .white_kings       = 0x0000000000000010ULL, \
    .black_pawns       = 0x00FF000000000000ULL, \
    .black_knights     = 0x4200000000000000ULL, \
    .black_bishops     = 0x2400000000000000ULL, \
    .black_rooks       = 0x8100000000000000ULL, \
    .black_queens      = 0x0800000000000000ULL, \
    .black_kings       = 0x1000000000000000ULL, \
    .castling_rights   = 0xF, \
    .en_passant        = 255, \
    .halfmove_clock    = 0, \
    .fullmove_number   = 1, \
    .current_player    = 1  /* Bianco muove */ \
}

#define g_init_state2  { \
    .white_pawns       = 0x000000000000FF00ULL, \
    .white_knights     = 0x0000000000000042ULL, \
    .white_bishops     = 0x0000000000000024ULL, \
    .white_rooks       = 0x0000000000000081ULL, \
    .white_queens      = 0x0000000000000008ULL, \
    .white_kings       = 0x0000000000000010ULL, \
    .black_pawns       = 0x00FF000000000000ULL, \
    .black_knights     = 0x4200000000000000ULL, \
    .black_bishops     = 0x2400000000000000ULL, \
    .black_rooks       = 0x8100000000000000ULL, \
    .black_queens      = 0x0800000000000000ULL, \
    .black_kings       = 0x1000000000000000ULL, \
    .castling_rights   = 0xF, \
    .en_passant        = 255, \
    .halfmove_clock    = 0, \
    .fullmove_number   = 1, \
    .current_player    = -1 /* Nero muove */ \
}

static bitboard_state_t g_test_init_states[NUM_TESTS] = {
    g_init_state1,
    g_init_state2
};

// Mossa #1: Bianco pedone b2(9)->b3(17)
#define g_move1  { \
    .from = 9, \
    .to   = 17, \
    .promotion    = 0, \
    .is_castling  = 0, \
    .is_en_passant= 0 \
}
// Mossa #2: Nero pedone g7(54)->g6(46)
#define g_move2  { \
    .from = 54, \
    .to   = 46, \
    .promotion    = 0, \
    .is_castling  = 0, \
    .is_en_passant= 0 \
}

static chess_move_t g_test_moves[NUM_TESTS] = {
    g_move1,
    g_move2
};

// Stato finale #1 (dopo b2->b3)
#define g_final_state1  { \
    .white_pawns       = 0x000000000002FD00ULL, \
    .white_knights     = 0x0000000000000042ULL, \
    .white_bishops     = 0x0000000000000024ULL, \
    .white_rooks       = 0x0000000000000081ULL, \
    .white_queens      = 0x0000000000000008ULL, \
    .white_kings       = 0x0000000000000010ULL, \
    .black_pawns       = 0x00FF000000000000ULL, \
    .black_knights     = 0x4200000000000000ULL, \
    .black_bishops     = 0x2400000000000000ULL, \
    .black_rooks       = 0x8100000000000000ULL, \
    .black_queens      = 0x0800000000000000ULL, \
    .black_kings       = 0x1000000000000000ULL, \
    .castling_rights   = 0xF, \
    .en_passant        = 255, \
    .halfmove_clock    = 0, \
    .fullmove_number   = 1, \
    .current_player    = -1 \
}

// Stato finale #2 (dopo g7->g6)
#define g_final_state2  { \
    .white_pawns       = 0x000000000000FF00ULL, \
    .white_knights     = 0x0000000000000042ULL, \
    .white_bishops     = 0x0000000000000024ULL, \
    .white_rooks       = 0x0000000000000081ULL, \
    .white_queens      = 0x0000000000000008ULL, \
    .white_kings       = 0x0000000000000010ULL, \
    .black_pawns       = 0x00BF400000000000ULL, \
    .black_knights     = 0x4200000000000000ULL, \
    .black_bishops     = 0x2400000000000000ULL, \
    .black_rooks       = 0x8100000000000000ULL, \
    .black_queens      = 0x0800000000000000ULL, \
    .black_kings       = 0x1000000000000000ULL, \
    .castling_rights   = 0xF, \
    .en_passant        = 255, \
    .halfmove_clock    = 0, \
    .fullmove_number   = 2, \
    .current_player    = 1 \
}

static bitboard_state_t g_test_expected[NUM_TESTS] = {
    g_final_state1,
    g_final_state2
};

// =============================================================================
// Funzione di confronto bitboard_state
// =============================================================================
static bool compare_bitboard_states(const bitboard_state_t *s1, const bitboard_state_t *s2)
{
    if (s1->white_pawns   != s2->white_pawns)   return false;
    if (s1->white_knights != s2->white_knights) return false;
    if (s1->white_bishops != s2->white_bishops) return false;
    if (s1->white_rooks   != s2->white_rooks)   return false;
    if (s1->white_queens  != s2->white_queens)  return false;
    if (s1->white_kings   != s2->white_kings)   return false;

    if (s1->black_pawns   != s2->black_pawns)   return false;
    if (s1->black_knights != s2->black_knights) return false;
    if (s1->black_bishops != s2->black_bishops) return false;
    if (s1->black_rooks   != s2->black_rooks)   return false;
    if (s1->black_queens  != s2->black_queens)  return false;
    if (s1->black_kings   != s2->black_kings)   return false;

    if (s1->castling_rights != s2->castling_rights) return false;
    if (s1->en_passant      != s2->en_passant)      return false;
    if (s1->halfmove_clock  != s2->halfmove_clock)  return false;
    if (s1->fullmove_number != s2->fullmove_number) return false;
    if (s1->current_player  != s2->current_player)  return false;

    return true;
}

// =============================================================================
// Funzione di supporto: stampare in debug (TRACE_DEBUG) i bitboard in esadecimale
// =============================================================================
static void debug_print_state_hex(const char *title, const bitboard_state_t *s)
{
    TRACE_DEBUG(&stdtrace, "=== %s (stampa esadecimale) ===", title);

    TRACE_DEBUG(&stdtrace, "white_pawns   = 0x%016llX", (unsigned long long)s->white_pawns);
    TRACE_DEBUG(&stdtrace, "white_knights = 0x%016llX", (unsigned long long)s->white_knights);
    TRACE_DEBUG(&stdtrace, "white_bishops = 0x%016llX", (unsigned long long)s->white_bishops);
    TRACE_DEBUG(&stdtrace, "white_rooks   = 0x%016llX", (unsigned long long)s->white_rooks);
    TRACE_DEBUG(&stdtrace, "white_queens  = 0x%016llX", (unsigned long long)s->white_queens);
    TRACE_DEBUG(&stdtrace, "white_kings   = 0x%016llX", (unsigned long long)s->white_kings);

    TRACE_DEBUG(&stdtrace, "black_pawns   = 0x%016llX", (unsigned long long)s->black_pawns);
    TRACE_DEBUG(&stdtrace, "black_knights = 0x%016llX", (unsigned long long)s->black_knights);
    TRACE_DEBUG(&stdtrace, "black_bishops = 0x%016llX", (unsigned long long)s->black_bishops);
    TRACE_DEBUG(&stdtrace, "black_rooks   = 0x%016llX", (unsigned long long)s->black_rooks);
    TRACE_DEBUG(&stdtrace, "black_queens  = 0x%016llX", (unsigned long long)s->black_queens);
    TRACE_DEBUG(&stdtrace, "black_kings   = 0x%016llX", (unsigned long long)s->black_kings);

    TRACE_DEBUG(&stdtrace, "castling_rights= 0x%02X", s->castling_rights);
    TRACE_DEBUG(&stdtrace, "en_passant     = %d", s->en_passant);
    TRACE_DEBUG(&stdtrace, "halfmove_clock = %d", s->halfmove_clock);
    TRACE_DEBUG(&stdtrace, "fullmove_number= %d", s->fullmove_number);
    TRACE_DEBUG(&stdtrace, "current_player = %d", s->current_player);
}

// =============================================================================
// MAIN DI TEST
// =============================================================================
int main(void)
{
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

    for (int i = 0; i < NUM_TESTS; i++)
    {
        printf("\n=== TEST CASE #%d ===\n", i+1);

        bitboard_state_t *pInit   = &g_test_init_states[i];
        chess_move_t     *pMove   = &g_test_moves[i];
        bitboard_state_t *pExpect = &g_test_expected[i];

        // 1) Stampa stato iniziale (forma grafica + debug in hex)
        printf("[Stato iniziale]\n");
        print_board(pInit);
        debug_print_state_hex("Stato Iniziale", pInit);

        // 2) Stampa la mossa
        printf("\n[Mossa da applicare] from=%d, to=%d, promotion=%d, castling=%d, en_passant=%d\n",
               pMove->from, pMove->to, pMove->promotion, pMove->is_castling, pMove->is_en_passant);

        // 3) Stampa lo stato atteso (forma grafica + debug in hex)
        printf("\n[Stato atteso]\n");
        print_board(pExpect);
        debug_print_state_hex("Stato Atteso", pExpect);

        // 4) Esegui la apply move
        void *new_state_void = chess_apply_move(pInit, pMove);

        // 5) Se la mossa è illegale
        if (!new_state_void) {
            printf("\nRisultato: Mossa ILLEGALE (chess_apply_move = NULL)\n");

#if defined(TEST_BREAKPOINTS)
            while (true) {
                printf("\nDigita 'y' per continuare o 'q' per uscire: ");
                int c = getchar();
                while (getchar() != '\n') {}
                if (c == 'y' || c == 'Y') {
                    break;
                } else if (c == 'q' || c == 'Q') {
                    printf("Uscita dal programma di test.\n");
                    return 0;
                }
            }
#endif
            continue;
        }

        // Cast a bitboard_state_t*
        bitboard_state_t *pResult = (bitboard_state_t*)new_state_void;

        // 6) Stampa lo stato ottenuto (forma grafica + debug in hex)
        printf("\n[Stato ottenuto]\n");
        print_board(pResult);
        debug_print_state_hex("Stato Ottenuto", pResult);

        // 7) Confronto con l’atteso
        bool match = compare_bitboard_states(pResult, pExpect);
        if (match) {
            printf("\nESITO: PASS => Lo stato coincide con quello atteso.\n");
        } else {
            printf("\nESITO: FAIL => Lo stato NON coincide con quello atteso.\n");
        }

        // 8) Dealloca pResult
        chess_free_state(pResult);

#if defined(TEST_BREAKPOINTS)
        while (true) {
            printf("\nDigita 'y' per continuare o 'q' per uscire: ");
            int c = getchar();
            while (getchar() != '\n') {}
            if (c == 'y' || c == 'Y') {
                break;
            } else if (c == 'q' || c == 'Q') {
                printf("Uscita dal programma di test.\n");
                return 0;
            }
        }
#endif
    }

    printf("\n=== Fine test ===\n");
    return 0;
}
