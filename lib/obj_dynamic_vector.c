/**
 * ##VERSION## "obj_dynamic_vector.c 1.0"
*/

#include "obj_mem.h"
#include "obj_trace.h"
#include "obj_dynamic_vector.h"
#include <stdlib.h>  /* per malloc, free, realloc */
#include <string.h>  /* per memset, se necessario */

/* 
 * Costanti configurabili:
 * - INITIAL_CAPACITY: capacità iniziale (in questo caso 3).
 * - CAPACITY_INCREMENT: quanto aggiungere a ogni esaurimento dello spazio (in questo caso 20).
 */
#define INITIAL_CAPACITY 128
#define CAPACITY_INCREMENT 128

/*
 * Struttura interna del vettore dinamico.
 * Contiene:
 *   - data:     array di puntatori a void*
 *   - size:     numero di elementi attualmente memorizzati
 *   - capacity: dimensione dell'array data (numero max di elementi senza riallocare)
 */
struct dynamic_vector_s {
    void   **data;    /* array di void* */
    size_t   size;    /* quanti elementi attualmente in data */
    size_t   capacity;/* dimensione massima di data prima della riallocazione */
};

dynamic_vector_t* dv_create(void) {
    /* Alloca la struttura principale */
    dynamic_vector_t *dv = (dynamic_vector_t*) malloc(sizeof(*dv));
    if (!dv) {
        return NULL;  /* fallimento allocazione */
    }

    dv->size = 0;
    dv->capacity = INITIAL_CAPACITY;

    /* Alloca data con capacity iniziale */
    dv->data = (void**) malloc(dv->capacity * sizeof(void*));
    if (!dv->data) {
        free(dv);
        return NULL;
    }

    return dv;
}

void dv_free(dynamic_vector_t *dv) {
    if (!dv) return;

    /* Libera l'array di puntatori */
    if (dv->data) {
        free(dv->data);
    }

    /* Libera la struttura */
    free(dv);
}

size_t dv_size(const dynamic_vector_t *dv) {
    if (!dv) return 0;
    return dv->size;
}

void* dv_get(const dynamic_vector_t *dv, size_t index) {
    if (!dv) return NULL;
    /* Se index è fuori dal range, potrebbe causare accessi non validi */
    if (index >= dv->size) {
        return NULL; 
    }
    return dv->data[index];
}

void dv_set(dynamic_vector_t *dv, size_t index, void *value) {
    if (!dv) return;
    if (index < dv->size) {
        dv->data[index] = value;
    }
}

/*
 * Funzione interna che verifica se serve aumentare la capacità.
 * Se size >= capacity, incrementiamo la capacità di CAPACITY_INCREMENT unità.
 * Ritorna 0 se OK, -1 se fallisce la realloc.
 */
static int dv_resize_if_needed(dynamic_vector_t *dv) {
    /* Se la size è minore di capacity, non serve ridimensionare */
    if (dv->size < dv->capacity) {
        return 0; 
    }

    /* Aumentiamo la capacità di CAPACITY_INCREMENT */
    size_t new_capacity = dv->capacity + CAPACITY_INCREMENT;
    void **new_data = (void**) realloc(dv->data, new_capacity * sizeof(void*));
    if (!new_data) {
        /* fallimento */
        return -1;
    }

    /* Aggiorniamo data e capacity */
    dv->data = new_data;
    dv->capacity = new_capacity;
    return 0;
}

int dv_push_back(dynamic_vector_t *dv, void *value) {
    if (!dv) return -1;

    /* Verifichiamo se serve ridimensionare */
    if (dv_resize_if_needed(dv) != 0) {
        return -1; /* fallimento dell'allocazione */
    }

    /* Inseriamo il nuovo elemento in coda */
    dv->data[dv->size] = value;
    dv->size += 1;

    return 0;
}
