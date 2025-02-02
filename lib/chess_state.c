// chess_state.c
#include "chess_state.h"
#include <string.h>
#include <stdio.h>

/**
 * @brief Crea una copia profonda dello stato del gioco.
 */
void* chess_copy_state(const void *state) {
    if (!state) return NULL;

    bitboard_state_t* new_state = (bitboard_state_t*)malloc(sizeof(bitboard_state_t));
    if (!new_state) return NULL;

    memcpy(new_state, state, sizeof(bitboard_state_t));
    return new_state;
}

/**
 * @brief Libera la memoria allocata per uno stato del gioco.
 */
void chess_free_state(void *state) {
    if (state) {
        free(state);
    }
}

/**
 * @brief Inizializza la scacchiera con la posizione di partenza standard.
 */
void initialize_board(bitboard_state_t *state) {
    if (!state) return;

    // Inizializza tutti i Bitboards a zero
    memset(state, 0, sizeof(bitboard_state_t));

    // Posizionamento dei pedoni bianchi (seconda fila)
    state->white_pawns = 0x000000000000FF00;

    // Posizionamento dei pedoni neri (settima fila)
    state->black_pawns = 0x00FF000000000000;

    // Posizionamento delle torri bianche
    state->white_rooks = 0x0000000000000081;

    // Posizionamento delle torri nere
    state->black_rooks = 0x8100000000000000;

    // Posizionamento dei cavalli bianchi
    state->white_knights = 0x0000000000000042;

    // Posizionamento dei cavalli neri
    state->black_knights = 0x4200000000000000;

    // Posizionamento degli alfieri bianchi
    state->white_bishops = 0x0000000000000024;

    // Posizionamento degli alfieri neri
    state->black_bishops = 0x2400000000000000;

    // Posizionamento delle regine bianche
    state->white_queens = 0x0000000000000008;

    // Posizionamento delle regine nere
    state->black_queens = 0x0800000000000000;

    // Posizionamento dei re bianchi
    state->white_kings = 0x0000000000000010;

    // Posizionamento dei re neri
    state->black_kings = 0x1000000000000000;

    // Inizializza i diritti di arrocco (bianco e nero, corto e lungo)
    // Bitmask esempio:
    // 0000: nessun arrocco disponibile
    // 1111: tutti i diritti di arrocco disponibili
    // Bit 0: bianco corto (O-O)
    // Bit 1: bianco lungo (O-O-O)
    // Bit 2: nero corto (o-o)
    // Bit 3: nero lungo (o-o-o)
    state->castling_rights = 0xF; // Tutti i diritti di arrocco disponibili

    // Inizializza en passant a 255 (nessuna possibilità)
    state->en_passant = 255;

    // Inizializza il contatore delle mosse a 0
    state->halfmove_clock = 0;

    // Inizializza il numero del turno a 1 (primo turno)
    state->fullmove_number = 1;

    // **Inizializza il giocatore corrente**
    state->current_player = 1; // 1 = Bianco, -1 = Nero
}

/**
 * @brief Converte un indice di casella in notazione standard (es. 'e4').
 *
 * @param[in] square Indice della casella (0-63).
 * @param[out] notation Buffer per la notazione standard (almeno 3 caratteri).
 */
void square_to_notation(int square, char *notation) {
    if (square < 0 || square >= 64) {
        strcpy(notation, "-");
        return;
    }
    int file = square % 8;
    int rank = square / 8;
    snprintf(notation, 3, "%c%d", 'a' + file, rank + 1);
}

/**
 * @brief Decodifica i diritti di arrocco in una stringa leggibile.
 *
 * @param castling_rights Bitmask dei diritti di arrocco.
 * @param buffer Buffer di output per la stringa decodificata.
 * @param buffer_size Dimensione del buffer di output.
 */
void decode_castling_rights(uint8_t castling_rights, char *buffer, size_t buffer_size) {
    buffer[0] = '\0'; // Inizializza la stringa vuota

    // Controlla ogni bit e aggiungi il carattere corrispondente
    if (castling_rights & 1) {
        strcat(buffer, "K"); // Bianco corto
    }
    if (castling_rights & 2) {
        strcat(buffer, "Q"); // Bianco lungo
    }
    if (castling_rights & 4) {
        strcat(buffer, "k"); // Nero corto
    }
    if (castling_rights & 8) {
        strcat(buffer, "q"); // Nero lungo
    }

    // Se nessun diritto di arrocco è disponibile, usa '-'
    if (strlen(buffer) == 0) {
        strcat(buffer, "-");
    }
}

/**
 * @brief Stampa lo stato della scacchiera in formato testuale.
 *
 * Questa funzione visualizza la scacchiera in un formato 8x8, mostrando tutti i pezzi
 * e le informazioni aggiuntive come i diritti di arrocco, la casella en passant,
 * il giocatore corrente, i contatori delle mosse, ecc.
 *
 * @param[in] state Puntatore allo stato del gioco da stampare.
 */
void print_board(const bitboard_state_t *state) {
    if (!state) return;

    // Array per mappare i pezzi ai simboli
    const char pieces[12] = {
        'P', // white pawns
        'N', // white knights
        'B', // white bishops
        'R', // white rooks
        'Q', // white queens
        'K', // white kings
        'p', // black pawns
        'n', // black knights
        'b', // black bishops
        'r', // black rooks
        'q', // black queens
        'k'  // black kings
    };

    // Array per facilitare l'accesso ai bitboards
    const uint64_t bitboards[12] = {
        state->white_pawns,
        state->white_knights,
        state->white_bishops,
        state->white_rooks,
        state->white_queens,
        state->white_kings,
        state->black_pawns,
        state->black_knights,
        state->black_bishops,
        state->black_rooks,
        state->black_queens,
        state->black_kings
    };

    // Crea una mappa della scacchiera, inizializzata a '.'
    char board[64];
    memset(board, '.', sizeof(board));

    // Itera su ogni tipo di pezzo
    for (int i = 0; i < 12; i++) {
        for (int square = 0; square < 64; square++) {
            if (bitboards[i] & (1ULL << square)) {
                board[square] = pieces[i];
            }
        }
    }

    // Stampa la scacchiera
    printf("  +------------------------+\n");
    for (int rank = 7; rank >= 0; rank--) {
        printf("%d | ", rank + 1);
        for (int file = 0; file < 8; file++) {
            int idx = rank * 8 + file;
            printf("%c ", board[idx]);
        }
        printf("|\n");
    }
    printf("  +------------------------+\n");
    printf("    a b c d e f g h\n\n");

    // Stampa le informazioni aggiuntive dello stato
    printf("Informazioni Aggiuntive:\n");

    // **a. Diritti di Arrocco**
    char castling_rights_str[5];
    decode_castling_rights(state->castling_rights, castling_rights_str, sizeof(castling_rights_str));
    printf("Diritti di Arrocco: %s\n", castling_rights_str);

    // **b. Casella En Passant**
    char en_passant_str[3];
    if (state->en_passant != 255) {
        square_to_notation(state->en_passant, en_passant_str);
    } else {
        strcpy(en_passant_str, "-");
    }
    printf("En Passant: %s\n", en_passant_str);

    // **c. Contatori delle Mosse**
    printf("Halfmove Clock (contatore 50 mosse): %d\n", state->halfmove_clock);
    printf("Fullmove Number: %d\n", state->fullmove_number);

    // **d. Giocatore Corrente**
    printf("Giocatore Corrente: %s\n", state->current_player == 1 ? "Bianco" : "Nero");
}