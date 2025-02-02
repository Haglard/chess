#include "chess_gui_font.h"
#include <stdio.h>
#include <string.h>

// Percorsi fissi ai font (regular e bold)
static const char* gRegularFontPath = "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf";
static const char* gBoldFontPath    = "/usr/share/fonts/truetype/liberation/LiberationSans-Bold.ttf";

/**
 * @brief Inizializza SDL_ttf se non già avviato.
 */
void ChessGuiFont_Init(void)
{
    if (TTF_WasInit() == 0) {
        if (TTF_Init() == -1) {
            printf("Errore TTF_Init: %s\n", TTF_GetError());
            // In un programma reale potresti decidere di uscire o gestire l'errore diversamente
        }
    }
}

/**
 * @brief Chiude SDL_ttf se risulta avviato.
 */
void ChessGuiFont_Quit(void)
{
    if (TTF_WasInit() != 0) {
        TTF_Quit();
    }
}

/**
 * @brief Funzione di supporto per disegnare un pezzo di testo con un TTF_Font* 
 *        e restituire la larghezza in pixel.
 */
static int drawChunk(SDL_Renderer* renderer, TTF_Font* font,
                     const char* chunk, int x, int y)
{
    if (!chunk || !*chunk) {
        return 0;
    }

    // Colore bianco
    SDL_Color white = {255, 255, 255, 255};

    // Crea surface
    SDL_Surface* surf = TTF_RenderUTF8_Blended(font, chunk, white);
    if (!surf) {
        printf("Errore TTF_RenderUTF8_Blended: %s\n", TTF_GetError());
        return 0;
    }

    // Crea texture
    SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
    if (!tex) {
        printf("Errore SDL_CreateTextureFromSurface: %s\n", SDL_GetError());
        SDL_FreeSurface(surf);
        return 0;
    }

    int w = surf->w;
    int h = surf->h;
    SDL_FreeSurface(surf);

    SDL_Rect dst = { x, y, w, h };
    SDL_RenderCopy(renderer, tex, NULL, &dst);

    SDL_DestroyTexture(tex);
    return w; // larghezza orizzontale effettiva
}

/**
 * @brief Disegna testo in cui /b e /r modificano il font (bold, regular).
 */
int ChessGuiFont_DrawText(SDL_Renderer* renderer, const char* text,
                          int x, int y, int fontSize)
{
    if (!text || !renderer) {
        return 0;
    }

    // Carichiamo i due font (regular e bold) nella dimensione richiesta
    TTF_Font* fontReg = TTF_OpenFont(gRegularFontPath, fontSize);
    TTF_Font* fontBold= TTF_OpenFont(gBoldFontPath,    fontSize);
    if (!fontReg || !fontBold) {
        printf("Impossibile caricare i font reg/bold dimensione %d\n", fontSize);
        if (fontReg)  TTF_CloseFont(fontReg);
        if (fontBold) TTF_CloseFont(fontBold);
        return 0;
    }

    // Partiamo con font regular
    TTF_Font* currentFont = fontReg;

    char chunk[1024];
    memset(chunk, 0, sizeof(chunk));

    int outX = x;
    const char* p = text;

    while (*p) {
        // Controlliamo se la sequenza /b o /r è rilevata
        if (*p == '/' && (*(p+1) == 'b' || *(p+1) == 'r')) {
            // Disegniamo ciò che abbiamo accumulato finora
            if (chunk[0] != '\0') {
                int w = drawChunk(renderer, currentFont, chunk, outX, y);
                outX += w;
                memset(chunk, 0, sizeof(chunk));
            }
            // Cambiamo font
            if (*(p+1) == 'b') {
                currentFont = fontBold;
            } else {
                currentFont = fontReg;
            }
            // Saltiamo /b o /r
            p += 2;
        }
        else {
            // Accumula il carattere
            size_t len = strlen(chunk);
            if (len < sizeof(chunk)-1) {
                chunk[len] = *p;
                chunk[len+1] = '\0';
            }
            p++;
        }
    }

    // Disegna eventuale chunk rimasto
    if (chunk[0] != '\0') {
        int w = drawChunk(renderer, currentFont, chunk, outX, y);
        outX += w;
    }

    // Chiudiamo i font
    TTF_CloseFont(fontReg);
    TTF_CloseFont(fontBold);

    return outX - x; // larghezza orizzontale disegnata
}


int ChessGuiFont_GetLineHeight(int fontSize)
{
    // Carichiamo i font regular e bold, uno dei due è sufficiente per calcolare l'altezza
    // (oppure solo regular)
    static const char* regFontPath = "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf";
    TTF_Font* font = TTF_OpenFont(regFontPath, fontSize);
    if (!font) {
        // fallback
        return fontSize;
    }

    // Chiediamo a SDL_ttf l'altezza
    int height = TTF_FontHeight(font);

    TTF_CloseFont(font);
    return height;
}

