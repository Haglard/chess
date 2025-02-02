#include <stdio.h>
#include <stdlib.h>

#include "chess_state.h"             // per bitboard_state_t, print_board, initialize_board, ...
#include "chess_game_descriptor.h"    // per initialize_chess_game_descriptor()
#include "chess_game_dynamics.h"      // per chess_is_terminal, chess_apply_move, ...
#include "minimax.h"                  // per get_best_move
#include "obj_trace.h"                // per stdtrace e trace_open_file_channel
#include "obj_cache.h"                // per cache_create, cache_lookup, cache_store, ecc.

int main(void)
{
    // 1) Inizializza il canale di logging stdtrace per scrivere su chess_test.log
    if (trace_open_file_channel(&stdtrace, "stdtrace", "chess_test.log", TRACE_LEVEL_DEBUG, true))
    {
        TRACE_INFO(&stdtrace, "stdtrace is now logging to 'chess_test.log' at DEBUG level");
    }
    else
    {
        fprintf(stderr, "Impossibile aprire il file di log chess_test.log\n");
    }

    // 2) Alloca dinamicamente lo stato corrente, quindi inizializza la scacchiera standard
    bitboard_state_t *current_state = (bitboard_state_t*)malloc(sizeof(bitboard_state_t));
    if (!current_state) {
        TRACE_FATAL(&stdtrace, "Errore allocazione current_state");
        return 1;
    }
    initialize_board(current_state);

    // 3) Inizializza il descrittore di gioco scacchistico (con callback minimax)
    game_descriptor_t *gd = initialize_chess_game_descriptor();
    if (!gd) {
        TRACE_FATAL(&stdtrace, "Errore: impossibile inizializzare il descrittore di gioco.");
        free(current_state);
        return 1;
    }

    // 4) Creiamo la cache per memorizzare gli stati già analizzati dal minimax
    //    Usiamo le callback gd->hash_state e gd->equals_state come parametri di hash/ugualianza.
    TRACE_DEBUG(&stdtrace, "[MAIN] Creo la cache per il minimax...");
    generic_hash_table_t *my_cache = cache_create(gd->hash_state, gd->equals_state);
    if (!my_cache) {
        TRACE_FATAL(&stdtrace, "Errore: impossibile creare la cache hash.");
        // Libera le risorse e termina
        free(current_state);
        // free(gd); // se necessario
        return 1;
    }

    // 5) Parametri di profondità per la ricerca
    int depth = 5;

    // 6) Ciclo di gioco finché la partita non è terminata
    while (1) {
        // Stampa lo stato attuale
        printf("\n=== Stato Attuale ===\n");
        print_board(current_state);

        // Verifica se è terminale
        if (chess_is_terminal(current_state)) {
            printf("\nPartita terminata!\n");
            break;
        }

        // 6.1) Trova la mossa migliore con get_best_move + cache
        TRACE_DEBUG(&stdtrace, "[MAIN] Invoco get_best_move (depth=%d) con la cache...", depth);
//        void *best_move = get_best_move(gd, current_state, depth, my_cache);
        void *best_move = get_best_move(gd, current_state, depth, NULL);
        TRACE_DEBUG(&stdtrace, "[MAIN] get_best_move ha restituito best_move=%p", best_move);

        if (!best_move) {
            // Nessuna mossa => stato terminale
            printf("Nessuna mossa disponibile => Fine.\n");
            break;
        }

        // Stampa la mossa in notazione semplice
        chess_move_t *mv = (chess_move_t*)best_move;
        int from_file = mv->from % 8;
        int from_rank = mv->from / 8;
        int to_file   = mv->to % 8;
        int to_rank   = mv->to / 8;

        char from_file_c = 'a' + from_file;
        char from_rank_c = '1' + from_rank;
        char to_file_c   = 'a' + to_file;
        char to_rank_c   = '1' + to_rank;

        printf("\nMossa scelta da %s: %c%c -> %c%c",
               (current_state->current_player == 1) ? "Bianco" : "Nero",
               from_file_c, from_rank_c, to_file_c, to_rank_c);

        // (Arrocco, promozione, e.p.)
        if (mv->is_castling) {
            if (mv->to == mv->from + 2) {
                printf(" (O-O)");
            } else {
                printf(" (O-O-O)");
            }
        }
        if (mv->promotion) {
            char promoChar = '?';
            switch (mv->promotion) {
                case 1: promoChar = 'N'; break;
                case 2: promoChar = 'B'; break;
                case 3: promoChar = 'R'; break;
                case 4: promoChar = 'Q'; break;
            }
            printf("= %c", promoChar);
        }
        if (mv->is_en_passant) {
            printf(" e.p.");
        }
        printf("\n");

        // 6.2) Applica la mossa
        void *new_state_v = chess_apply_move(current_state, mv);
        gd->free_move(best_move); // libera la mossa

        if (!new_state_v) {
            printf("ERRORE: mossa illegale => esco.\n");
            break;
        }

        // Rimpiazziamo current_state
        chess_free_state(current_state);
        current_state = (bitboard_state_t*)new_state_v;

        // 6.3) Attendi input utente
        while (1) {
            printf("\nDigita 'y' per continuare, 'q' per uscire: ");
            int c = getchar();
            while (getchar() != '\n') {}
            if (c == 'y' || c == 'Y') {
                break;
            } else if (c == 'q' || c == 'Q') {
                printf("Uscita dal programma.\n");
                goto end_game;
            }
        }
    }

end_game:
    // 7) Pulizia finale
    printf("\n=== Fine della partita ===\n");
    if (my_cache) {
        TRACE_DEBUG(&stdtrace, "[MAIN] Distruggo la cache hash...");
        cache_destroy(my_cache);
    }
    if (current_state) {
        TRACE_DEBUG(&stdtrace, "[MAIN] Distruggo current_state...");
        chess_free_state(current_state);
    }
    // if (gd) free(gd); // se necessario

    // trace_close_channel(&stdtrace); // se vuoi chiudere file di log

    return 0;
}
