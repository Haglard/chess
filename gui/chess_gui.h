#ifndef CHESS_GUI_H
#define CHESS_GUI_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdbool.h>

#include "chess_to_gui_interface.h"
#include "chess_gui_font.h"

/**
 * @brief Larghezza e altezza della finestra principale (in pixel).
 *
 * La finestra conterrà la scacchiera, le aree dei pezzi morti, e altri elementi dell'interfaccia.
 */
#define WINDOW_WIDTH   850  /**< Larghezza della finestra. */
#define WINDOW_HEIGHT  641  /**< Altezza della finestra. */

/**
 * @brief Coordinate e dimensioni dell'area di disegno della scacchiera.
 *
 * La scacchiera vera e propria (board.png + caselle) risiede all'interno di quest'area.
 */
#define BOARD_W        451  /**< Larghezza dell'area scacchiera. */
#define BOARD_H        451  /**< Altezza dell'area scacchiera. */
#define BOARD_X        20   /**< Posizione X (pixel) della scacchiera. */
#define BOARD_Y        20   /**< Posizione Y (pixel) della scacchiera. */

/**
 * @brief Parametri per il disegno delle singole caselle e la griglia di scacchi.
 *
 * \c OFFSET è lo spazio di distacco interno dal bordo della scacchiera all'inizio delle caselle.
 * \c CELL_SIZE è la dimensione di ogni casella in pixel.
 * \c BOARD_ROWS e \c BOARD_COLS determinano il numero di righe e colonne del tabellone (8×8).
 */
#define OFFSET         14   /**< Distacco (in pixel) dal bordo della scacchiera all'area delle caselle. */
#define CELL_SIZE      53   /**< Dimensione di ogni singola casella (in pixel). */
#define BOARD_ROWS     8    /**< Numero di righe della scacchiera (8). */
#define BOARD_COLS     8    /**< Numero di colonne della scacchiera (8). */

/**
 * @brief Dimensioni e posizioni dell'area in cui disegnare i "pezzi morti" (dead pieces).
 *
 * Vengono disegnate due aree orizzontali (una per i bianchi, una per i neri):
 * - \c DEAD_WHITE_X, \c DEAD_WHITE_Y per i bianchi
 * - \c DEAD_BLACK_X, \c DEAD_BLACK_Y per i neri
 * Ciascuna area ha dimensioni \c DEAD_WIDTH × \c DEAD_HEIGHT, e i pezzi sono disegnati con \c DEAD_PIECE_SIZE.
 */
#define DEAD_WIDTH       480 /**< Larghezza dell'area dedicata ai pezzi morti. */
#define DEAD_HEIGHT      30  /**< Altezza dell'area dedicata ai pezzi morti. */
#define DEAD_PIECE_SIZE  30  /**< Dimensione di ciascun pezzo "morto" disegnato nell'area. */

#define DEAD_WHITE_X     20  /**< Posizione X (pixel) dell'area dei pezzi bianchi morti. */
#define DEAD_WHITE_Y    (BOARD_Y + BOARD_H + 20) /**< Posizione Y (pixel) dell'area dei pezzi bianchi morti. */

#define DEAD_BLACK_X     20  /**< Posizione X (pixel) dell'area dei pezzi neri morti. */
#define DEAD_BLACK_Y    (DEAD_WHITE_Y + DEAD_HEIGHT + 20) /**< Posizione Y (pixel) dell'area dei pezzi neri morti. */

/**
 * @brief Definizioni per l'area di testo "font 12" a 20 px dal bordo sinistro
 *        e 20 px sotto il tray dei pezzi neri.
 */
#define TEXT_FONT_SIZE   20          /**< Dimensione (in pixel) del font aggiuntivo. */
#define TEXT_AREA_X      20          /**< X a 20 px dal bordo sinistro. */
#define TEXT_AREA_Y     (DEAD_BLACK_Y + DEAD_HEIGHT + 20) 
                                     /**< 20 px sotto il tray nero. */

#define TEXT_AREA_WIDTH  (WINDOW_WIDTH - TEXT_AREA_X - 20) 
         /**< Ampiezza, se vuoi definire un'area fissa, ad es. (720 - 20 - 20) = 680. */
#define TEXT_AREA_HEIGHT 50
         /**< Un'area di 50 px di altezza, a piacere. */

// Definizioni per l'area "history"
#define HISTORY_X       (BOARD_X + BOARD_W + 20) // 20 px a destra della scacchiera
#define HISTORY_Y       20                       // 20 px dall'alto
#define HISTORY_WIDTH   (WINDOW_WIDTH - HISTORY_X - 20) 
                      // ad es. 720 - 491 - 20 = 209 (se BOARD_X=20, BOARD_W=451)
#define HISTORY_HEIGHT  BOARD_H                  // uguale all'altezza scacchiera, 451

#define HISTORY_FONT_SIZE  14  // dimensione in pixel del font da usare per l'history
#define HISTORY_LINE_SPACING 5 // 5 px di distanza verticale fra le righe
#define HISTORY_COL1_X  (HISTORY_X + 0)   // progressivo
#define HISTORY_COL2_X  (HISTORY_X + 60)  // mossa
#define HISTORY_COL3_X  (HISTORY_X + 160) // cattura
#define HISTORY_COL4_X  (HISTORY_X + 190) // check
#define HISTORY_COL5_X  (HISTORY_X + 220) // mate
#define HISTORY_COL6_X  (HISTORY_X + 250) // draw
#define HISTORY_COL7_X  (HISTORY_X + 280) // value



/**
 * @struct GuiColor
 * @brief Rappresenta un colore RGBA (Red, Green, Blue, Alpha).
 *
 * Ogni componente è un byte (Uint8), con valore compreso tra 0 e 255.
 * - \c r: componente rossa
 * - \c g: componente verde
 * - \c b: componente blu
 * - \c a: canale alpha (trasparenza), dove 0 = trasparente e 255 = opaco
 */
typedef struct {
    Uint8 r; /**< Componente rossa, [0..255]. */
    Uint8 g; /**< Componente verde, [0..255]. */
    Uint8 b; /**< Componente blu, [0..255]. */
    Uint8 a; /**< Canale alpha (trasparenza), [0..255]. */
} GuiColor;


/**
 * @brief Restituisce il colore chiaro (LightColor).
 * @return Il GuiColor associato a LightColor.
 */
GuiColor getLightColor(void);


/**
 * @brief Restituisce il colore scuro (DarkColor).
 * @return Il GuiColor associato a DarkColor.
 */
GuiColor getDarkColor(void);

/**
 * @brief Restituisce BackGroundColor.
 * @return Il GuiColor associato a BackGroundColor.
 */
GuiColor getBackgroundColor(void);


/**
 * @struct PiecesTexture_t
 * @brief Vettore di SDL_Texture*, uno per ogni valore di ChessGuiPieces_t.
 *
 * Esempio:
 * \code
 * PiecesTextures.textures[CHESS_GUI_PAWN_WHITE] = ...;
 * \endcode
 */
typedef struct PiecesTexture_s {
    SDL_Texture* textures[CHESS_GUI_PIECE_COUNT];
} PiecesTexture_t;


/**
 * @var PiecesTextures
 * @brief Variabile globale (definita nel .c) che contiene le texture
 *        associate ai pezzi, indicizzate da ChessGuiPieces_t.
 */
extern PiecesTexture_t PiecesTextures;


/**
 * @var BoardTexture
 * @brief Variabile globale (definita nel .c) che contiene la texture
 *        della scacchiera (board.png o simili).
 */
extern SDL_Texture* BoardTexture;


/**
 * @brief Carica board.png e le texture dei pezzi (<nomepezzo>W|B.png) 
 *        in BoardTexture e PiecesTextures. Restituisce true se tutto ok, false se ci sono errori.
 *
 * CHESS_GUI_EMPTY = 0 => PiecesTextures.textures[0] = NULL,
 * i pezzi bianchi/neri valgono 1..(CHESS_GUI_PIECE_COUNT - 1).
 */
bool ChessGui_LoadAllTextures(SDL_Renderer* renderer);


/**
 * @brief Restituisce la texture associata al pezzo indicato.
 *
 * @param chessPiece Un valore dell'enum ChessGuiPieces_t (ad es. CHESS_GUI_PAWN_WHITE).
 * @return Un puntatore a SDL_Texture* associato a quel pezzo, 
 *         oppure NULL se il pezzo è CHESS_GUI_EMPTY o se non è stato caricato.
 *
 * Questa funzione calcola l'indice corrispondente all'enum (saltando CHESS_GUI_EMPTY)
 * e accede all'array globale PiecesTextures. Se il valore di 'chessPiece' 
 * non rientra nell'intervallo valido, restituisce NULL.
 */
SDL_Texture* getTextureByName(const ChessGuiPieces_t chessPiece);


/**
 * @brief Restituisce la texture della scacchiera.
 *
 * @return SDL_Texture* associato a BoardTexture, oppure NULL se non è stata caricata.
 *
 * Questa funzione semplicemente restituisce la variabile globale BoardTexture
 * (caricata altrove). Se BoardTexture non è stata impostata, restituisce NULL.
 */
SDL_Texture* getBoardTexture(void);


/**
 * @brief Inizializza la finestra principale e il renderer SDL.
 *
 * Crea una finestra di dimensioni predefinite (es. \c WINDOW_WIDTH × \c WINDOW_HEIGHT),
 * e un renderer accelerato. In caso di successo, i parametri \p window e \p renderer
 * verranno assegnati con i relativi oggetti SDL.
 *
 * @param[out] window   Puntatore a \c SDL_Window*, verrà popolato se la funzione ha successo.
 * @param[out] renderer Puntatore a \c SDL_Renderer*, verrà popolato se la funzione ha successo.
 *
 * @return \c true se l’inizializzazione è andata a buon fine, \c false in caso di errore.
 *
 * @note Internamente effettua anche l’inizializzazione di SDL (\c SDL_Init) e SDL_image (\c IMG_Init)
 *       con i flag necessari.
 */
bool initMainWindow(SDL_Window** window, SDL_Renderer** renderer);


/**
 * @brief Chiude e rilascia le risorse SDL.
 *
 * Distrugge \p renderer e \p window, se non \c NULL, e chiude le librerie
 * \c SDL_image e \c SDL. Da invocare al termine del programma
 * per liberare le risorse.
 *
 * @param[in,out] window   Finestra SDL da distruggere.
 * @param[in,out] renderer Renderer SDL da distruggere.
 *
 * @note Dopo la chiamata, i riferimenti a \p window e \p renderer non sono più validi.
 */
void closeSDL(SDL_Window* window, SDL_Renderer* renderer);


/**
 * @brief Disegna la board “vuota” (texture + caselle colorate).
 *
 * Utilizza la variabile globale `BoardTexture` come sfondo,
 * poi disegna caselle chiare e scure basandosi su `lightColor` e `darkColor`.
 *
 * @param renderer    Il renderer SDL.
 * @param lightColor  Colore delle caselle “chiare”.
 * @param darkColor   Colore delle caselle “scure”.
 */
void drawBoardEmpty(SDL_Renderer* renderer, GuiColor lightColor, GuiColor darkColor);


/**
 * @brief Disegna un singolo pezzo `chessPiece` nella cella (row, col).
 *
 * Se `chessPiece` è CHESS_GUI_EMPTY o non ha una texture caricata, non disegna nulla.
 *
 * @param renderer   Il renderer SDL.
 * @param chessPiece Uno dei valori di `ChessGuiPieces_t` (es. CHESS_GUI_PAWN_WHITE).
 * @param row        Riga [0..7].
 * @param col        Colonna [0..7].
 */
void drawSinglePiece(SDL_Renderer* renderer, ChessGuiPieces_t chessPiece, int row, int col);


/**
 * @brief Disegna tutti i pezzi nella scacchiera `board`.
 *
 * Per ogni cella non vuota (CHESS_GUI_EMPTY), richiama `drawSinglePiece`.
 *
 * @param renderer   Il renderer SDL.
 * @param board      Una matrice 8x8 di ChessGuiPieces_t (ChessGuiBoard_t).
 */
void drawPieces(SDL_Renderer* renderer, const ChessGuiBoard_t board);


/**
 * @brief Disegna i dead pieces (bianchi e neri) contenuti in una struttura 2xN.
 *
 * Riempie le due aree dedicate (per i bianchi e i neri), 
 * poi disegna i pezzi in quegli slot chiamando `drawSingleDeadPiece` 
 * o usando una funzione analoga a `drawSinglePiece`.
 *
 * @param renderer    Il renderer SDL.
 * @param dp          Puntatore alla struttura ChessGuiDeadPieces_t (2xN).
 */
void drawDeadPiecesTray(SDL_Renderer* renderer, const ChessGuiDeadPieces_t *dp);


/**
 * @brief Visualizza un messaggio testuale nell'area dedicata (TEXT_AREA_X, TEXT_AREA_Y)
 *        usando la dimensione di font TEXT_FONT_SIZE. 
 *        Può interpretare i "comandi" /b e /r per switchare font bold/regular,
 *        se implementati in ChessGuiFont_DrawText.
 *
 * @param renderer  Il renderer SDL in cui disegnare.
 * @param message   La stringa da disegnare (eventualmente contenente /b e /r).
 *
 * La funzione pulisce l'area di testo, disegna la stringa e, 
 * facoltativamente, fa SDL_RenderPresent (a seconda di come la implementi).
 */
void ChessGui_showUserMessage(SDL_Renderer* renderer, const char* message);


/**
 * @brief Mostra lo storico delle mosse accanto alla scacchiera, all'interno di un'area dedicata.
 *
 * L'area di disegno inizia a 20px a destra della scacchiera (HISTORY_X) e 
 * 20px dall'alto (HISTORY_Y), e si estende in verticale per un'altezza 
 * pari a \c BOARD_H (HISTORY_HEIGHT).
 *
 * La funzione calcola quante righe di testo possono stare in 
 * quell'area, basandosi sull'altezza di ogni riga (font di 18 pixel, 
 * più uno spacing fra righe), e mostra di conseguenza le ultime N mosse
 * che ci entrano. Le mosse sono recuperate tramite le API di game_history.
 *
 * Ogni riga mostra i campi: <progressivo> <mossa> <cattura> <check> <checkmate> <draw> <valore>.
 * - progressivo: indice 1-based della mossa (allineato a destra su 3 cifre)
 * - mossa: a1b2, a1b2 e.p., O-O, O-O-O (dipende da castling/enpassant)
 * - cattura: 'x' o '-' a seconda se c'è una cattura
 * - check: 'c' o '-'
 * - checkmate: 'm' o '-'
 * - draw: 'd' o '-'
 * - valore: se diverso da 0, l'intero; altrimenti '-'
 *
 * @param renderer Il renderer SDL su cui disegnare le righe di testo.
 */
void ChessGui_showHistory(SDL_Renderer* renderer);



#endif // CHESS_GUI_H
