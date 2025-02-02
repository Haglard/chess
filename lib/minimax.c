/**
 * ##VERSION## "minimax.c 1.1"
 */

 /******************************************************************************
  * File: minimax.c
  * 
  * Implementazione dell'algoritmo Minimax con alpha-beta pruning, con supporto
  * a una cache generica (transposition table) e a un vettore dinamico di mosse.
  ******************************************************************************/

#include "minimax.h"
#include "obj_trace.h"      // Inclusione della libreria di tracing
#include <stdlib.h>         // per malloc, free
#include <string.h>         // per eventuali operazioni di copia in get_best_move
#include <limits.h>         // per INT_MIN, INT_MAX

/******************************************************************************
 * Struttura memorizzata nella cache per ogni stato.
 * Contiene il valore valutato, la profondità e il tipo di nodo.
 ******************************************************************************/
typedef enum {
    EXACT,
    LOWER_BOUND,
    UPPER_BOUND
} NodeType;

typedef struct {
    int value;      /* Valore Minimax calcolato per lo stato */
    int depth;      /* Profondità alla quale è stato calcolato */
    NodeType type;  /* Tipo di valore (EXACT, LOWER_BOUND, UPPER_BOUND) */
} minimax_cache_entry_t;

/******************************************************************************
 * Funzione: minimax_ab
 ******************************************************************************/
int minimax_ab(
    const game_descriptor_t *gd,
    const void *state,
    int depth,
    int alpha,
    int beta,
    void *cache_handle
) {
    int alpha_orig = alpha;
    int beta_orig = beta;

    /* 1. Se abbiamo una cache, proviamo a recuperare un valore memorizzato */
    if (cache_handle != NULL) {
        minimax_cache_entry_t *cached_val = (minimax_cache_entry_t*) cache_lookup(cache_handle, state);
        if (cached_val != NULL && cached_val->depth >= depth) {
            TRACE_DEBUG(&stdtrace, "Cache hit at depth %d: value=%d, type=%d", depth, cached_val->value, cached_val->type);
            if (cached_val->type == EXACT) {
                return cached_val->value;
            } else if (cached_val->type == LOWER_BOUND) {
                if (cached_val->value > alpha) {
                    alpha = cached_val->value;
                }
            } else if (cached_val->type == UPPER_BOUND) {
                if (cached_val->value < beta) {
                    beta = cached_val->value;
                }
            }
            if (alpha >= beta) {
                TRACE_DEBUG(&stdtrace, "Alpha-beta cutoff after cache retrieval: alpha=%d, beta=%d", alpha, beta);
                return cached_val->value;
            }
        } else {
            TRACE_DEBUG(&stdtrace, "Cache miss at depth %d", depth);
        }
    }

    /* 2. Se lo stato è terminale o abbiamo raggiunto la profondità massima, valuta e ritorna. */
    if (gd->is_terminal(state) || depth == 0) {
        int eval = gd->evaluate(state);

        /* Memorizza in cache, se disponibile */
        if (cache_handle != NULL) {
            minimax_cache_entry_t entry;
            entry.value = eval;
            entry.depth = depth;
            entry.type = EXACT;
            cache_store(cache_handle, state, &entry);
            TRACE_DEBUG(&stdtrace, "Stored in cache at depth %d: value=%d, type=EXACT", depth, eval);
        }
        return eval;
    }

    /* 3. Otteniamo il vettore dinamico di mosse disponibili */
    dynamic_vector_t *moves_vec = gd->get_moves(state);
    int num_moves = gd->get_num_moves(moves_vec);

    /* Se non ci sono mosse, consideriamo lo stato come terminale di fatto */
    if (num_moves == 0) {
        int eval = gd->evaluate(state);
        if (cache_handle != NULL) {
            minimax_cache_entry_t entry;
            entry.value = eval;
            entry.depth = depth;
            entry.type = EXACT;
            cache_store(cache_handle, state, &entry);
            TRACE_DEBUG(&stdtrace, "Stored in cache at depth %d: value=%d, type=EXACT (no available moves)", depth, eval);
        }
        /* Non dimentichiamo di liberare il vettore */
        gd->free_moves(moves_vec);
        return eval;
    }

    /* 4. Determiniamo se il giocatore è massimizzatore (1) o minimizzatore (-1) */
    int player = gd->player_to_move(state);
    int best_value = (player == 1) ? INT_MIN : INT_MAX;

    /* 5. Cicliamo sulle mosse */
    for (int i = 0; i < num_moves; i++) {
        /* Otteniamo la mossa i-esima */
        void *move = gd->get_move_at(moves_vec, i);

        /* Applichiamo la mossa per ottenere un nuovo stato */
        void *new_state = gd->apply_move(state, move);
        if (new_state == NULL) {
            TRACE_ERROR(&stdtrace, "Failed to apply move at index %d", i);
            continue; /* Salta questa mossa in caso di errore */
        }

        /* Ricorsione */
        int value = minimax_ab(gd, new_state, depth - 1, alpha, beta, cache_handle);

        /* Libera lo stato generato */
        gd->free_state(new_state);

        /* Aggiorna best_value e alpha/beta in base al tipo di giocatore */
        if (player == 1) {
            /* Massimizzatore */
            if (value > best_value) {
                best_value = value;
                /* Libera eventuale best_move precedente */
                if (best_value != INT_MIN) { /* Condizione per evitare free(NULL) */
                    /* Nota: best_move è gestito nella funzione get_best_move, non qui */
                }
                TRACE_DEBUG(&stdtrace, "New best_value for MAX: %d", best_value);
            }
            if (best_value > alpha) {
                alpha = best_value;
                TRACE_DEBUG(&stdtrace, "Updated alpha to %d", alpha);
            }
        } else {
            /* Minimizatore */
            if (value < best_value) {
                best_value = value;
                /* Libera eventuale best_move precedente */
                if (best_value != INT_MAX) { /* Condizione per evitare free(NULL) */
                    /* Nota: best_move è gestito nella funzione get_best_move, non qui */
                }
                TRACE_DEBUG(&stdtrace, "New best_value for MIN: %d", best_value);
            }
            if (best_value < beta) {
                beta = best_value;
                TRACE_DEBUG(&stdtrace, "Updated beta to %d", beta);
            }
        }

        /* Potatura */
        if (alpha >= beta) {
            TRACE_DEBUG(&stdtrace, "Alpha-beta cutoff: alpha=%d, beta=%d", alpha, beta);
            break; /* esci dal ciclo: non serve analizzare altre mosse */
        }
    }

    /* 6. Libera il vettore di mosse */
    gd->free_moves(moves_vec);

    /* 7. Determina il tipo di nodo per la cache */
    NodeType node_type;
    if (best_value <= alpha_orig) {
        node_type = UPPER_BOUND;
    } else if (best_value >= beta_orig) {
        node_type = LOWER_BOUND;
    } else {
        node_type = EXACT;
    }

    /* 8. Memorizza in cache il risultato, se possibile */
    if (cache_handle != NULL) {
        minimax_cache_entry_t entry;
        entry.value = best_value;
        entry.depth = depth;
        entry.type = node_type;
        cache_store(cache_handle, state, &entry);
        TRACE_DEBUG(&stdtrace, "Stored in cache at depth %d: value=%d, type=%d", depth, best_value, node_type);
    }

    return best_value;
}

/******************************************************************************
 * Funzione: get_best_move
 ******************************************************************************/
void* get_best_move(
    const game_descriptor_t *gd,
    const void *state,
    int depth,
    void *cache_handle
) {
    /* Se lo stato è terminale, non ci sono mosse da restituire */
    if (gd->is_terminal(state)) {
        TRACE_INFO(&stdtrace, "get_best_move called on terminal state.");
        return NULL;
    }

    /* Otteniamo il vettore di mosse disponibili */
    dynamic_vector_t *moves_vec = gd->get_moves(state);
    int num_moves = gd->get_num_moves(moves_vec);

    if (num_moves == 0) {
        /* Nessuna mossa disponibile => stato terminale di fatto */
        TRACE_INFO(&stdtrace, "No available moves found in get_best_move.");
        gd->free_moves(moves_vec);
        return NULL;
    }

    /* Determiniamo il giocatore */
    int player = gd->player_to_move(state);

    /* Inizializziamo best_value e i parametri alpha/beta */
    int best_value = (player == 1) ? INT_MIN : INT_MAX;
    int alpha = INT_MIN;
    int beta  = INT_MAX;

    /* Puntatore alla miglior mossa trovata */
    void *best_move = NULL;

    /* Analizziamo ciascuna mossa */
    for (int i = 0; i < num_moves; i++) {
        /* Prendiamo la mossa i-esima */
        void *move = gd->get_move_at(moves_vec, i);

        /* Generiamo lo stato risultante */
        void *new_state = gd->apply_move(state, move);
        if (new_state == NULL) {
            TRACE_ERROR(&stdtrace, "Failed to apply move at index %d in get_best_move.", i);
            continue; /* Salta questa mossa in caso di errore */
        }

        /* Calcoliamo il valore con minimax */
        int value = minimax_ab(gd, new_state, depth - 1, alpha, beta, cache_handle);
        TRACE_INFO(&stdtrace, " - Move %d applied, minimax value=%d", i, value);

        /* Libera lo stato transitorio */
        gd->free_state(new_state);

        /* Controlliamo se è migliore per il giocatore in questione */
        if (player == 1) {
            /* Massimizzatore */
            if (value > best_value) {
                best_value = value;
                /* Libera eventuale best_move precedente */
                if (best_move) {
                    gd->free_move(best_move);
                }
                /* Copia profonda della mossa corrente */
                best_move = gd->copy_move(move);
                TRACE_DEBUG(&stdtrace, "New best_move for MAX: column=%d with value=%d", *(int*)move, best_value);
            }
            if (best_value > alpha) {
                alpha = best_value;
                TRACE_DEBUG(&stdtrace, "Updated alpha to %d in get_best_move", alpha);
            }
        } else {
            /* Minimizatore */
            if (value < best_value) {
                best_value = value;
                /* Libera eventuale best_move precedente */
                if (best_move) {
                    gd->free_move(best_move);
                }
                /* Copia profonda della mossa corrente */
                best_move = gd->copy_move(move);
                TRACE_DEBUG(&stdtrace, "New best_move for MIN: column=%d with value=%d", *(int*)move, best_value);
            }
            if (best_value < beta) {
                beta = best_value;
                TRACE_DEBUG(&stdtrace, "Updated beta to %d in get_best_move", beta);
            }
        }

        /* Potatura */
        if (alpha >= beta) {
            TRACE_DEBUG(&stdtrace, "Alpha-beta cutoff in get_best_move: alpha=%d, beta=%d", alpha, beta);
            break;
        }
    }

    /* Libera il vettore di mosse */
    gd->free_moves(moves_vec);

    /* Restituisce la migliore mossa trovata (o NULL se non ci sono) */
    if (best_move != NULL) {
        TRACE_INFO(&stdtrace, "Best move found: column=%d with value=%d", *(int*)best_move, best_value);
    } else {
        TRACE_WARN(&stdtrace, "No best move found.");
    }
    return best_move;
}
