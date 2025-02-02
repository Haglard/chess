#include "obj_trace.h"
#include "obj_mem.h"
#include "obj_ver.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#define NUM_BLOCKS 50   // Numero di blocchi ridotto per una maggiore chiarezza
#define NUM_SIZES 5     // Diversificazione limitata delle dimensioni

int main(void) {
    // Configurazione del tracing su stderr a livello DEBUG
    trace_set_channel_output(&stdtrace, stderr);
    trace_set_channel_level(&stdtrace, TRACE_LEVEL_DEBUG);

    TRACE_DEBUG(&stdtrace, "Inizio programma main");

    print_versions();

    srand(time(NULL));

    // Dimensioni ridotte per blocchi più leggibili
    size_t block_sizes[NUM_SIZES] = {0, 0, 0, 0, 1};
    void* blocks[NUM_BLOCKS];

    // Allocazione di NUM_BLOCKS oggetti con dimensioni variabili
    for (int i = 0; i < NUM_BLOCKS; i++) {
        size_t size = block_sizes[rand() % NUM_SIZES];
        blocks[i] = malloc(size);
        if (!blocks[i]) {
            TRACE_ERROR(&stdtrace, "malloc(%zu) ha restituito NULL", size);
        } else {
            memset(blocks[i], i % 256, size); // Riempie i blocchi con un valore ripetuto per migliorare il dump
            TRACE_DEBUG(&stdtrace, "Allocato blocco %d con dimensione %zu bytes", i, size);
        }
    }

    // Dump della memoria allocata
    TRACE_INFO(&stdtrace, "==== DUMP MEMORIA IN ESACODICE ====");
    dump_allocated_memory_hex();

    // Liberazione dei blocchi allocati
    for (int i = 0; i < NUM_BLOCKS; i++) {
        if (blocks[i]) {
            free(blocks[i]);
            TRACE_DEBUG(&stdtrace, "Liberato blocco %d", i);
        }
    }

    // Dump finale della memoria allocata
    TRACE_INFO(&stdtrace, "==== DUMP FINALE DELLA MEMORIA ====");
    dump_allocated_memory();

    TRACE_DEBUG(&stdtrace, "Fine programma main");
    return 0;
}
