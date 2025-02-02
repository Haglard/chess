#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdbool.h>
#include <dirent.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define START_DIR "/usr/share/fonts/truetype"  // Directory principale da cui partire
#define MAX_FONTS 256                          // Numero max di font da collezionare
#define MAX_PATH_LEN 512                       // Lunghezza massima path font

// Elenco di colori "rolling"
static SDL_Color gColors[] = {
    {255, 255, 255, 255}, // Bianco
    {255, 255,   0, 255}, // Giallo
    {  0, 255,   0, 255}, // Verde (chiaro)
    {  0,   0, 255, 255}, // Blu
    {139,  69,  19, 255}, // Marrone
    {128,   0, 128, 255}, // Viola
    {  0, 128,   0, 255}, // Verde (scuro)
    {135, 206, 235, 255}, // Celeste
};
static const int gNumColors = sizeof(gColors) / sizeof(gColors[0]);

// Funzione ricorsiva per cercare file .ttf in tutte le sottocartelle.
void scanDirRecursive(const char* basePath, char filePaths[][MAX_PATH_LEN], int* count, int maxCount) {
    DIR* dir = opendir(basePath);
    if (!dir) {
        return;
    }

    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        // Ignora . e ..
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        // Costruisce il path completo
        char path[1024];
        snprintf(path, sizeof(path), "%s/%s", basePath, entry->d_name);

        // Stat per controllare se è file regolare o directory
        struct stat info;
        if (stat(path, &info) == 0) {
            // Se è una directory, chiama la funzione in modo ricorsivo
            if (S_ISDIR(info.st_mode)) {
                scanDirRecursive(path, filePaths, count, maxCount);
            }
            // Se è un file, controlla l'estensione
            else if (S_ISREG(info.st_mode)) {
                const char* dot = strrchr(entry->d_name, '.');
                if (dot && strcmp(dot, ".ttf") == 0) {
                    // Salva il path se abbiamo ancora spazio
                    if (*count < maxCount) {
                        strncpy(filePaths[*count], path, MAX_PATH_LEN - 1);
                        filePaths[*count][MAX_PATH_LEN - 1] = '\0'; // Terminatore di sicurezza
                        (*count)++;
                    }
                }
            }
        }
    }

    closedir(dir);
}

// Funzione di utilità: renderizza e disegna testo in una certa posizione (x, y).
// Restituisce l'altezza effettiva occupata dal testo (utile se vuoi impilare più righe).
int renderText(SDL_Renderer* renderer, TTF_Font* font, const char* text, 
               SDL_Color color, int x, int y, int wrapLength) 
{
    // Crea la surface
    SDL_Surface* surface = TTF_RenderUTF8_Blended_Wrapped(font, text, color, wrapLength);
    if (!surface) {
        printf("Errore nella creazione della Surface: %s\n", TTF_GetError());
        return 0;
    }

    // Crea la texture
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) {
        printf("Errore nella creazione della Texture: %s\n", SDL_GetError());
        SDL_FreeSurface(surface);
        return 0;
    }

    // Dimensioni del testo
    int texW = surface->w;
    int texH = surface->h;
    SDL_FreeSurface(surface);

    // Posizionamento
    SDL_Rect dstRect;
    dstRect.x = x;
    dstRect.y = y;
    dstRect.w = texW;
    dstRect.h = texH;

    // Disegno
    SDL_RenderCopy(renderer, texture, NULL, &dstRect);

    // Pulizia
    SDL_DestroyTexture(texture);

    // Restituisco l'altezza per poter "accumulare" le righe
    return texH;
}

int main(int argc, char* argv[]) {
    // Inizializza SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("Errore SDL_Init: %s\n", SDL_GetError());
        return 1;
    }

    // Inizializza SDL_ttf
    if (TTF_Init() == -1) {
        printf("Errore TTF_Init: %s\n", TTF_GetError());
        SDL_Quit();
        return 1;
    }

    // Verifica la versione di SDL_ttf (per TTF_OpenFontDPI serve >= 2.0.18)
    // (Facoltativo: se sai già di avere la versione giusta, puoi ometterlo)

    // Crea finestra
    SDL_Window* window = SDL_CreateWindow(
        "Visualizzatore Font (8pt, 10pt, 12pt, colori rolling)",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        800, 600,
        SDL_WINDOW_SHOWN
    );
    if (!window) {
        printf("Errore SDL_CreateWindow: %s\n", SDL_GetError());
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    // Crea renderer
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        printf("Errore SDL_CreateRenderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    // Buffer per salvare i percorsi dei font
    char fontFiles[MAX_FONTS][MAX_PATH_LEN];
    int fontCount = 0;

    // Scansione ricorsiva dei font .ttf
    scanDirRecursive(START_DIR, fontFiles, &fontCount, MAX_FONTS);

    if (fontCount == 0) {
        printf("Nessun file .ttf trovato in %s (o sottocartelle)\n", START_DIR);
    } else {
        printf("Trovati %d font .ttf in %s (ricorsivamente)\n", fontCount, START_DIR);
    }

    bool running = true;
    SDL_Event event;

    // Definisci i DPI (spesso 96 per i monitor classici, ma può variare)
    unsigned int hdpi = 96;
    unsigned int vdpi = 96;

    // Mostra i font uno alla volta
    for (int i = 0; i < fontCount && running; i++) {
        // Controlla se l'utente chiude la finestra prima di disegnare
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }
        if (!running) break;

        // Apriamo il font in 3 dimensioni: 8pt, 10pt, 12pt (in punti tipografici reali)
        TTF_Font* font8  = TTF_OpenFontDPI(fontFiles[i],  8, hdpi, vdpi);
        TTF_Font* font10 = TTF_OpenFontDPI(fontFiles[i], 10, hdpi, vdpi);
        TTF_Font* font12 = TTF_OpenFontDPI(fontFiles[i], 12, hdpi, vdpi);

        // Se almeno uno non è stato aperto correttamente, passiamo oltre
        if (!font8 || !font10 || !font12) {
            printf("Impossibile aprire il font (8pt,10pt,12pt) per: %s\n", fontFiles[i]);
            if (font8)  TTF_CloseFont(font8);
            if (font10) TTF_CloseFont(font10);
            if (font12) TTF_CloseFont(font12);
            continue;
        }

        // Pulizia schermo (nero)
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // Scegli il colore in modo rolling (ciclico) in base a i
        SDL_Color color = gColors[i % gNumColors];

        // Creiamo il testo da mostrare
        char text[1024];
        snprintf(text, sizeof(text), 
            "Font:\n%s\n\nTre dimensioni (8pt, 10pt, 12pt)\nColore rolling %d",
            fontFiles[i], (i % gNumColors) + 1);

        // Disegniamo la scritta 3 volte con dimensioni diverse,
        // partendo dall'alto (y=50) e spaziandole verticalmente
        int startY = 50;
        int currentY = startY;
        int posX = 50; // posizione orizzontale

        // 1) Corpo 8 pt
        int h8 = renderText(renderer, font8,  text, color, posX, currentY, 700);
        currentY += h8 + 20; 

        // 2) Corpo 10 pt
        int h10 = renderText(renderer, font10, text, color, posX, currentY, 700);
        currentY += h10 + 20; 

        // 3) Corpo 12 pt
        int h12 = renderText(renderer, font12, text, color, posX, currentY, 700);
        currentY += h12 + 20; 

        // Mostra a schermo
        SDL_RenderPresent(renderer);

        // Chiudiamo i 3 font
        TTF_CloseFont(font8);
        TTF_CloseFont(font10);
        TTF_CloseFont(font12);

        // Attendi 3 secondi
        SDL_Delay(300);

        // Controlla se l'utente ha chiuso la finestra durante l'attesa
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }
    }

    // Cleanup finale
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();

    return 0;
}
