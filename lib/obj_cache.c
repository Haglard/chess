/**
 * ##VERSION## "obj_cache.c 1.1"
*/

#include "obj_mem.h"
#include "obj_trace.h"
#include "obj_cache.h"
#include <stdio.h>
#include <string.h>

/* 
 * Struttura interna per memorizzare la singola voce (entry)
 * di una lista collegata in un bucket.
 */
struct hash_entry {
    void *key;             /* chiave */
    void *value;           /* valore associato */
    struct hash_entry *next;
};

/*
 * Struttura principale per la tabella hash:
 *  - array di puntatori a "hash_entry" (bucket)
 *  - dimensione attuale (capacity)
 *  - callback di hash e di confronto
 */
struct generic_hash_table {
    struct hash_entry **buckets;  /* array di liste collegate */
    size_t capacity;              /* dimensione dell'array buckets */

    hash_func_t   hash_cb;        /* calcolo dell'hash */
    equals_func_t eq_cb;          /* confronto delle chiavi */
};

/*
 * Struttura per l'iteratore esplicito
 */
struct cache_iterator {
    generic_hash_table_t *cache;   /* tabella su cui stiamo iterando */
    size_t current_bucket;         /* indice dell'array di bucket in cui siamo */
    struct hash_entry *current_entry; /* puntatore all'entry nella lista corrente */
};

/*
 * Funzione di utilità: calcola l'indice del bucket
 * in base all'hash e alla capacity della tabella.
 */
static size_t get_bucket_index(uint64_t hash, size_t capacity) {
    return (size_t)(hash % capacity);
}

/*
 * Crea la tabella hash con dimensione iniziale fissa (INITIAL_CAPACITY).
 * Non effettua rehashing dinamico in questo esempio.
 */
generic_hash_table_t* cache_create(hash_func_t hash_cb, equals_func_t eq_cb) {
    if (!hash_cb || !eq_cb) {
        TRACE_DEBUG(&stdtrace,
                    "cache_create: callback di hash o equals mancanti => ritorno NULL");
        return NULL; /* Callbacks obbligatorie */
    }

    TRACE_DEBUG(&stdtrace,
                "cache_create: Alloco la struttura principale con capacity=%d",
                INITIAL_CAPACITY);

    generic_hash_table_t *ht = (generic_hash_table_t*) malloc(sizeof(*ht));
    if (!ht) {
        TRACE_DEBUG(&stdtrace, "cache_create: malloc fallita per la struttura principale");
        return NULL;
    }

    ht->capacity = INITIAL_CAPACITY;
    ht->hash_cb = hash_cb;
    ht->eq_cb   = eq_cb;

    /* Alloca l'array di bucket */
    ht->buckets = (struct hash_entry**) calloc(ht->capacity, sizeof(struct hash_entry*));
    if (!ht->buckets) {
        TRACE_DEBUG(&stdtrace, "cache_create: calloc fallita per l'array di bucket");
        free(ht);
        return NULL;
    }

    TRACE_DEBUG(&stdtrace, "cache_create: completato => ht=%p, buckets=%p",
                (void*)ht, (void*)ht->buckets);
    return ht;
}

/*
 * Distrugge la tabella hash.
 * Nota: questa implementazione "grezza" NON chiama funzioni di free su key e value.
 * Se devi liberarle, fallo prima di chiamare cache_destroy, oppure estendi il codice
 * per accettare callback di cleanup (o mantieni un descrittore dedicato).
 */
void cache_destroy(generic_hash_table_t *cache) {
    if (!cache) {
        TRACE_DEBUG(&stdtrace, "cache_destroy: cache=NULL => non faccio nulla");
        return;
    }

    TRACE_DEBUG(&stdtrace, "cache_destroy: inizio => cache=%p", (void*)cache);

    /* Svuota ciascun bucket */
    for (size_t i = 0; i < cache->capacity; i++) {
        struct hash_entry *entry = cache->buckets[i];
        while (entry) {
            struct hash_entry *tmp = entry;
            entry = entry->next;
            /* Non liberiamo key e value perché "owned" dall'utente */
            free(tmp);
        }
    }

    /* Libera l'array di bucket */
    free(cache->buckets);

    /* Infine libera la struttura principale */
    free(cache);

    TRACE_DEBUG(&stdtrace, "cache_destroy: completato");
}

/*
 * Ricerca la chiave `key` e restituisce il value associato, o NULL se non c'è.
 */
void* cache_lookup(generic_hash_table_t *cache, const void *key) {
    if (!cache) {
        TRACE_DEBUG(&stdtrace, "cache_lookup: cache=NULL => ritorno NULL");
        return NULL;
    }
    if (!key) {
        TRACE_DEBUG(&stdtrace, "cache_lookup: key=NULL => ritorno NULL");
        return NULL;
    }

    /* Calcola l'hash e l'indice del bucket */
    uint64_t h = cache->hash_cb(key);
    size_t index = get_bucket_index(h, cache->capacity);

    TRACE_DEBUG(&stdtrace,
                "cache_lookup: cerco key=%p in bucket=%zu (hash=%llu)",
                key, index, (unsigned long long)h);

    /* Scorri la lista nel bucket */
    struct hash_entry *entry = cache->buckets[index];
    while (entry) {
        if (cache->eq_cb(entry->key, key)) {
            TRACE_DEBUG(&stdtrace, "cache_lookup: TROVATO => value=%p", entry->value);
            return entry->value;
        }
        entry = entry->next;
    }

    TRACE_DEBUG(&stdtrace, "cache_lookup: NON trovata la key=%p", key);
    return NULL; /* non trovata */
}

/*
 * Inserisce o sostituisce la coppia (key, value) nella tabella hash.
 * - Se la chiave esiste già, aggiorna il value.
 * - Altrimenti, crea un nuovo nodo e lo inserisce in testa alla lista.
 */
void cache_store(generic_hash_table_t *cache, const void *key, const void *value) {
    if (!cache) {
        TRACE_DEBUG(&stdtrace, "cache_store: cache=NULL => esco");
        return;
    }
    if (!key) {
        TRACE_DEBUG(&stdtrace, "cache_store: key=NULL => esco");
        return;
    }

    uint64_t h = cache->hash_cb(key);
    size_t index = get_bucket_index(h, cache->capacity);

    TRACE_DEBUG(&stdtrace,
                "cache_store: inserisco key=%p, value=%p in bucket=%zu (hash=%llu)",
                key, value, index, (unsigned long long)h);

    /* Cerchiamo se la chiave è già presente */
    struct hash_entry *entry = cache->buckets[index];
    while (entry) {
        if (cache->eq_cb(entry->key, key)) {
            /* Chiave trovata: aggiorna il value esistente */
            TRACE_DEBUG(&stdtrace,
                        "cache_store: key=%p già esistente => aggiorno il value da %p a %p",
                        key, entry->value, value);
            entry->value = (void*) value;
            return;
        }
        entry = entry->next;
    }

    /* Se non esiste, creiamo un nuovo entry e lo inseriamo in testa */
    struct hash_entry *new_entry = (struct hash_entry*) malloc(sizeof(*new_entry));
    if (!new_entry) {
        TRACE_DEBUG(&stdtrace, "cache_store: malloc fallita per new_entry => non inserisco nulla");
        return;
    }
    new_entry->key   = (void*) key;
    new_entry->value = (void*) value;
    new_entry->next  = cache->buckets[index];

    cache->buckets[index] = new_entry;
    TRACE_DEBUG(&stdtrace,
                "cache_store: key=%p, value=%p INSERITI in testa al bucket=%zu",
                key, value, index);
}

/* --------------------------------------------------------------------------
 * Modalità di navigazione 1: funzione for-each (callback-based)
 * -------------------------------------------------------------------------- */
void cache_for_each(generic_hash_table_t *cache, cache_enum_fn fn, void *user_data) {
    if (!cache || !fn) {
        TRACE_DEBUG(&stdtrace, "cache_for_each: cache=NULL o fn=NULL => esco");
        return;
    }

    TRACE_DEBUG(&stdtrace, "cache_for_each: inizio enumerazione su cache=%p", (void*)cache);

    /* Iteriamo su ciascun bucket */
    for (size_t i = 0; i < cache->capacity; i++) {
        struct hash_entry *entry = cache->buckets[i];
        while (entry) {
            fn(entry->key, entry->value, user_data);
            entry = entry->next;
        }
    }

    TRACE_DEBUG(&stdtrace, "cache_for_each: completato");
}

/* --------------------------------------------------------------------------
 * Modalità di navigazione 2: iteratore esplicito
 * -------------------------------------------------------------------------- */
cache_iterator_t* cache_iterator_create(generic_hash_table_t *cache) {
    if (!cache) {
        TRACE_DEBUG(&stdtrace, "cache_iterator_create: cache=NULL => esco");
        return NULL;
    }

    cache_iterator_t *it = (cache_iterator_t*) malloc(sizeof(*it));
    if (!it) {
        TRACE_DEBUG(&stdtrace, "cache_iterator_create: malloc fallita => esco");
        return NULL;
    }

    it->cache = cache;
    it->current_bucket = 0;
    it->current_entry = NULL;

    TRACE_DEBUG(&stdtrace, "cache_iterator_create: iteratore=%p creato su cache=%p",
                (void*)it, (void*)cache);

    return it;
}

void cache_iterator_destroy(cache_iterator_t *iter) {
    if (!iter) {
        TRACE_DEBUG(&stdtrace, "cache_iterator_destroy: iter=NULL => nulla da fare");
        return;
    }
    TRACE_DEBUG(&stdtrace, "cache_iterator_destroy: libero iter=%p", (void*)iter);
    free(iter);
}

/*
 * Ritorna l'elemento corrente e avanza all'elemento successivo.
 * Ritorna 1 se c'è un elemento valido, 0 se fine iterazione.
 */
int cache_iterator_next(cache_iterator_t *iter, const void **pkey, void **pvalue) {
    if (!iter) {
        TRACE_DEBUG(&stdtrace, "cache_iterator_next: iter=NULL => esco con 0");
        return 0;
    }
    if (!pkey || !pvalue) {
        TRACE_DEBUG(&stdtrace, "cache_iterator_next: pkey=NULL o pvalue=NULL => esco con 0");
        return 0;
    }

    generic_hash_table_t *cache = iter->cache;

    /* Se abbiamo già un current_entry, andiamo al successivo */
    if (iter->current_entry) {
        iter->current_entry = iter->current_entry->next;
    }

    /* Se iter->current_entry è NULL, passiamo ai successivi bucket */
    while (!iter->current_entry) {
        if (iter->current_bucket >= cache->capacity) {
            TRACE_DEBUG(&stdtrace, 
                        "cache_iterator_next: superato ultimo bucket => fine iterazione");
            return 0; /* fine iterazione */
        }
        iter->current_entry = cache->buckets[iter->current_bucket];
        iter->current_bucket++;
        if (iter->current_entry) {
            break;
        }
    }

    if (iter->current_entry) {
        *pkey   = iter->current_entry->key;
        *pvalue = iter->current_entry->value;
        TRACE_DEBUG(&stdtrace,
                    "cache_iterator_next: restituisco key=%p, value=%p [bucket=%zu]",
                    *pkey, *pvalue, iter->current_bucket-1);
        return 1;
    }

    return 0; 
}
