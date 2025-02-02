/**
 * @file test_minimax.c
 * @brief Esempio di unit test per il package MiniMax (minimax.h).
 *
 * ##VERSION## "test_minimax.c 1.0"
 *
 * Il test crea un game_descriptor_t fittizio con callback minime,
 * e invoca minimax_ab e get_best_move con stati di esempio.
 * Eventuali funzioni reali di un gioco andranno sostituite ai "stub".
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "minimax.h"
#include "obj_mem.h"
#include "obj_trace.h"
#include "obj_cache.h"         // se vuoi testare la transposition table
#include "obj_dynamic_vector.h" // per testare la gestione delle mosse

/* -------------------------------------------------------------------------
 * Sezione: callback "fake" di esempio
 * -------------------------------------------------------------------------
 * Queste funzioni sono STUB: fanno qualcosa di molto semplice, giusto per test.
 */

/* STATO DI ESEMPIO: supponiamo che lo stato sia solo un int* che indica "valore".
   In un progetto reale, avresti una struct più complessa. */
static void* fake_copy_state(const void *state) {
    const int *orig = (const int*)state;
    int *copy = (int*) malloc(sizeof(int));
    if (copy) {
        *copy = *orig;
    }
    return copy;
}

static void fake_free_state(void *state) {
    if (state) free(state);
}

/* Mosse di ESEMPIO: supponiamo che la mossa sia anch'essa un int* */
static dynamic_vector_t* fake_get_moves(const void *state) {
    const int *val = (const int*) state;
    /* Esempio fittizio:
       Se *val >= 5 => nessuna mossa (stato considerato "quasi terminale").
       Altrimenti generiamo 2 mosse (move = +1, move = +2) */
    if (*val >= 5) {
        return dv_create(); // nessuna mossa
    }
    dynamic_vector_t *dv = dv_create();
    if (!dv) return NULL;

    /* Mossa 1: +1 */
    int *m1 = (int*) malloc(sizeof(int));
    *m1 = 1;
    dv_push_back(dv, m1);

    /* Mossa 2: +2 */
    int *m2 = (int*) malloc(sizeof(int));
    *m2 = 2;
    dv_push_back(dv, m2);

    return dv;
}

static void fake_free_moves(dynamic_vector_t *moves_vec) {
    if (!moves_vec) return;
    /* Dobbiamo liberare ogni mossa prima di dv_free */
    int num = dv_size(moves_vec);
    for (int i=0; i<num; i++) {
        int *m = (int*) dv_get(moves_vec, i);
        if (m) free(m);
    }
    dv_free(moves_vec);
}

static int fake_get_num_moves(const dynamic_vector_t *moves_vec) {
    if (!moves_vec) return 0;
    return dv_size(moves_vec);
}

static void* fake_get_move_at(const dynamic_vector_t *moves_vec, int index) {
    if (!moves_vec) return NULL;
    if (index < 0 || index >= (int) dv_size(moves_vec)) return NULL;
    return dv_get(moves_vec, index);
}

/* Copia e liberazione di una singola mossa (int*) */
static void* fake_copy_move(const void *move) {
    const int *pm = (const int*) move;
    int *copy = (int*) malloc(sizeof(int));
    if (copy) {
        *copy = *pm;
    }
    return copy;
}

static void fake_free_move(void *move) {
    if (move) free(move);
}

/* Applica la mossa: stato + mossa => nuovo stato */
static void* fake_apply_move(const void *state, const void *move) {
    const int *s = (const int*) state;
    const int *m = (const int*) move;
    int *new_s = (int*) malloc(sizeof(int));
    if (new_s) {
        *new_s = (*s) + (*m);
    }
    return new_s;
}

static int fake_is_terminal(const void *state) {
    const int *s = (const int*) state;
    /* Consideriamo terminale se *s >= 10 */
    return (*s >= 10) ? 1 : 0;
}

/* Evaluate: punteggio = *s, fittizio */
static int fake_evaluate(const void *state) {
    const int *s = (const int*) state;
    return *s; 
}

/* Chi muove? Semplice: se *s è pari => player=1, se dispari => player=-1 */
static int fake_player_to_move(const void *state) {
    const int *s = (const int*) state;
    return ((*s)%2 == 0) ? 1 : -1;
}

/* Hash e equals dello stato (int*) */
static uint64_t fake_hash_state(const void *state) {
    if (!state) return 0;
    uint64_t val = *(const int*)state;
    /* Leggera manipolazione */
    val = (val << 3) ^ (val >> 2) ^ 0xABCDEF;
    return val;
}

static int fake_equals_state(const void *stateA, const void *stateB) {
    if (!stateA || !stateB) return 0;
    return (*(const int*)stateA == *(const int*)stateB) ? 1 : 0;
}

/* -------------------------------------------------------------------------
 * MAIN di TEST
 * -------------------------------------------------------------------------
 */

int main(void) {
    /* Inizializzazione del logging e del debug memory (se necessario) */
    trace_set_channel_output(&stdtrace, stderr);
    trace_set_channel_level(&stdtrace, TRACE_LEVEL_DEBUG);

    /* Creiamo un game_descriptor_t con le callback di test */
    game_descriptor_t gd;
    memset(&gd, 0, sizeof(gd));

    gd.copy_state    = fake_copy_state;
    gd.free_state    = fake_free_state;
    gd.get_moves     = fake_get_moves;
    gd.free_moves    = fake_free_moves;
    gd.get_num_moves = fake_get_num_moves;
    gd.get_move_at   = fake_get_move_at;
    gd.copy_move     = fake_copy_move;
    gd.free_move     = fake_free_move;
    gd.apply_move    = fake_apply_move;
    gd.is_terminal   = fake_is_terminal;
    gd.evaluate      = fake_evaluate;
    gd.player_to_move= fake_player_to_move;
    gd.hash_state    = fake_hash_state;
    gd.equals_state  = fake_equals_state;

    printf("=== Test MiniMax con callback di test ===\n");

    /* 1) Stato iniziale = 0 (player=1) */
    int *init_state = (int*) malloc(sizeof(int));
    *init_state = 0;

    /* 2) Chiamiamo minimax_ab */
    printf("\n--- Esempio: minimax_ab(init=0, depth=5) ---\n");
    int alpha = INT_MIN;
    int beta  = INT_MAX;
    int value = minimax_ab(&gd, init_state, 5, alpha, beta, NULL /* no cache */);
    printf("minimax_ab => valore=%d\n", value);

    /* 3) get_best_move */
    printf("\n--- Esempio: get_best_move(init=0, depth=5) ---\n");
    void *best = get_best_move(&gd, init_state, 5, NULL);
    if (best) {
        int best_int = *(int*)best;
        printf("get_best_move => mossa=%d\n", best_int);
        gd.free_move(best);
    } else {
        printf("Nessuna mossa disponibile!\n");
    }

    /* 4) Stato terminale: es. *s=10 */
    *init_state = 10;
    printf("\n--- Esempio: stato terminale (10) ---\n");
    int valueTerm = minimax_ab(&gd, init_state, 5, INT_MIN, INT_MAX, NULL);
    printf("minimax_ab su stato=10 => valore=%d\n", valueTerm);

    void *bestTerm = get_best_move(&gd, init_state, 5, NULL);
    if (bestTerm) {
        printf("ERRORE: get_best_move su stato terminale non deve restituire mosse.\n");
        gd.free_move(bestTerm);
    } else {
        printf("get_best_move => NULL, come atteso (stato terminale).\n");
    }

    /* 5) Prova con la cache (transposition table), se vuoi */
    printf("\n--- Esempio con cache_create, se vuoi testare la transposition table ---\n");
    generic_hash_table_t *cache = cache_create(fake_hash_state, fake_equals_state);
    if (cache) {
        *init_state = 0;
        int valCache = minimax_ab(&gd, init_state, 5, INT_MIN, INT_MAX, cache);
        printf("minimax_ab con cache => valore=%d\n", valCache);

        cache_destroy(cache);
        printf("Cache distrutta.\n");
    }

    /* Pulizia finale */
    free(init_state);

    dump_allocated_memory();
    dump_allocated_memory_hex();

    printf("\n=== Fine test MiniMax ===\n");
    return 0;
}
