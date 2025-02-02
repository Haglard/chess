/**
 * @file test_obj_dynamic_vector.c
 * @brief Esempio di unit test per la libreria obj_dynamic_vector, con test di corner case.
 *
 * ## Obiettivi
 * - Verificare la corretta gestione della memoria (alloc, realloc, free) senza stampare alcun dump.
 * - Testare che funzioni come dv_get e dv_set rispettino gli indici validi.
 * - Assicurare che dv_free non generi errori su vettori null o già liberati.
 * - Forzare alcuni comportamenti di ridimensionamento (push di molti elementi).
 *
 * ##VERSION## "test_obj_dynamic_vector.c 1.0"
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "obj_mem.h"
#include "obj_dynamic_vector.h"
#include "obj_trace.h"
#include "obj_ver.h"

/**
 * @brief Funzione di utility per stampare un elemento nel vettore.
 * 
 * @param dv     Il vettore dinamico.
 * @param index  L'indice da stampare.
 */
static void print_element(const dynamic_vector_t *dv, size_t index) {
    const char *elem = (const char *) dv_get(dv, index);
    if (elem) {
        printf("Elemento [%zu] = '%s'\n", index, elem);
    } else {
        printf("Elemento [%zu] = (null)\n", index);
    }
}

int main(void) {
    /* Inizializzazione del canale standard di logging: di default manda su stderr */
    trace_set_channel_output(&stdtrace, stderr);
    trace_set_channel_level(&stdtrace, TRACE_LEVEL_DEBUG);

    /* Stampa le versioni dei moduli */
    print_versions();

    /* --- Test 1: dv_free(NULL) deve essere innocua --- */
    printf("\n--- Test 1: dv_free(NULL) ---\n");
    dv_free(NULL);
    printf("Chiamato dv_free(NULL). Nessun crash atteso.\n");

    /* --- Test 2: Creazione vettore dinamico --- */
    printf("\n--- Test 2: Creazione vettore ---\n");
    dynamic_vector_t *dv = dv_create();
    if (!dv) {
        fprintf(stderr, "Errore: impossibile creare il vettore dinamico.\n");
        return 1; /* Exit code di errore */
    }
    printf("Vettore creato correttamente. dv_size = %zu\n", dv_size(dv));

    /* --- Test 3: Accesso out-of-bounds su vettore vuoto --- */
    printf("\n--- Test 3: Accesso out-of-bounds su vettore vuoto ---\n");
    void *out_elem = dv_get(dv, 0);
    if (!out_elem) {
        printf("dv_get(dv, 0) su vettore vuoto ha restituito NULL, come previsto.\n");
    }
    dv_set(dv, 0, "ValoreInesistente");
    printf("dv_set(dv, 0, ...) su vettore vuoto: nessun crash previsto.\n");

    /* --- Test 4: Inserimento di alcuni elementi --- */
    printf("\n--- Test 4: Inserimento di alcuni elementi ---\n");
    const char *strings[] = {"Hello", "World", "This", "Is", "A Test", NULL};
    for (int i = 0; strings[i] != NULL; i++) {
        if (dv_push_back(dv, (void*)strings[i]) != 0) {
            fprintf(stderr, "Errore: allocazione fallita durante il push_back.\n");
            dv_free(dv);
            return 1;
        }
        printf("Inserito '%s', dv_size = %zu\n", strings[i], dv_size(dv));
    }

    /* --- Verifica contenuto vettore --- */
    printf("\nContenuto attuale del vettore:\n");
    for (size_t i = 0; i < dv_size(dv); i++) {
        print_element(dv, i);
    }

    /* --- Test 5: Modifica di un elemento esistente (indice 1 -> "Dynamic Vector") --- */
    printf("\n--- Test 5: Modifica di un elemento esistente ---\n");
    dv_set(dv, 1, "Dynamic Vector");
    printf("Elemento in posizione 1 modificato. Verifichiamo:\n");
    for (size_t i = 0; i < dv_size(dv); i++) {
        print_element(dv, i);
    }

    /* --- Test 6: Accesso out-of-bounds su vettore NON vuoto --- */
    printf("\n--- Test 6: Accesso out-of-bounds su vettore NON vuoto ---\n");
    size_t current_size = dv_size(dv);
    void *out_elem2 = dv_get(dv, current_size); // ultimo indice valido = current_size - 1
    if (!out_elem2) {
        printf("dv_get(dv, %zu) (out-of-bounds) ha restituito NULL, ok.\n", current_size);
    }
    dv_set(dv, current_size, "OutOfRange");
    printf("dv_set(dv, %zu, ...) non deve crashare. (Nessuna modifica reale)\n", current_size);

    /* --- Test 7: Inserimento massiccio di elementi per forzare il ridimensionamento --- */
    printf("\n--- Test 7: Inserimento massiccio (300 elementi) ---\n");
    for (int i = 0; i < 300; i++) {
        if (dv_push_back(dv, "Prova") != 0) {
            fprintf(stderr, "Errore: allocazione fallita durante il push_back massiccio.\n");
            dv_free(dv);
            return 1;
        }
    }
    printf("Dopo l'inserimento massiccio, dv_size = %zu\n", dv_size(dv));

    /* --- Test 8: Impostazione di parte degli elementi a NULL (simile a "rimozione logica") --- */
    printf("\n--- Test 8: Azzero i primi 50 elementi del vettore (li metto a NULL) ---\n");
    for (int i = 0; i < 50; i++) {
        dv_set(dv, i, NULL);
    }
    printf("Verifichiamo i primi 10 elementi:\n");
    for (int i = 0; i < 10; i++) {
        print_element(dv, i);
    }

    /* --- Test 9: Liberazione del vettore --- */
    printf("\n--- Test 9: Liberazione del vettore ---\n");
    dv_free(dv);
    dv = NULL;
    printf("Vettore liberato correttamente.\n");

    /* --- Test 10: dv_free di un vettore gia` liberato o puntatore NULL --- */
    printf("\n--- Test 10: dv_free(dv) di nuovo su dv = NULL ---\n");
    dv_free(dv); // dv è NULL, non deve crashare.
    printf("Chiamato dv_free(NULL) di nuovo. Nessun crash.\n");

    printf("\nTutti i test sono stati eseguiti con successo.\n");
    return 0;
}
