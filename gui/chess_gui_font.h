#ifndef CHESS_GUI_FONT_H
#define CHESS_GUI_FONT_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdbool.h>

/**
 * @brief Inizializza il modulo di gestione font, avviando SDL_ttf (se non già fatto).
 *
 * Da chiamare una sola volta, prima di utilizzare le altre funzioni del modulo.
 */
void ChessGuiFont_Init(void);

/**
 * @brief Chiude il modulo di gestione font, arrestando SDL_ttf (se avviato).
 *
 * Da chiamare al termine del programma, se non viene gestito altrove.
 */
void ChessGuiFont_Quit(void);

/**
 * @brief Renderizza una singola riga di testo in cui /b e /r modificano il font 
 *        (bold, regular) al volo.
 *
 * @param renderer  Renderer SDL dove disegnare.
 * @param text      Stringa (può contenere /b e /r per switchare i font).
 * @param x         Coordinata X di partenza.
 * @param y         Coordinata Y di partenza.
 * @param fontSize  Dimensione (in pixel) dei font. 
 *                  Verranno caricati LiberationSans-Regular/Bold in questa dimensione.
 *
 * @return Larghezza (in pixel) complessiva di quanto disegnato.
 *
 * Esempio:
 * \code
 *   ChessGuiFont_DrawText(renderer, "Ciao /bMondo/r!", 50, 100, 14);
 * \endcode
 */
int ChessGuiFont_DrawText(SDL_Renderer* renderer, const char* text,
                          int x, int y, int fontSize);


/**
 * @brief Restituisce l'altezza (in pixel) di una riga di testo
 *        se si usa il font regular a dimensione fontSize.
 *
 * @param fontSize dimensione in pixel
 * @return altezza (pixel) secondo SDL_ttf, o fontSize se errore
 */
int ChessGuiFont_GetLineHeight(int fontSize);


#endif // CHESS_GUI_FONT_H
