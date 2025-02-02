/**
 * ##VERSION## "obj_mem.c 1.0"
*/

#define OBJ_MEM_C  /* Sono in OBJ_MEM_C */

#include "obj_mem.h"
#include "obj_trace.h"
#include "obj_trace.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#if defined(OBJ_MEM_DEBUG_ON)

typedef struct obj_mem_descriptor_s {
    struct obj_mem_descriptor_s *prev;
    struct obj_mem_descriptor_s *next;
    size_t size;
    const char *file;
    int line;
    time_t alloc_time;
} obj_mem_descriptor_t;

static obj_mem_descriptor_t *g_alloc_list_head = NULL;
static obj_mem_descriptor_t *g_alloc_list_tail = NULL;

static void insert_descriptor(obj_mem_descriptor_t *desc) {
    desc->prev = g_alloc_list_tail;
    desc->next = NULL;
    if (g_alloc_list_tail) {
        g_alloc_list_tail->next = desc;
    } else {
        g_alloc_list_head = desc;
    }
    g_alloc_list_tail = desc;
}

static void remove_descriptor(obj_mem_descriptor_t *desc) {
    if (desc->prev) {
        desc->prev->next = desc->next;
    } else {
        g_alloc_list_head = desc->next;
    }
    if (desc->next) {
        desc->next->prev = desc->prev;
    } else {
        g_alloc_list_tail = desc->prev;
    }
}

/* In modalità debug, obj_mem_malloc usa le malloc standard,
   poiché in questo file OBJ_MEM_INTERNAL è definito e quindi malloc non è ridefinita. */
void* obj_mem_malloc(size_t size, const char *file, int line) {
    size_t total_size = sizeof(obj_mem_descriptor_t) + size;
    unsigned char *mem = (unsigned char*)malloc(total_size);
    if (!mem) {
        TRACE_ERROR(&stdtrace, "obj_mem_malloc: malloc(%zu) fallita (file=%s line=%d)", total_size, file, line);
        return NULL;
    }

    obj_mem_descriptor_t *desc = (obj_mem_descriptor_t*)mem;
    desc->size = size;
    desc->file = file;
    desc->line = line;
    desc->alloc_time = time(NULL);

    insert_descriptor(desc);

    void *payload = (void*)(mem + sizeof(obj_mem_descriptor_t));
    TRACE_DEBUG(&stdtrace, "obj_mem_malloc: Allocato %zu bytes (file=%s line=%d) payload=0x%p", 
                size, file, line, payload);

    return payload;
}

/**
 * @brief Alloca memoria per un array di \p nmemb elementi, ciascuno di dimensione \p size,
 *        con tracciamento avanzato (sostituisce `calloc` in modalità debug).
 *
 * @param[in] nmemb Numero di elementi da allocare.
 * @param[in] size  Dimensione di ciascun elemento (in byte).
 * @param[in] file  Nome del file sorgente che richiama la \c calloc (usare \c __FILE__).
 * @param[in] line  Numero di linea del file sorgente (usare \c __LINE__).
 *
 * @return Puntatore alla memoria allocata (azzerata), oppure \c NULL in caso di errore.
 *
 * Il comportamento è analogo a \c calloc(nmemb, size):
 *  1. Viene calcolata la dimensione totale (\p nmemb * \p size).
 *  2. Si chiama \c obj_mem_malloc per tracciare l'allocazione e ottenere un blocco di memoria
 *     di dimensione totale.
 *  3. Se l'allocazione ha successo, si azzera l'intera zona di memoria (in modo da imitare
 *     esattamente il comportamento di \c calloc).
 *  4. Se l'allocazione non è riuscita, viene restituito \c NULL.
 */
void* obj_mem_calloc(size_t nmemb, size_t size, const char *file, int line)
{
    // 1) Calcola la dimensione totale in byte
    size_t total = nmemb * size;

    // 2) Richiama la nostra obj_mem_malloc con tracking di debug
    void* ptr = obj_mem_malloc(total, file, line);

    // 3) Se l'allocazione ha avuto successo, azzeriamo la zona di memoria
    if (ptr) {
        memset(ptr, 0, total);
    }

    // 4) Ritorniamo il puntatore (o NULL se la malloc è fallita)
    return ptr;
}

/**
 * @brief Ridimensiona un blocco di memoria precedentemente allocato con obj_mem_malloc,
 *        aggiornando il descrittore interno di debug.
 *
 * @param ptr       Puntatore al blocco precedentemente allocato (può essere NULL).
 * @param new_size  Nuova dimensione richiesta in byte.
 * @param file      Nome del file sorgente che richiama la realloc (usare __FILE__).
 * @param line      Numero di linea del file sorgente (usare __LINE__).
 * @return Puntatore al nuovo blocco di memoria, o NULL in caso di errore.
 *
 * Se \p ptr è NULL, il comportamento è equivalente a \c obj_mem_malloc(\p new_size, \p file, \p line).
 * Se \p new_size è 0, il comportamento è equivalente a \c obj_mem_free(\p ptr), e la funzione
 * restituisce NULL.
 */
void* obj_mem_realloc(void* ptr, size_t new_size, const char *file, int line)
{
    /* 1. Caso speciale: ptr == NULL => allocazione ex-novo */
    if (!ptr) {
        return obj_mem_malloc(new_size, file, line);
    }

    /* 2. Caso speciale: new_size == 0 => free del blocco esistente, ritorna NULL */
    if (new_size == 0) {
        obj_mem_free(ptr);
        return NULL;
    }

    /* 3. Caso generale: ridimensionamento del blocco esistente */
    unsigned char *old_mem = (unsigned char*)ptr;
    obj_mem_descriptor_t *old_desc = (obj_mem_descriptor_t*)(old_mem - sizeof(obj_mem_descriptor_t));

    /* Rimuoviamo momentaneamente il descrittore dalla lista */
    remove_descriptor(old_desc);

    /* Calcoliamo la nuova dimensione totale (descrittore + payload) */
    size_t total_size = sizeof(obj_mem_descriptor_t) + new_size;

    /* Ridimensioniamo */
    unsigned char *new_mem = (unsigned char*)realloc((void*)old_desc, total_size);
    if (!new_mem) {
        TRACE_ERROR(&stdtrace, "obj_mem_realloc: realloc(%zu) fallita (file=%s line=%d)",
                    total_size, file, line);
        /* Reinseriamo il vecchio descrittore nella lista, perché ancora valido */
        insert_descriptor(old_desc);
        return NULL;
    }

    /* Il blocco potrebbe essersi spostato: otteniamo il nuovo descrittore */
    obj_mem_descriptor_t *new_desc = (obj_mem_descriptor_t*)new_mem;
    /* Aggiorniamo le info di debug (dimensione, file, linea).
       Se vuoi mantenere l'ora di allocazione originale, NON sovrascrivere new_desc->alloc_time. */
    new_desc->size = new_size;
    new_desc->file = file;
    new_desc->line = line;
    /* new_desc->alloc_time = time(NULL); // Se vuoi aggiornare il timestamp, altrimenti commenta. */

    /* Reinseriamo il descrittore nella lista globale */
    insert_descriptor(new_desc);

    void *new_payload = (void*)(new_mem + sizeof(obj_mem_descriptor_t));

    TRACE_DEBUG(&stdtrace,
                "obj_mem_realloc: Realloc da %zu a %zu bytes (file=%s line=%d) old_payload=0x%p new_payload=0x%p",
                old_desc->size, new_size, file, line, ptr, new_payload);

    return new_payload;
}

void obj_mem_free(void* ptr) {
    if (!ptr) {
        TRACE_WARN(&stdtrace, "obj_mem_free: Tentativo di liberare un puntatore NULL");
        return;
    }

    unsigned char* mem = (unsigned char*)ptr;
    obj_mem_descriptor_t *desc = (obj_mem_descriptor_t*)(mem - sizeof(obj_mem_descriptor_t));

    TRACE_DEBUG(&stdtrace, "obj_mem_free: Liberato blocco (file=%s line=%d) payload=0x%p", 
                desc->file, desc->line, ptr);

    remove_descriptor(desc);
    free(desc);
}

void dump_allocated_memory(void) {
    size_t count = 0;
    size_t total_bytes = 0;
    obj_mem_descriptor_t *cur = g_alloc_list_head;

    // Conta i blocchi e calcola il totale dei byte allocati
    while (cur) {
        count++;
        total_bytes += cur->size;
        cur = cur->next;
    }

    TRACE_INFO(&stdtrace, "Dump memoria allocata: %zu blocchi per un totale di %zu bytes", count, total_bytes);

    // Itera nuovamente per stampare i dettagli dei blocchi
    cur = g_alloc_list_head;
    while (cur) {
        char time_str[64];
        struct tm tm_info;
        localtime_r(&(cur->alloc_time), &tm_info);
        strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", &tm_info);

        unsigned char *payload = (unsigned char *)(cur + 1);

        TRACE_INFO(&stdtrace, "Blocco: file=%s linea=%d size=%zu alloc_time=%s payload=0x%p", 
                   cur->file, cur->line, cur->size, time_str, (void *)payload);

        cur = cur->next;
    }
}

void dump_allocated_memory_hex(void) {
    obj_mem_descriptor_t *cur = g_alloc_list_head;
    TRACE_INFO(&stdtrace, "==== DUMP MEMORIA IN FORMATO ESADECIMALE ====");

    while (cur) {
        unsigned char *mem = (unsigned char*)(cur + 1);
        TRACE_INFO(&stdtrace, "Blocco: file=%s linea=%d size=%zu payload=0x%p", 
                   cur->file, cur->line, cur->size, (void*)mem);

        for (size_t i = 0; i < cur->size; i += 16) {
            char hex_line[128] = {0};
            char *ptr = hex_line;

            // Offset esadecimale (relativo all'inizio del payload)
            ptr += snprintf(ptr, sizeof(hex_line) - (ptr - hex_line), "    %04zx: ", i);

            // Prima sezione da 8 byte
            for (size_t j = 0; j < 8; j++) {
                if ((i + j) < cur->size) {
                    ptr += snprintf(ptr, sizeof(hex_line) - (ptr - hex_line), "%02x ", mem[i + j]);
                } else {
                    ptr += snprintf(ptr, sizeof(hex_line) - (ptr - hex_line), "__ ");
                }
            }

            ptr += snprintf(ptr, sizeof(hex_line) - (ptr - hex_line), "    "); // Spaziatura tra gruppi di 8 byte

            // Seconda sezione da 8 byte
            for (size_t j = 8; j < 16; j++) {
                if ((i + j) < cur->size) {
                    ptr += snprintf(ptr, sizeof(hex_line) - (ptr - hex_line), "%02x ", mem[i + j]);
                } else {
                    ptr += snprintf(ptr, sizeof(hex_line) - (ptr - hex_line), "__ ");
                }
            }

            // Se la seconda colonna è vuota, completa con `__`
            if ((i + 8) >= cur->size) {
                for (size_t k = 0; k < 8; k++) {
                    ptr += snprintf(ptr, sizeof(hex_line) - (ptr - hex_line), "__ ");
                }
            }

            // Stampa della riga formattata tramite TRACE_INFO
            TRACE_INFO(&stdtrace, "%s", hex_line);
        }

        cur = cur->next;
    }

    TRACE_INFO(&stdtrace, "==== FINE DUMP MEMORIA ESADECIMALE ====");
}

#endif /* OBJ_MEM_DEBUG_ON */
