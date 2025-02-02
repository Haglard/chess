/**
 * @file forza4.c
 * @brief Gioco Forza 4 (Connect Four) con MiniMax e cache.
 *
 * ##VERSION## "forza4.c 1.1"
 *
 * Umano = 'X' (player=1), Computer = 'O' (player=-1).
 * Board di 6 righe × 7 colonne.
 * Algoritmo MiniMax con alpha-beta pruning e transposition table (cache).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <limits.h>

#include "minimax.h"            // Implementazione MiniMax
#include "obj_cache.h"          // Transposition table
#include "obj_dynamic_vector.h" // Vettore dinamico
#include "obj_trace.h"          // Logging
#include "obj_mem.h"            // Debug memoria

#define C4_ROWS 6
#define C4_COLS 7
#define C4_SIZE (C4_ROWS * C4_COLS)

typedef struct {
    int board[C4_SIZE];
    int next_player;   /* 1=umano(X), -1=computer(O) */
} C4_State;

typedef int C4_Move;

#define AT(st, r, c)  ((st)->board[(r)*C4_COLS + (c)])

/* Funzione di hashing per la cache (FNV-1a) */
static uint64_t c4_hash_state(const void *state) {
    const C4_State *st = (const C4_State*) state;
    uint64_t h = 14695981039346656037ULL; // FNV offset basis
    for (int i = 0; i < C4_SIZE; i++) {
        h ^= (st->board[i] + 2);
        h *= 1099511628211ULL; // FNV prime
    }
    h ^= (st->next_player & 0x0F);
    h *= 1099511628211ULL;
    return h;
}

/* Funzione di confronto per la cache */
static int c4_equals_state(const void *stateA, const void *stateB) {
    const C4_State *a = (const C4_State*) stateA;
    const C4_State *b = (const C4_State*) stateB;
    for (int i = 0; i < C4_SIZE; i++) {
        if (a->board[i] != b->board[i]) return 0;
    }
    return (a->next_player == b->next_player);
}

/* Funzione ausiliaria per valutare una finestra di quattro celle */
static int evaluate_window(int a, int b, int c, int d) {
    int window[4] = {a, b, c, d};
    int count_X = 0, count_O = 0;
    for (int i = 0; i < 4; i++) {
        if (window[i] == 1) count_X++;
        else if (window[i] == -1) count_O++;
    }
    int score = 0;
    if (count_X == 4) score += 100;
    else if (count_X == 3 && count_O == 0) score += 5;
    else if (count_X == 2 && count_O == 0) score += 2;
    
    if (count_O == 4) score -= 100;
    else if (count_O == 3 && count_X == 0) score -= 5;
    else if (count_O == 2 && count_X == 0) score -= 2;
    
    return score;
}

/* Stampa la board */
static void print_board(const C4_State *st) {
    printf("\n");
    for (int r = 0; r < C4_ROWS; r++) {
        printf("|");
        for (int c = 0; c < C4_COLS; c++) {
            char ch = ' ';
            if (AT(st, r, c) == 1) ch = 'X';
            else if (AT(st, r, c) == -1) ch = 'O';
            printf("%c|", ch);
        }
        printf("\n");
    }
    printf(" 0 1 2 3 4 5 6  (colonne)\n\n");
}

/* Verifica se una colonna è piena */
static int is_col_full(const C4_State *st, int c) {
    return (AT(st, 0, c) != 0);
}

/* Trova la riga libera più bassa in una colonna */
static int find_free_row(const C4_State *st, int c) {
    for (int r = C4_ROWS-1; r >= 0; r--) {
        if (AT(st, r, c) == 0) return r;
    }
    return -1;
}

/* Controlla se c'è un vincitore */
static int check_winner(const C4_State *st) {
    // Orizzontale
    for (int r=0; r<C4_ROWS; r++) {
        for (int c=0; c<C4_COLS-3; c++) {
            int sum = AT(st,r,c) + AT(st,r,c+1) + AT(st,r,c+2) + AT(st,r,c+3);
            if (sum == 4) return 1;
            if (sum == -4) return -1;
        }
    }
    // Verticale
    for (int r=0; r<C4_ROWS-3; r++) {
        for (int c=0; c<C4_COLS; c++) {
            int sum = AT(st,r,c) + AT(st,r+1,c) + AT(st,r+2,c) + AT(st,r+3,c);
            if (sum == 4) return 1;
            if (sum == -4) return -1;
        }
    }
    // Diagonale discendente
    for (int r=0; r<C4_ROWS-3; r++) {
        for (int c=0; c<C4_COLS-3; c++) {
            int sum = AT(st,r,c) + AT(st,r+1,c+1) + AT(st,r+2,c+2) + AT(st,r+3,c+3);
            if (sum == 4) return 1;
            if (sum == -4) return -1;
        }
    }
    // Diagonale ascendente
    for (int r=3; r<C4_ROWS; r++) {
        for (int c=0; c<C4_COLS-3; c++) {
            int sum = AT(st,r,c) + AT(st,r-1,c+1) + AT(st,r-2,c+2) + AT(st,r-3,c+3);
            if (sum == 4) return 1;
            if (sum == -4) return -1;
        }
    }
    return 0;
}

/* Callback: copia profonda dello stato */
static void* c4_copy_state(const void *state) {
    const C4_State *st = (const C4_State*) state;
    C4_State *copy = (C4_State*) malloc(sizeof(C4_State));
    if (copy) {
        memcpy(copy->board, st->board, C4_SIZE*sizeof(int));
        copy->next_player = st->next_player;
    }
    return copy;
}

/* Callback: libera lo stato */
static void c4_free_state(void *state) {
    free(state);
}

/* Callback: genera mosse disponibili */
static dynamic_vector_t* c4_get_moves(const void *state) {
    const C4_State *st = (const C4_State*) state;
    dynamic_vector_t *moves = dv_create();
    if (!moves) return NULL;

    for (int c=0; c<C4_COLS; c++) {
        if (!is_col_full(st, c)) {
            int *m = (int*) malloc(sizeof(int));
            if (!m) continue;
            *m = c;
            dv_push_back(moves, m);
        }
    }
    return moves;
}

/* Callback: libera le mosse */
static void c4_free_moves(dynamic_vector_t *moves_vec) {
    if (!moves_vec) return;
    int n = dv_size(moves_vec);
    for (int i=0; i<n; i++) {
        int *colPtr = (int*) dv_get(moves_vec, i);
        free(colPtr);
    }
    dv_free(moves_vec);
}

/* Callback: ottiene il numero di mosse */
static int c4_get_num_moves(const dynamic_vector_t *moves_vec) {
    return dv_size(moves_vec);
}

/* Callback: ottiene una mossa specifica */
static void* c4_get_move_at(const dynamic_vector_t *moves_vec, int index) {
    return dv_get(moves_vec, index);
}

/* Callback: copia profonda di una mossa */
static void* c4_copy_move(const void *move) {
    const int *m = (const int*) move;
    int *copy = (int*) malloc(sizeof(int));
    if (copy) *copy = *m;
    return copy;
}

/* Callback: libera una mossa */
static void c4_free_move(void *move) {
    free(move);
}

/* Callback: applica una mossa */
static void* c4_apply_move(const void *state, const void *move) {
    const C4_State *st = (const C4_State*) state;
    const int *col = (const int*) move;

    C4_State *new_st = (C4_State*) malloc(sizeof(C4_State));
    if (!new_st) return NULL;

    memcpy(new_st->board, st->board, C4_SIZE*sizeof(int));
    new_st->next_player = -st->next_player;

    int r = find_free_row(st, *col);
    if (r >= 0) {
        new_st->board[r*C4_COLS + (*col)] = st->next_player;
    }
    return new_st;
}

/* Callback: verifica se lo stato è terminale */
static int c4_is_terminal(const void *state) {
    const C4_State *st = (const C4_State*) state;
    int w = check_winner(st);
    if (w != 0) return 1;
    for (int c=0; c<C4_COLS; c++) {
        if (!is_col_full(st, c)) return 0;
    }
    return 1;
}

/* Callback: valuta lo stato */
static int c4_evaluate(const void *state) {
    const C4_State *st = (const C4_State*) state;
    int score = 0;
    int w = check_winner(st);
    
    if (w == 1) return +100;
    if (w == -1) return -100;
    
    // Orizzontale
    for (int r = 0; r < C4_ROWS; r++) {
        for (int c = 0; c < C4_COLS - 3; c++) {
            score += evaluate_window(AT(st, r, c), AT(st, r, c+1), AT(st, r, c+2), AT(st, r, c+3));
        }
    }
    
    // Verticale
    for (int c = 0; c < C4_COLS; c++) {
        for (int r = 0; r < C4_ROWS - 3; r++) {
            score += evaluate_window(AT(st, r, c), AT(st, r+1, c), AT(st, r+2, c), AT(st, r+3, c));
        }
    }
    
    // Diagonale Discendente
    for (int r = 0; r < C4_ROWS - 3; r++) {
        for (int c = 0; c < C4_COLS - 3; c++) {
            score += evaluate_window(AT(st, r, c), AT(st, r+1, c+1), AT(st, r+2, c+2), AT(st, r+3, c+3));
        }
    }
    
    // Diagonale Ascendente
    for (int r = 3; r < C4_ROWS; r++) {
        for (int c = 0; c < C4_COLS - 3; c++) {
            score += evaluate_window(AT(st, r, c), AT(st, r-1, c+1), AT(st, r-2, c+2), AT(st, r-3, c+3));
        }
    }
    
    // Preferenza per il centro
    int center_col = C4_COLS / 2;
    for (int r = 0; r < C4_ROWS; r++) {
        if (AT(st, r, center_col) == 1) score += 3;
        else if (AT(st, r, center_col) == -1) score -= 3;
    }
    
    return score;
}

/* Callback: ottiene il giocatore corrente */
static int c4_player_to_move(const void *state) {
    const C4_State *st = (const C4_State*) state;
    return st->next_player;
}

int main(void) {
    trace_set_channel_output(&stdtrace, stderr);
    trace_set_channel_level(&stdtrace, TRACE_LEVEL_INFO);

    game_descriptor_t gd;
    memset(&gd, 0, sizeof(gd));

    gd.copy_state    = c4_copy_state;
    gd.free_state    = c4_free_state;
    gd.get_moves     = c4_get_moves;
    gd.free_moves    = c4_free_moves;
    gd.get_num_moves = c4_get_num_moves;
    gd.get_move_at   = c4_get_move_at;
    gd.copy_move     = c4_copy_move;
    gd.free_move     = c4_free_move;
    gd.apply_move    = c4_apply_move;
    gd.is_terminal   = c4_is_terminal;
    gd.evaluate      = c4_evaluate;
    gd.player_to_move= c4_player_to_move;
    gd.hash_state    = c4_hash_state;    // Usa la cache
    gd.equals_state  = c4_equals_state;

    /* Creazione della cache */
    generic_hash_table_t *cache = cache_create(gd.hash_state, gd.equals_state);
    if (!cache) {
        fprintf(stderr, "Avviso: impossibile creare la cache, si prosegue senza.\n");
    }

    /* Stato iniziale: board vuota, next_player=1 */
    C4_State *initial = (C4_State*) malloc(sizeof(C4_State));
    if (!initial) {
        fprintf(stderr, "Errore: impossibile allocare lo stato iniziale.\n");
        return 1;
    }
    for (int i=0; i<C4_SIZE; i++) {
        initial->board[i] = 0;
    }
    initial->next_player = 1; // X umano

    printf("===== FORZA 4 (MiniMax con Cache) =====\n");
    printf(" Umano = X (1), Computer = O (-1)\n");
    printf(" Inizia l'umano (X).\n");

    C4_State *current = initial;
    while (!c4_is_terminal(current)) {
        print_board(current);

        int p = c4_player_to_move(current);
        if (p == 1) {
            /* Turno Umano */
            int col;
            do {
                printf("Scegli colonna (0..6): ");
                if (scanf("%d", &col) != 1) {
                    printf("Input non valido! Inserisci un numero da 0 a 6.\n");
                    int c;
                    while ((c = getchar()) != '\n' && c != EOF);
                    col = -1;
                    continue;
                }
                if (col < 0 || col >= C4_COLS || is_col_full(current, col)) {
                    printf("Mossa non valida!\n");
                    col = -1;
                }
            } while (col == -1);

            C4_State *next_st = (C4_State*) c4_apply_move(current, &col);
            if (!next_st) {
                printf("Errore nell'applicare la mossa. Termino il gioco.\n");
                break;
            }
            c4_free_state(current);
            current = next_st;
        } else {
            /* Turno Computer */
            printf("Il computer (O) sta pensando...\n");
            void *best = get_best_move(&gd, current, 7, cache); // Usa la cache
            if (!best) {
                printf("Nessuna mossa trovata per il computer!\n");
                break;
            }
            int c = *(int*) best;
            gd.free_move(best);

            C4_State *next_st = (C4_State*) c4_apply_move(current, &c);
            if (!next_st) {
                printf("Errore nell'applicare la mossa del computer. Termino il gioco.\n");
                break;
            }
            c4_free_state(current);
            current = next_st;
        }
    }

    /* Fine partita */
    print_board(current);
    int w = check_winner(current);
    if (w == 1) {
        printf("Hai vinto! Congratulazioni.\n");
    } else if (w == -1) {
        printf("Ha vinto il computer!\n");
    } else {
        printf("Pareggio! Board piena.\n");
    }

    /* Pulizia finale */
    c4_free_state(current);

    if (cache) {
        cache_destroy(cache);
    }

    printf("Grazie per aver giocato a Forza 4!\n");
    return 0;
}
