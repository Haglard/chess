/**
 * @file test_obj_cache.c
 * @brief Programma di unit test esteso per la libreria obj_cache, con test di:
 *  - creazione/distruzione,
 *  - inserimento/lookup/aggiornamento,
 *  - gestione collisioni,
 *  - traversamento con due metodi (callback e iteratore).
 *
 * ##VERSION## "test_obj_cache.c 1.1"
 *
 * Esempio di compilazione:
 *   make build TARGET=test_obj_cache CFLAGS="-DOBJ_MEM_DEBUG_ON"
 * Esecuzione:
 *   ./test_obj_cache
 */


#include "obj_mem.h"
#include "obj_trace.h"
#include "obj_cache.h"
#include "obj_ver.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>



/* ----------------------------------------------------------------------------
 * Callback di hash e confronto per CHIAVI STRINGA
 * ----------------------------------------------------------------------------
 */
static uint64_t hash_str(const void *key) {
    if (!key) return 0;
    const char *str = (const char *)key;
    uint64_t hash = 5381;
    int c;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    }
    return hash;
}

static int equals_str(const void *keyA, const void *keyB) {
    if (!keyA || !keyB) return 0;
    return (strcmp((const char *)keyA, (const char *)keyB) == 0) ? 1 : 0;
}

/* ----------------------------------------------------------------------------
 * Callback di hash e confronto per CHIAVI INTERE
 * ----------------------------------------------------------------------------
 */
static uint64_t hash_int(const void *key) {
    if (!key) return 0;
    /* Implementazione semplice e potenzialmente soggetta a collisioni */
    uint64_t val = *(const int*)key;
    /* Leggera manipolazione dei bit */
    val = (val ^ (val << 21)) ^ (val >> 35);
    return val;
}

static int equals_int(const void *keyA, const void *keyB) {
    if (!keyA || !keyB) return 0;
    return (*(const int*)keyA == *(const int*)keyB) ? 1 : 0;
}

/* ----------------------------------------------------------------------------
 * Funzioni di test
 * ----------------------------------------------------------------------------
 */

/**
 * @brief Testa la creazione e la distruzione della cache.
 */
static int test_create_destroy(void) {
    printf("\n=== test_create_destroy ===\n");
    generic_hash_table_t *cache = cache_create(hash_str, equals_str);
    if (!cache) {
        fprintf(stderr, "ERRORE: cache_create con hash_str/equals_str ha restituito NULL.\n");
        return 1;
    }
    printf("Cache creata correttamente (hash_str, equals_str).\n");

    dump_allocated_memory();
    dump_allocated_memory_hex();

    cache_destroy(cache);
    printf("Cache distrutta correttamente.\n");

    /* Test con callback mancanti -> dovrebbe restituire NULL */
    generic_hash_table_t *cache2 = cache_create(NULL, equals_str);
    if (cache2) {
        fprintf(stderr, "ERRORE: cache_create doveva restituire NULL per hash_cb= NULL.\n");
        cache_destroy(cache2);
        return 1;
    }
    printf("cache_create(NULL,...) => NULL, come previsto.\n");

    return 0;
}

/**
 * @brief Testa inserimento e lookup di chiavi stringa, inclusi aggiornamenti.
 */
static int test_string_store_lookup(void) {
    printf("\n=== test_string_store_lookup ===\n");
    generic_hash_table_t *cache = cache_create(hash_str, equals_str);
    if (!cache) {
        fprintf(stderr, "ERRORE: impossibile creare la cache (hash_str, equals_str).\n");
        return 1;
    }

    /* Inseriamo alcune chiavi (string -> string) */
    cache_store(cache, "apple",  "fruit");
    cache_store(cache, "carrot", "vegetable");
    cache_store(cache, "tomato", "berry?");
    cache_store(cache, "banana", "fruit");
    printf("Inserite 4 coppie (string->string).\n");

    /* Lookup di una chiave esistente */
    const char *val1 = (const char*) cache_lookup(cache, "apple");
    if (val1 && strcmp(val1, "fruit") == 0) {
        printf("lookup('apple') => '%s' (OK)\n", val1);
    } else {
        fprintf(stderr, "ERRORE: 'apple' non trovata o valore errato.\n");
        cache_destroy(cache);
        return 1;
    }

    /* Lookup di chiave assente */
    const char *val2 = (const char*) cache_lookup(cache, "pineapple");
    if (!val2) {
        printf("lookup('pineapple') => NULL (chiave inesistente), ok.\n");
    } else {
        fprintf(stderr, "ERRORE: 'pineapple' non dovrebbe esistere!\n");
        cache_destroy(cache);
        return 1;
    }

    /* Aggiornamento di 'carrot' */
    cache_store(cache, "carrot", "root");
    const char *val3 = (const char*) cache_lookup(cache, "carrot");
    if (!val3 || strcmp(val3, "root") != 0) {
        fprintf(stderr, "ERRORE: 'carrot' non risulta aggiornata a 'root'.\n");
        cache_destroy(cache);
        return 1;
    }
    printf("Aggiornamento 'carrot' => 'root' (OK)\n");

    cache_destroy(cache);
    printf("Cache distrutta correttamente.\n");
    return 0;
}

/**
 * @brief Testa inserimento e lookup di chiavi intere (int->string),
 *        inclusi aggiornamenti e key assenti.
 */
static int test_int_store_lookup(void) {
    printf("\n=== test_int_store_lookup ===\n");
    generic_hash_table_t *cache = cache_create(hash_int, equals_int);
    if (!cache) {
        fprintf(stderr, "ERRORE: impossibile creare la cache (hash_int, equals_int).\n");
        return 1;
    }

    int k1 = 42, k2 = 100, k3 = 9999, k4 = 42, k5 = -1, k6 = 9999;
    cache_store(cache, &k1, "Answer");
    cache_store(cache, &k2, "Hundred");
    cache_store(cache, &k3, "BigOne");
    cache_store(cache, &k5, "NegativeOne");
    printf("Inserite 4 coppie int->string.\n");

    /* Verifica */
    const char *v1 = (const char*) cache_lookup(cache, &k1);
    if (!v1 || strcmp(v1, "Answer") != 0) {
        fprintf(stderr, "ERRORE: lookup(42) => '%s', atteso 'Answer'.\n", v1 ? v1 : "(null)");
        cache_destroy(cache);
        return 1;
    }
    printf("lookup(42) => '%s' (OK)\n", v1);

    /* Aggiorniamo 42 con "NewAnswer" */
    cache_store(cache, &k4, "NewAnswer");
    const char *v4 = (const char*) cache_lookup(cache, &k4);
    if (!v4 || strcmp(v4, "NewAnswer") != 0) {
        fprintf(stderr, "ERRORE: aggiornamento 42 => 'NewAnswer' fallito.\n");
        cache_destroy(cache);
        return 1;
    }
    printf("Aggiornamento (42) => 'NewAnswer' (OK)\n");

    /* Aggiorniamo 9999 con "VeryBigOne" */
    cache_store(cache, &k6, "VeryBigOne");
    const char *v6 = (const char*) cache_lookup(cache, &k3);
    if (!v6 || strcmp(v6, "VeryBigOne") != 0) {
        fprintf(stderr, "ERRORE: aggiornamento 9999 => 'VeryBigOne' fallito.\n");
        cache_destroy(cache);
        return 1;
    }
    printf("Aggiornamento (9999) => 'VeryBigOne' (OK)\n");

    /* Key inesistente */
    int missing = 12345;
    const char *vmiss = (const char*) cache_lookup(cache, &missing);
    if (!vmiss) {
        printf("lookup(12345) => NULL (OK, chiave assente)\n");
    } else {
        fprintf(stderr, "ERRORE: 12345 non doveva esistere!\n");
        cache_destroy(cache);
        return 1;
    }

    cache_destroy(cache);
    printf("Cache distrutta correttamente.\n");
    return 0;
}

/**
 * @brief Inserisce un grande numero di chiavi int per forzare collisioni
 *        e verifica un sottoinsieme di esse, poi distrugge la cache.
 */
static int test_hash_conflicts(void) {
    printf("\n=== test_hash_conflicts ===\n");
    generic_hash_table_t *cache = cache_create(hash_int, equals_int);
    if (!cache) {
        fprintf(stderr, "ERRORE: impossibile creare la cache (hash_int, equals_int).\n");
        return 1;
    }

    const int N = 2000;
    printf("Inserisco %d chiavi int, per forzare collisioni.\n", N);
    for (int i = 0; i < N; i++) {
        /* per i valori salviamo una stringa statica o dinamica */
        /* Esempio: i come stringa */
        char *val = (char*) malloc(32);
        if (!val) {
            fprintf(stderr, "ERRORE: alloc fallita.\n");
            cache_destroy(cache);
            return 1;
        }
        snprintf(val, 32, "Val_%d", i);

        int *key_ptr = (int*) malloc(sizeof(int));
        if (!key_ptr) {
            fprintf(stderr, "ERRORE: alloc fallita.\n");
            free(val);
            cache_destroy(cache);
            return 1;
        }
        *key_ptr = i;

        cache_store(cache, key_ptr, val);
    }

    /* Verifichiamo un sottoinsieme */
    int test_keys[] = {0, 10, 42, 999, 1999, -1};
    for (int i = 0; i < (int)(sizeof(test_keys)/sizeof(test_keys[0])); i++) {
        int tk = test_keys[i];
        const char *val = (const char*) cache_lookup(cache, &tk);
        if (tk < 0 || tk >= N) {
            /* tk=-1 => non era inserito => deve essere NULL */
            if (!val) {
                printf("lookup(%d) => NULL, come atteso (non inserito)\n", tk);
            } else {
                fprintf(stderr, "ERRORE: lookup(%d) ha restituito '%s' ma non doveva esistere!\n", tk, val);
            }
        } else {
            /* Era stato inserito */
            if (!val) {
                fprintf(stderr, "ERRORE: lookup(%d) => NULL, ma l'avevo inserito!\n", tk);
            } else {
                printf("lookup(%d) => '%s' (OK)\n", tk, val);
            }
        }
    }

    /* Non stiamo liberando (key, value) manualmente prima di distruggere la cache,
       quindi potremmo avere un leak se stiamo facendo debug della memoria.
       Nel tuo progetto, potresti gestire la pulizia di key_ptr e val.
    */

    cache_destroy(cache);
    printf("Cache distrutta. (Chiavi/valori non liberati automaticamente)\n");
    return 0;
}

/* ----------------------------------------------------------------------------
 *  Esempi di TEST sulle funzioni di navigazione
 * ----------------------------------------------------------------------------
 */

/* callback per cache_for_each */
static void print_entry_callback(const void *key, const void *value, void *user_data) {
    (void)user_data;
    printf("  key=%p, value=%p\n", key, value);
}

/* Test che mostra come usare cache_for_each */
static int test_for_each(void) {
    printf("\n=== test_for_each ===\n");
    generic_hash_table_t *cache = cache_create(hash_str, equals_str);
    if (!cache) {
        fprintf(stderr, "ERRORE: impossibile creare la cache.\n");
        return 1;
    }

    cache_store(cache, "alpha", "AAA");
    cache_store(cache, "beta",  "BBB");
    cache_store(cache, "gamma", "GGG");
    printf("Inserite 3 coppie (string->string).\n");

    printf("Enumerazione con cache_for_each:\n");
    cache_for_each(cache, print_entry_callback, NULL);

    cache_destroy(cache);
    printf("Cache distrutta.\n");
    return 0;
}

/* Test che mostra come usare l'iteratore esplicito */
static int test_iterator(void) {
    printf("\n=== test_iterator ===\n");
    generic_hash_table_t *cache = cache_create(hash_int, equals_int);
    if (!cache) {
        fprintf(stderr, "ERRORE: impossibile creare la cache.\n");
        return 1;
    }

    int k1=10, k2=20, k3=30;
    cache_store(cache, &k1, "Ten");
    cache_store(cache, &k2, "Twenty");
    cache_store(cache, &k3, "Thirty");
    printf("Inserite 3 coppie int->string.\n");

    cache_iterator_t *it = cache_iterator_create(cache);
    if (!it) {
        fprintf(stderr, "ERRORE: impossibile creare iteratore.\n");
        cache_destroy(cache);
        return 1;
    }

    printf("Iterazione con cache_iterator:\n");
    const void *key;
    void *val;
    while (cache_iterator_next(it, &key, &val)) {
        printf("  key=%p, value=%p\n", key, val);
    }
    cache_iterator_destroy(it);

    cache_destroy(cache);
    printf("Cache distrutta.\n");
    return 0;
}

/* ----------------------------------------------------------------------------
 * MAIN: Eseguo tutti i test
 * ----------------------------------------------------------------------------
 */
int main(void) {
    /* Se usi obj_trace e obj_mem per debug, impostali se serve */
    trace_set_channel_output(&stdtrace, stderr);
    trace_set_channel_level(&stdtrace, TRACE_LEVEL_DEBUG);

    /* Stampa versioni (se usi obj_ver) */
    printf("\n=== STAMPA DELLE VERSIONI ===\n");
    print_versions();

    /* Esecuzione test in sequenza */
    int retval = 0;

    if ((retval = test_create_destroy()) != 0) return retval;
    if ((retval = test_string_store_lookup()) != 0) return retval;
    if ((retval = test_int_store_lookup()) != 0) return retval;
    if ((retval = test_hash_conflicts()) != 0) return retval;

    /* Test di enumerazione */
    if ((retval = test_for_each()) != 0) return retval;
    if ((retval = test_iterator()) != 0) return retval;

    printf("\nTutti i test su obj_cache completati con SUCCESSO.\n");
    return 0;
}
