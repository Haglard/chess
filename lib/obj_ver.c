// Questo file è generato automaticamente dal Makefile
/*
 * ##VERSION## "obj_ver.c 1.344"
 */
#include "obj_ver.h"
#include <stdio.h>

const char *module_versions[] = {
    "obj_trace.c 1.0",
    "obj_ver.c 1.344",
    "obj_mem.c 1.0",
    "obj_cache.c 1.1",
    "obj_dynamic_vector.c 1.0",
    "minimax.c 1.1",
    "chess_game_descriptor.h 1.0",
    "chess_hash.h 1.0",
    "chess_moves.h 1.0",
    "minimax.h 1.0",
    "obj_cache.h 1.1",
    "obj_dynamic_vector.h 1.0",
    "obj_mem.h 1.1",
    "obj_trace.h 1.0",
    "obj_ver.h 1.0",
    "obj_ver.c 1.344",
    NULL
};

void print_versions(void) {
    printf("=== Moduli e Versioni ===\n");
    for (int i = 0; module_versions[i] != NULL; i++) {
        printf("%s\n", module_versions[i]);
    }
    printf("=========================\n");
}
