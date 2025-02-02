#include "obj_mem.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>    // per INT_MIN, INT_MAX
#include "minimax.h"   // Libreria MiniMax (deve essere già implementata)
#include "obj_dynamic_vector.h"  // Per dynamic_vector_t e relative funzioni

/****************************************************************************
 * RAPPRESENTAZIONE DELLO STATO DI TRIS (TIC-TAC-TOE)
 ****************************************************************************/

#define BOARD_SIZE 9  /* 3x3 = 9 caselle */

/* 
 * Valori possibili in board[i]:
 *   0 = casella vuota
 *   1 = 'X'
 *  -1 = 'O'
 *
 * next_player indica chi giocherà:
 *   1  = tocca a 'X' (umano in questo esempio)
 *  -1  = tocca a 'O' (computer)
 */
typedef struct {
    int board[BOARD_SIZE];
    int next_player;
} TTT_State;

/* 
 * MOSSA: una semplice posizione da 0..8, dove pos % 3 è la colonna,
 * pos / 3 è la riga.
 */
typedef int TTT_Move;

/*****************************************************************************
 * FUNZIONI DI SUPPORTO PER LA GESTIONE DELLA GRIGLIA
 *****************************************************************************/

/**
 * @brief Stampa la griglia di Tris su console.
 */
static void print_board(const TTT_State *st) {
    printf("\n");
    for (int r = 0; r < 3; r++) {
        for (int c = 0; c < 3; c++) {
            int pos = r * 3 + c;
            char ch;
            if (st->board[pos] == 1) ch = 'X';
            else if (st->board[pos] == -1) ch = 'O';
            else ch = ' ';
            printf(" %c ", ch);
            if (c < 2) printf("|");
        }
        printf("\n");
        if (r < 2) {
            printf("---+---+---\n");
        }
    }
    printf("\n");
}

/**
 * @brief Controlla se c'è un vincitore (1 = X vince, -1 = O vince) o se è pareggio / in corso.
 * @return 1 se X ha vinto, -1 se O ha vinto, 0 se nessuno ha ancora vinto.
 */
static int check_winner(const TTT_State *st) {
    int (*b)[3] = (int (*)[3]) st->board;  /* reinterpret board come [3][3] */

    /* Controllo righe */
    for (int r = 0; r < 3; r++) {
        int sum = b[r][0] + b[r][1] + b[r][2];
        if (sum == 3) return 1;
        if (sum == -3) return -1;
    }
    /* Controllo colonne */
    for (int c = 0; c < 3; c++) {
        int sum = b[0][c] + b[1][c] + b[2][c];
        if (sum == 3) return 1;
        if (sum == -3) return -1;
    }
    /* Controllo diagonali */
    {
        int sum1 = b[0][0] + b[1][1] + b[2][2];
        int sum2 = b[0][2] + b[1][1] + b[2][0];
        if (sum1 == 3 || sum2 == 3) return 1;
        if (sum1 == -3 || sum2 == -3) return -1;
    }
    return 0; // nessun vincitore
}

/*****************************************************************************
 * CALLBACK PER MINIMAX (game_descriptor_t)
 ****************************************************************************/

/**
 * @brief Copia profonda dello stato (TTT_State).
 */
static void* ttt_copy_state(const void *state) {
    const TTT_State *st = (const TTT_State*) state;
    TTT_State *copy = (TTT_State*) malloc(sizeof(TTT_State));
    if (copy) {
        memcpy(copy->board, st->board, BOARD_SIZE * sizeof(int));
        copy->next_player = st->next_player;
    }
    return copy;
}

/**
 * @brief Libera lo stato.
 */
static void ttt_free_state(void *state) {
    free(state);
}

/**
 * @brief Genera tutte le possibili mosse (posizioni libere).
 * @return dynamic_vector_t* con mosse (int*).
 */
static dynamic_vector_t* ttt_get_moves(const void *state) {
    const TTT_State *st = (const TTT_State*) state;
    dynamic_vector_t *moves = dv_create();
    if (!moves) return NULL;

    /* Aggiunge tutte le posizioni vuote come mosse */
    for (int i = 0; i < BOARD_SIZE; i++) {
        if (st->board[i] == 0) {
            int *mv = (int*) malloc(sizeof(int));
            *mv = i;
            dv_push_back(moves, mv);
        }
    }
    return moves;
}

static void ttt_free_moves(dynamic_vector_t *moves_vec) {
    if (!moves_vec) return;
    int n = dv_size(moves_vec);
    for (int i = 0; i < n; i++) {
        int *m = (int*) dv_get(moves_vec, i);
        free(m);
    }
    dv_free(moves_vec);
}

static int ttt_get_num_moves(const dynamic_vector_t *moves_vec) {
    return dv_size(moves_vec);
}

static void* ttt_get_move_at(const dynamic_vector_t *moves_vec, int index) {
    return dv_get(moves_vec, index);
}

/* Copia e free di una singola mossa (int*) */
static void* ttt_copy_move(const void *move) {
    const int *m = (const int*) move;
    int *copy = (int*) malloc(sizeof(int));
    *copy = *m;
    return copy;
}

static void ttt_free_move(void *move) {
    free(move);
}

/**
 * @brief Applica la mossa: pos = (int)*move => st->board[pos] = st->next_player
 */
static void* ttt_apply_move(const void *state, const void *move) {
    const TTT_State *st = (const TTT_State*) state;
    const int *m = (const int*) move;

    TTT_State *new_st = (TTT_State*) malloc(sizeof(TTT_State));
    if (!new_st) return NULL;

    memcpy(new_st->board, st->board, BOARD_SIZE * sizeof(int));
    new_st->next_player = -st->next_player; // cambia giocatore

    new_st->board[*m] = st->next_player; // piazza 'X' o 'O'
    return new_st;
}

/**
 * @brief is_terminal: se c'è un vincitore o la board è piena.
 */
static int ttt_is_terminal(const void *state) {
    const TTT_State *st = (const TTT_State*) state;
    int w = check_winner(st);
    if (w != 0) return 1;  // X o O hanno vinto

    /* Se ci sono caselle vuote => non terminale */
    for (int i = 0; i < BOARD_SIZE; i++) {
        if (st->board[i] == 0) return 0;
    }
    /* Altrimenti pareggio => consideriamo terminale */
    return 1;
}

/**
 * @brief evaluate: +100 se X vince, -100 se O vince, 0 se in corso/pareggio.
 *
 * Oppure un'evoluzione più “fine” se vuoi.
 */
static int ttt_evaluate(const void *state) {
    const TTT_State *st = (const TTT_State*) state;
    int w = check_winner(st);
    if (w == 1) return +100;  // X vince
    if (w == -1) return -100; // O vince
    return 0; // pareggio / in corso => 0
}

/* 
 * @brief player_to_move: st->next_player
 */
static int ttt_player_to_move(const void *state) {
    const TTT_State *st = (const TTT_State*) state;
    return st->next_player;
}

/* 
 * @brief Hash e Equals per la cache (opzionale).
 * Se non vuoi la cache, puoi lasciare callback vuoti o non settarli.
 */
static uint64_t ttt_hash_state(const void *state) {
    const TTT_State *st = (const TTT_State*) state;
    /* Semplice “hash”: sommo con uno shift */
    uint64_t h = 0;
    for (int i = 0; i < BOARD_SIZE; i++) {
        h = (h << 3) ^ (st->board[i] + 2); // sposta e mischia
    }
    /* next_player in coda */
    h ^= (st->next_player & 0xF);
    return h;
}

static int ttt_equals_state(const void *stateA, const void *stateB) {
    const TTT_State *a = (const TTT_State*) stateA;
    const TTT_State *b = (const TTT_State*) stateB;
    for (int i = 0; i < BOARD_SIZE; i++) {
        if (a->board[i] != b->board[i]) return 0;
    }
    if (a->next_player != b->next_player) return 0;
    return 1;
}

/*****************************************************************************
 * MAIN: Partita con interfaccia testuale
 *****************************************************************************/
int main(void) {
    /* Inizializza debug trace se vuoi */
    trace_set_channel_output(&stdtrace, stderr);
    trace_set_channel_level(&stdtrace, TRACE_LEVEL_DEBUG); // o DEBUG

    /* Costruiamo il game_descriptor_t */
    game_descriptor_t gd;
    memset(&gd, 0, sizeof(gd));

    gd.copy_state    = ttt_copy_state;
    gd.free_state    = ttt_free_state;
    gd.get_moves     = ttt_get_moves;
    gd.free_moves    = ttt_free_moves;
    gd.get_num_moves = ttt_get_num_moves;
    gd.get_move_at   = ttt_get_move_at;
    gd.copy_move     = ttt_copy_move;
    gd.free_move     = ttt_free_move;
    gd.apply_move    = ttt_apply_move;
    gd.is_terminal   = ttt_is_terminal;
    gd.evaluate      = ttt_evaluate;
    gd.player_to_move= ttt_player_to_move;
    gd.hash_state    = ttt_hash_state;   // se vuoi la cache
    gd.equals_state  = ttt_equals_state; // se vuoi la cache

    /* Stato iniziale: board vuota, next_player=1 ('X' umano) */
    TTT_State *initial = (TTT_State*) malloc(sizeof(TTT_State));
    for (int i = 0; i < BOARD_SIZE; i++) {
        initial->board[i] = 0;
    }
    initial->next_player = 1; // 'X' (umano) inizia

    printf("========== BENVENUTO A TRIS (Tic-tac-toe) ========== \n");
    printf("Uomo = X, Computer = O. Inizia l'uomo.\n\n");

    /* Ciclo di gioco */
    TTT_State *current = initial;
    while (!ttt_is_terminal(current)) {
        print_board(current);

        int p = ttt_player_to_move(current);
        if (p == 1) {
            /* Turno UOMO */
            int pos;
            do {
                printf("Inserisci una posizione (0..8): ");
                scanf("%d", &pos);
                if (pos < 0 || pos > 8 || current->board[pos] != 0) {
                    printf("Mossa non valida!\n");
                    pos = -1; // ripeti
                }
            } while (pos == -1);

            /* Applica la mossa */
            TTT_State *next_st = (TTT_State*) ttt_apply_move(current, &pos);
            ttt_free_state(current);
            current = next_st;
        } else {
            /* Turno COMPUTER */
            printf("Il computer sta pensando...\n");
            /* Per la profondità, scegli un valore (3..9) => Tic-tac-toe non è grande */
            void *best_move = get_best_move(&gd, current, 9, NULL);
            if (!best_move) {
                /* Nessuna mossa => terminale (?) => break*/
                printf("Nessuna mossa per il computer!\n");
                break;
            } else {
                TTT_State *next_st = (TTT_State*) ttt_apply_move(current, best_move);
                gd.free_move(best_move);
                ttt_free_state(current);
                current = next_st;
            }
        }
    }

    /* Fine partita */
    print_board(current);
    int w = check_winner(current);
    if (w == 1) {
        printf("Hai vinto! Complimenti.\n");
    } else if (w == -1) {
        printf("Ha vinto il computer!\n");
    } else {
        printf("Pareggio!\n");
    }

    /* Pulizia */
    dump_allocated_memory();
    dump_allocated_memory_hex();
    printf("sto per pulire la memoria\n");
    ttt_free_state(current);
    dump_allocated_memory();
    dump_allocated_memory_hex();
    printf("sto per liberare initial\n");
//    free(initial); // se non già liberato
    dump_allocated_memory();
    dump_allocated_memory_hex();
    printf("Grazie per aver giocato!\n");

    return 0;
}
