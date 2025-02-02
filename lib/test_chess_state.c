// test_chess_hash.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <inttypes.h> // Per i macro di formattazione
#include "chess_moves.h"
#include "chess_state.h"
#include "chess_hash.h"

/**
 * @brief Verifica che due hash siano uguali.
 *
 * @param hash1 Il primo hash.
 * @param hash2 Il secondo hash.
 * @return 1 se uguali, 0 altrimenti.
 */
int hashes_are_equal(uint64_t hash1, uint64_t hash2) {
    return hash1 == hash2;
}

/**
 * @brief Stampa il risultato di un test.
 *
 * @param test_name Nome del test.
 * @param passed 1 se passato, 0 altrimenti.
 */
void print_test_result(const char *test_name, int passed) {
    if (passed) {
        printf("[PASS] %s\n", test_name);
    } else {
        printf("[FAIL] %s\n", test_name);
    }
}

/**
 * @brief Stampa lo stato di un bitboard in formato esadecimale.
 *
 * @param name Nome del bitboard.
 * @param bitboard Valore del bitboard.
 */
void print_bitboard(const char *name, uint64_t bitboard) {
    printf("%s: 0x%016" PRIX64 "\n", name, bitboard);
}

/**
 * @brief Testa la corretta inizializzazione della scacchiera.
 */
void test_initial_board_hash() {
    bitboard_state_t initial_state;
    initialize_board(&initial_state);
    
    printf("Stato Iniziale della Scacchiera:\n");
    print_board(&initial_state);
    
    uint64_t hash = chess_hash_state(&initial_state);
    
    // Calcola l'hash atteso utilizzando la stessa funzione
    uint64_t expected_hash = chess_hash_state(&initial_state);
    
    int passed = hashes_are_equal(hash, expected_hash);
    print_test_result("Test Inizializzazione Scacchiera", passed);
    assert(passed);
}

/**
 * @brief Testa la funzione di copia dello stato del gioco.
 */
void test_copy_state() {
    printf("\n--- Inizio Test Copia Stato ---\n");
    bitboard_state_t original_state;
    initialize_board(&original_state);
    
    printf("Stato Originale:\n");
    print_board(&original_state);
    
    void *copied_state_void = chess_copy_state(&original_state);
    assert(copied_state_void != NULL);
    bitboard_state_t *copied_state = (bitboard_state_t*)copied_state_void;
    
    printf("Stato Copiato Prima della Modifica:\n");
    print_board(copied_state);
    
    // Verifica che gli stati siano uguali
    int equals = chess_equals_state(&original_state, copied_state);
    print_test_result("Test Copia Stato", equals);
    assert(equals);
    
    // Modifica lo stato copiato e verifica che non influenzi l'originale
    printf("\nModifica dello stato copiato: Rimuove il pedone bianco dalla casella a2 (bit 8)\n");
    copied_state->white_pawns &= ~(1ULL << 8); // Rimuove il pedone bianco dalla casella a2
    
    printf("Stato Originale Dopo la Modifica:\n");
    print_board(&original_state);
    printf("Stato Copiato Dopo la Modifica:\n");
    print_board(copied_state);
    
    equals = chess_equals_state(&original_state, copied_state);
    print_test_result("Test Indipendenza Copia Stato", !equals);
    assert(!equals);
    
    // Pulisci
    chess_free_state(copied_state_void);
    printf("--- Fine Test Copia Stato ---\n");
}

/**
 * @brief Testa la funzione di confronto tra due stati di gioco.
 */
void test_equals_state() {
    printf("\n--- Inizio Test Confronto Stati ---\n");
    bitboard_state_t state1;
    bitboard_state_t state2;
    initialize_board(&state1);
    initialize_board(&state2);
    
    printf("Stato 1:\n");
    print_board(&state1);
    printf("Stato 2:\n");
    print_board(&state2);
    
    // Stati iniziali dovrebbero essere uguali
    int equals = chess_equals_state(&state1, &state2);
    print_test_result("Test Uguale Stato Iniziale", equals);
    assert(equals);
    
    // Modifica uno stato
    printf("\nModifica dello Stato 2: Rimuove il pedone bianco dalla casella a2 (bit 8)\n");
    state2.white_pawns &= ~(1ULL << 8); // Rimuove il pedone bianco dalla casella a2
    
    printf("Stato 1 Dopo la Modifica Stato 2:\n");
    print_board(&state1);
    printf("Stato 2 Dopo la Modifica:\n");
    print_board(&state2);
    
    // Ora gli stati dovrebbero essere diversi
    equals = chess_equals_state(&state1, &state2);
    print_test_result("Test Stato Diverso Dopo Modifica", !equals);
    assert(!equals);
    printf("--- Fine Test Confronto Stati ---\n");
}

/**
 * @brief Testa la funzione di hashing dopo una mossa.
 */
void test_hash_after_move() {
    printf("\n--- Inizio Test Hash Dopo Mossa ---\n");
    bitboard_state_t initial_state;
    initialize_board(&initial_state);
    
    printf("Stato Originale:\n");
    print_board(&initial_state);
    
    // Supponiamo di muovere un pedone bianco da e2 a e4 (casella 8 a 24)
    // Bitboard indexing: a1=0, b1=1, ..., h1=7, a2=8, ..., h8=63
    chess_move_t move;
    move.from = 8;  // a2
    move.to = 16;    // a3
    move.promotion = 0; // Nessuna promozione
    move.is_castling = 0;
    move.is_en_passant = 0;
    
    // Applica la mossa
    bitboard_state_t *new_state = (bitboard_state_t*)chess_copy_state(&initial_state);
    assert(new_state != NULL);
    
    // Stampa lo stato copiato prima della mossa
    printf("Stato Copiato Prima della Mossa:\n");
    print_board(new_state);
    
    // Rimuovi il pedone da a2
    new_state->white_pawns &= ~(1ULL << move.from);
    // Aggiungi il pedone a a3
    new_state->white_pawns |= (1ULL << move.to);
    // Cambia il giocatore corrente
    new_state->current_player = -1;
    // Aggiorna halfmove_clock e fullmove_number se necessario
    new_state->halfmove_clock = 0;
    new_state->fullmove_number += 1;
    
    // Stampa lo stato dopo la mossa
    printf("Stato Copiato Dopo la Mossa:\n");
    print_board(new_state);
    
    // Calcola gli hash
    uint64_t hash_initial = chess_hash_state(&initial_state);
    uint64_t hash_new = chess_hash_state(new_state);
    
    printf("Hash Iniziale: 0x%016" PRIX64 "\n", hash_initial);
    printf("Hash Dopo la Mossa: 0x%016" PRIX64 "\n", hash_new);
    
    // Gli hash dovrebbero essere diversi
    int passed = !hashes_are_equal(hash_initial, hash_new);
    print_test_result("Test Hash Dopo Mossa", passed);
    assert(passed);
    
    // Pulisci
    chess_free_state(new_state);
    printf("--- Fine Test Hash Dopo Mossa ---\n");
}

/**
 * @brief Testa i corner cases dell'hashing.
 */
void test_corner_cases() {
    printf("\n--- Inizio Test Corner Cases ---\n");
    
    // Caso 1: Scacchiera vuota
    printf("\nCaso 1: Scacchiera Vuota\n");
    bitboard_state_t empty_state = {0};
    empty_state.current_player = 1; // Bianco
    empty_state.castling_rights = 0;
    empty_state.en_passant = 255; // Nessuna casella en passant
    empty_state.halfmove_clock = 0;
    empty_state.fullmove_number = 1;
    
    uint64_t hash_empty = chess_hash_state(&empty_state);
    
    printf("Scacchiera Vuota:\n");
    print_board(&empty_state);
    printf("Hash Scacchiera Vuota: 0x%016" PRIX64 "\n", hash_empty);
    
    // Creiamo un'altra scacchiera vuota e confrontiamo gli hash
    bitboard_state_t empty_state_copy = {0};
    empty_state_copy.current_player = 1;
    empty_state_copy.castling_rights = 0;
    empty_state_copy.en_passant = 255;
    empty_state_copy.halfmove_clock = 0;
    empty_state_copy.fullmove_number = 1;
    
    uint64_t hash_empty_copy = chess_hash_state(&empty_state_copy);
    
    printf("Scacchiera Vuota Copia:\n");
    print_board(&empty_state_copy);
    printf("Hash Scacchiera Vuota Copia: 0x%016" PRIX64 "\n", hash_empty_copy);
    
    int equals = hashes_are_equal(hash_empty, hash_empty_copy);
    print_test_result("Test Hash Scacchiera Vuota", equals);
    assert(equals);
    
    // Caso 2: Solo re bianco e re nero
    printf("\nCaso 2: Solo Re Bianco e Re Nero\n");
    bitboard_state_t kings_only;
    memset(&kings_only, 0, sizeof(bitboard_state_t));
    kings_only.white_kings = 1ULL << 4; // e1
    kings_only.black_kings = 1ULL << 60; // e8
    kings_only.current_player = 1;
    kings_only.castling_rights = 0;
    kings_only.en_passant = 255;
    kings_only.halfmove_clock = 0;
    kings_only.fullmove_number = 1;
    
    uint64_t hash_kings_only = chess_hash_state(&kings_only);
    
    printf("Solo Re Bianco e Re Nero:\n");
    print_board(&kings_only);
    printf("Hash Solo Re: 0x%016" PRIX64 "\n", hash_kings_only);
    
    // Copia identica
    bitboard_state_t kings_only_copy;
    memcpy(&kings_only_copy, &kings_only, sizeof(bitboard_state_t));
    uint64_t hash_kings_only_copy = chess_hash_state(&kings_only_copy);
    
    printf("Solo Re Bianco e Re Nero Copia:\n");
    print_board(&kings_only_copy);
    printf("Hash Solo Re Copia: 0x%016" PRIX64 "\n", hash_kings_only_copy);
    
    equals = hashes_are_equal(hash_kings_only, hash_kings_only_copy);
    print_test_result("Test Hash Solo Re", equals);
    assert(equals);
    
    // Modifica una posizione del re e verifica hash diverso
    printf("\nModifica la posizione del Re Bianco da e1 a f1\n");
    kings_only_copy.white_kings = 1ULL << 5; // Sposta il re bianco da e1 a f1
    uint64_t hash_kings_only_modified = chess_hash_state(&kings_only_copy);
    
    printf("Re Bianco Spostato a f1:\n");
    print_board(&kings_only_copy);
    printf("Hash Solo Re Modificato: 0x%016" PRIX64 "\n", hash_kings_only_modified);
    
    equals = hashes_are_equal(hash_kings_only, hash_kings_only_modified);
    print_test_result("Test Hash Re Spostato", !equals);
    assert(!equals);
    
    // Caso 3: Diritti di arrocco
    printf("\nCaso 3: Diritti di Arrocco\n");
    bitboard_state_t castling_state = {0};
    castling_state.white_kings = 1ULL << 4; // e1
    castling_state.white_rooks = (1ULL << 0) | (1ULL << 7); // a1 e h1
    castling_state.black_kings = 1ULL << 60; // e8
    castling_state.black_rooks = (1ULL << 56) | (1ULL << 63); // a8 e h8
    castling_state.castling_rights = 0xF; // Tutti i diritti di arrocco disponibili
    castling_state.current_player = 1;
    castling_state.en_passant = 255;
    castling_state.halfmove_clock = 0;
    castling_state.fullmove_number = 1;
    
    uint64_t hash_castling = chess_hash_state(&castling_state);
    
    printf("Diritti di Arrocco Completi:\n");
    print_board(&castling_state);
    printf("Hash Diritti Arrocco: 0x%016" PRIX64 "\n", hash_castling);
    
    // Rimuovi i diritti di arrocco bianco lato re
    printf("Rimuove i diritti di arrocco lato re bianco\n");
    castling_state.castling_rights &= ~(1 << 0); // Rimuove l'arrocco lato re bianco
    
    uint64_t hash_castling_modified = chess_hash_state(&castling_state);
    
    printf("Diritti di Arrocco Dopo la Rimozione:\n");
    print_board(&castling_state);
    printf("Hash Diritti Arrocco Modificato: 0x%016" PRIX64 "\n", hash_castling_modified);
    
    // Hash dovrebbero essere diversi
    equals = hashes_are_equal(hash_castling, hash_castling_modified);
    print_test_result("Test Hash Diritti Arrocco Modificati", !equals);
    assert(!equals);
    
    // Caso 4: En passant
    printf("\nCaso 4: En Passant\n");
    bitboard_state_t en_passant_state = {0};
    en_passant_state.white_pawns = 1ULL << 12; // a3 (bit 12)
    en_passant_state.black_pawns = 1ULL << 20; // a6 (bit 20)
    en_passant_state.current_player = -1;
    en_passant_state.en_passant = 20; // a6 (bit 20)
    en_passant_state.castling_rights = 0;
    en_passant_state.halfmove_clock = 0;
    en_passant_state.fullmove_number = 1;
    
    uint64_t hash_en_passant = chess_hash_state(&en_passant_state);
    
    printf("Stato con En Passant Disponibile:\n");
    print_board(&en_passant_state);
    printf("Hash En Passant: 0x%016" PRIX64 "\n", hash_en_passant);
    
    // Rimuovi la casella en passant
    printf("Rimuove la casella En Passant\n");
    en_passant_state.en_passant = 255;
    uint64_t hash_en_passant_removed = chess_hash_state(&en_passant_state);
    
    printf("Stato Dopo la Rimozione di En Passant:\n");
    print_board(&en_passant_state);
    printf("Hash En Passant Rimosso: 0x%016" PRIX64 "\n", hash_en_passant_removed);
    
    // Hash dovrebbero essere diversi
    equals = hashes_are_equal(hash_en_passant, hash_en_passant_removed);
    print_test_result("Test Hash En Passant Modificato", !equals);
    assert(!equals);
    
    printf("--- Fine Test Corner Cases ---\n");
}

/**
 * @brief Testa la funzione di hash con una scacchiera completamente popolata.
 */
void test_full_board_hash() {
    printf("\n--- Inizio Test Scacchiera Completa ---\n");
    bitboard_state_t full_state;
    memset(&full_state, 0, sizeof(bitboard_state_t));
    
    // Posiziona tutti i pezzi bianchi
    full_state.white_pawns = 0x000000000000FF00;
    full_state.white_knights = 0x0000000000000042;
    full_state.white_bishops = 0x0000000000000024;
    full_state.white_rooks = 0x0000000000000081;
    full_state.white_queens = 0x0000000000000008;
    full_state.white_kings = 0x0000000000000010;
    
    // Posiziona tutti i pezzi neri
    full_state.black_pawns = 0x00FF000000000000;
    full_state.black_knights = 0x4200000000000000;
    full_state.black_bishops = 0x2400000000000000;
    full_state.black_rooks = 0x8100000000000000;
    full_state.black_queens = 0x0800000000000000;
    full_state.black_kings = 0x1000000000000000;
    
    // Imposta diritti di arrocco
    full_state.castling_rights = 0xF; // Tutti i diritti
    
    // Nessuna casella en passant
    full_state.en_passant = 255;
    
    // Imposta contatori
    full_state.halfmove_clock = 0;
    full_state.fullmove_number = 1;
    
    // Imposta il giocatore corrente
    full_state.current_player = 1;
    
    printf("Scacchiera Completa:\n");
    print_board(&full_state);
    
    uint64_t hash_full = chess_hash_state(&full_state);
    printf("Hash Scacchiera Completa: 0x%016" PRIX64 "\n", hash_full);
    
    // Copia identica
    bitboard_state_t full_state_copy;
    memcpy(&full_state_copy, &full_state, sizeof(bitboard_state_t));
    uint64_t hash_full_copy = chess_hash_state(&full_state_copy);
    
    printf("Scacchiera Completa Copia:\n");
    print_board(&full_state_copy);
    printf("Hash Scacchiera Completa Copia: 0x%016" PRIX64 "\n", hash_full_copy);
    
    int equals = hashes_are_equal(hash_full, hash_full_copy);
    print_test_result("Test Hash Scacchiera Completa Copia Identica", equals);
    assert(equals);
    
    // Modifica una casella e verifica hash diverso
    printf("Modifica: Rimuove un pedone nero da a7 (bit 48)\n");
    full_state_copy.black_pawns &= ~(1ULL << 48); // Rimuove un pedone nero da a7
    uint64_t hash_full_modified = chess_hash_state(&full_state_copy);
    
    printf("Scacchiera Completa Modificata:\n");
    print_board(&full_state_copy);
    printf("Hash Scacchiera Completa Modificata: 0x%016" PRIX64 "\n", hash_full_modified);
    
    equals = hashes_are_equal(hash_full, hash_full_modified);
    print_test_result("Test Hash Scacchiera Completa Modificata", !equals);
    assert(!equals);
    
    printf("--- Fine Test Scacchiera Completa ---\n");
}

int main() {
    printf("Inizio Test Modulo chess_hash...\n");

    chess_hash_init();
    
    test_initial_board_hash();
    test_copy_state();
    test_equals_state();
    test_hash_after_move();
    test_corner_cases();
    test_full_board_hash();
    
    printf("\nTutti i test sono stati completati con successo.\n");
    
    return 0;
}
