#include <stdio.h>
#include <stdbool.h>
#include <string.h>

// Tracing (logging)
#include "obj_trace.h"               // Contiene stdtrace, TRACE_DEBUG, TRACE_INFO, ecc.

// GUI e SDL
#include "chess_gui.h"              // initMainWindow, closeSDL, ChessGui_LoadAllTextures, 
                                    // drawBoardEmpty, drawPieces, drawDeadPiecesTray, ...
// Logica scacchi
#include "chess_to_gui_interface.h"  // ChessGuiBoard_InitStandardArrangement,
//                                    // ChessGuiDeadPieces_t, initDeadPiecesAllCaptured, ecc.

int main(int argc, char* argv[])
{
    // 1) Configura tracing (stdtrace)
    trace_set_channel_output(&stdtrace, stdout);
    trace_set_channel_level(&stdtrace, TRACE_LEVEL_DEBUG);
    trace_enable_channel(&stdtrace, true);

    TRACE_INFO(&stdtrace, "Avvio del programma main (finestra %dx%d)", 
               WINDOW_WIDTH, WINDOW_HEIGHT);

    // 2) Inizializza la finestra e il renderer
    SDL_Window* window = NULL;
    SDL_Renderer* renderer = NULL;
    if (!initMainWindow(&window, &renderer)) {
        TRACE_ERROR(&stdtrace, "Impossibile inizializzare la finestra principale. Esco.");
        return 1;
    }
    TRACE_DEBUG(&stdtrace, "Finestra e renderer inizializzati con successo");

    // 3) Carichiamo board e tutti i pezzi
    //    (resta inteso che ChessGui_LoadAllTextures restituisce bool
    //     se qualcosa va male, gestiamo l'errore)
    if (!ChessGui_LoadAllTextures(renderer)) {
        TRACE_ERROR(&stdtrace, "Errore nel caricamento delle texture (board e/o pezzi). Esco.");
        closeSDL(window, renderer);
        return 1;
    }
    TRACE_DEBUG(&stdtrace, "Board + pezzi caricati correttamente in memoria");

    // 4) Inizializziamo la scacchiera logica 
    ChessGuiBoard_t boardData;
    ChessGuiBoard_InitStandardArrangement(boardData);
    TRACE_DEBUG(&stdtrace, "Scacchiera inizializzata con i pezzi standard");

    // 5) Inizializziamo la struttura "dead pieces" come se TUTTI i pezzi fossero catturati
    ChessGuiDeadPieces_t dp;
    initDeadPiecesAllCaptured(&dp);  
    TRACE_DEBUG(&stdtrace, "Dead pieces inizializzati con TUTTI i pezzi di entrambi i colori");

    // 6) Loop principale
    bool running = true;
    SDL_Event e;
    TRACE_INFO(&stdtrace, "Inizio del loop principale (ciclo di eventi e rendering)");

        // Pulizia schermo (colore sfondo)
        SDL_SetRenderDrawColor(renderer, 40, 40, 40, 255);
        SDL_RenderClear(renderer);

        // Disegno la board (texture + caselle) 
        drawBoardEmpty(renderer, (GuiColor){222,184,135,220}, (GuiColor){160,82,45,220});
        TRACE_DEBUG(&stdtrace, "Board disegnata");

        // Disegno i pezzi sulla scacchiera (se vogliamo, mostrare i pezzi "in gioco")
        drawPieces(renderer, boardData);

        // Disegno i "dead pieces" con la struttura dp
        drawDeadPiecesTray(renderer, &dp);

        // Presento a schermo
        SDL_RenderPresent(renderer);
        SDL_Delay(16); // ~60 FPS


    while (running) {
        // Gestione eventi
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                TRACE_INFO(&stdtrace, "Richiesta di uscita (evento QUIT)");
                running = false;
            }
            // Altri eventi ...
        }

    }

    // 7) Pulizia finale
    TRACE_INFO(&stdtrace, "Inizio pulizia finale del programma");

    // Se vuoi liberare le texture in un "unloadAllTextures" o simile, fai qui
    closeSDL(window, renderer);

    TRACE_INFO(&stdtrace, "Chiusura del programma completata con successo");
    return 0;
}
