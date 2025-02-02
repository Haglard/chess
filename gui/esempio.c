#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

// Tracing/logging (opzionale)
#include "obj_trace.h"

// Include principale GUI
#include "chess_gui.h"           
// Logica scacchi
#include "chess_to_gui_interface.h"
// Modulo font
#include "chess_gui_font.h"      


void fillHistoryRandomly(int n)
{
    // Inizializza il seed del random (se non l'hai già fatto altrove)
    srand((unsigned)time(NULL));

    for (int i = 0; i < n; i++) {
        move_record_t rec;
        memset(&rec, 0, sizeof(rec));  // imposta tutti i campi a 0

        // Se move_id = 0, verrà assegnato automaticamente
        rec.move_id = 0;

        // Pezzo mosso: un valore casuale (1..(CHESS_GUI_PIECE_COUNT-1)) => no CHESS_GUI_EMPTY
        rec.moved_piece = (ChessGuiPieces_t)(1 + rand() % (CHESS_GUI_PIECE_COUNT - 1));

        // Giocatore: 0=GUI_WHITE_PLAYER o 1=GUI_BLACK_PLAYER
        rec.player_who_moved = (rand() % 2 == 0) ? GUI_WHITE_PLAYER : GUI_BLACK_PLAYER;

        // Coordinate di partenza e arrivo: 0..7
        rec.from_c = rand() % 8;
        rec.from_r = rand() % 8;
        rec.to_c   = rand() % 8;
        rec.to_r   = rand() % 8;

        // Flag (bool) random
        rec.castling_right       = (rand() % 2 == 1);
        rec.castling_left        = (rand() % 2 == 1);
        rec.en_passant           = (rand() % 2 == 1);
        rec.opponent_under_check = (rand() % 2 == 1);
        rec.checkmate            = (rand() % 2 == 1);
        rec.draw                 = (rand() % 2 == 1);

        // Pezzo catturato: CHESS_GUI_EMPTY oppure un pezzo
        rec.captured_piece = (ChessGuiPieces_t)(rand() % CHESS_GUI_PIECE_COUNT);

        // Valutazione: un int casuale tra -10 e +10
        rec.board_status_evaluation = (rand() % 21) - 10;  // da -10 a +10

        // Aggiunge il record alla history
        game_history_add_move(&rec);
    }
}


int main(int argc, char* argv[])
{
    // 1) Configurazione tracing (facoltativa)
    trace_set_channel_output(&stdtrace, stdout);
    trace_set_channel_level(&stdtrace, TRACE_LEVEL_DEBUG);
    trace_enable_channel(&stdtrace, true);

    TRACE_INFO(&stdtrace, "Avvio del programma main (con scacchiera, dead tray e text area)");

    // 2) Inizializziamo finestra e renderer
    SDL_Window* window = NULL;
    SDL_Renderer* renderer = NULL;
    if (!initMainWindow(&window, &renderer)) {
        TRACE_ERROR(&stdtrace, "Impossibile inizializzare la finestra principale. Esco.");
        return 1;
    }
    TRACE_DEBUG(&stdtrace, "Finestra e renderer inizializzati con successo");

    // 3) Carichiamo board e pezzi
    if (!ChessGui_LoadAllTextures(renderer)) {
        TRACE_ERROR(&stdtrace, "Errore nel caricamento delle texture (board e/o pezzi). Esco.");
        closeSDL(window, renderer);
        return 1;
    }
    TRACE_DEBUG(&stdtrace, "Board + pezzi caricati correttamente");

    // 4) Inizializziamo la logica scacchi (board, dead pieces e history)
    ChessGuiBoard_t boardData;
    ChessGuiBoard_InitStandardArrangement(boardData);
    TRACE_DEBUG(&stdtrace, "Scacchiera inizializzata con i pezzi standard");

    game_history_init();
    TRACE_DEBUG(&stdtrace, "History inizializzata");

    ChessGuiDeadPieces_t dp;
    initDeadPiecesAllCaptured(&dp);  
    TRACE_DEBUG(&stdtrace, "Dead pieces con tutti i pezzi (dimostrativo)");

    // 5) Inizializza il sistema font
    ChessGuiFont_Init(); // Avvia SDL_ttf se necessario, definisce i path font

    // 6) Loop principale
    bool running = true;
    SDL_Event e;
    TRACE_INFO(&stdtrace, "Inizio del loop principale (eventi + rendering)");

    /////////////////////////////////////////
    fillHistoryRandomly(102);
    /////////////////////////////////////////

    while (running) {
        // Gestione eventi
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                running = false;
            }
        }

        // Puliamo lo schermo
        SDL_SetRenderDrawColor(renderer, 40, 40, 40, 255);
        SDL_RenderClear(renderer);

        // Disegniamo la board (texture + caselle)
        drawBoardEmpty(renderer, getLightColor(), getDarkColor());
        // Disegniamo i pezzi “in gioco”
        drawPieces(renderer, boardData);
        // Disegniamo i dead pieces
        drawDeadPiecesTray(renderer, &dp);
        // Stampiamo la history
        ChessGui_showHistory(renderer);

        // 7) Mostriamo il messaggio con markup /b e /r nella text area
        // Esempio di stringa con i "tag" e un /n (se interpretato letteralmente come testo).
        ChessGui_showUserMessage(renderer, "/bBlack/r moved /be2xd3/r white is under /bcheck/r");

        // Presentiamo a schermo
        SDL_RenderPresent(renderer);

        SDL_Delay(600); // ~60 FPS
    }

    // 8) Pulizia finale
    ChessGuiFont_Quit(); // Chiude SDL_ttf se aperto
    closeSDL(window, renderer);
    TRACE_INFO(&stdtrace, "Chiusura del programma completata con successo");
    return 0;
}







