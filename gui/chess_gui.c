#include <stdio.h>
#include <string.h>

#include "obj_trace.h"
#include "chess_gui.h"
#include "chess_to_gui_interface.h"  // dove sono definiti ChessGuiPieces_t, PiecesTextures, BoardTexture
                                     // e la firma di ChessGui_LoadAllTextures(...)

/* ------------------------------------------------------------------
   Dichiarazione/definizione di loadTexture, così non è più "implicit"
   ------------------------------------------------------------------ */
static SDL_Texture* loadTexture(SDL_Renderer* renderer, const char* filePath)
{
    // Carichiamo l’immagine da file
    SDL_Surface* surface = IMG_Load(filePath);
    if (!surface) {
        fprintf(stderr, "Impossibile caricare immagine %s: %s\n", filePath, IMG_GetError());
        return NULL;
    }
    // Creiamo la texture dallo surface
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);

    if (!texture) {
        fprintf(stderr, "Impossibile creare texture da %s: %s\n", filePath, SDL_GetError());
    }
    return texture;
}

/* ------------------------------------------------------------------
   Definizioni di variabili globali
   ------------------------------------------------------------------ */

PiecesTexture_t PiecesTextures; // definizione della variabile globale
SDL_Texture* BoardTexture = NULL; // definizione e inizializzazione a NULL

// Definizione delle variabili globali per i colori
static GuiColor LightColor = {222, 184, 135, 220};
static GuiColor DarkColor  = {160,  82,  45, 220};
static GuiColor BackGroundColor = {40, 40, 40, 255};

/**
 * @brief Restituisce il Light Color delle caselle bianche.
 */
GuiColor getLightColor(void)
{
    return LightColor;
}

/**
 * @brief Restituisce il Dark Color delle caselle nere.
 */
GuiColor getDarkColor(void)
{
    return DarkColor;
}

/**
 * @brief Restituisce BackGroundColor.
 */
GuiColor getBackgroundColor(void)
{
    return BackGroundColor;
}


/* ------------------------------------------------------------------
   ChessGui_LoadAllTextures
   ------------------------------------------------------------------ */

bool ChessGui_LoadAllTextures(SDL_Renderer* renderer)
{
    bool success = true;  // Presumiamo che tutto vada bene

    // 1) Carichiamo la board
    TRACE_DEBUG(&stdtrace, "Caricamento board texture: ./images/board.png");
    BoardTexture = loadTexture(renderer, "./images/board.png");
    if (!BoardTexture) {
        TRACE_ERROR(&stdtrace, "Impossibile caricare board.png");
        success = false;
    } else {
        TRACE_DEBUG(&stdtrace, "BoardTexture caricato con successo (%p)", (void*)BoardTexture);
    }

    // 2) Array di stringhe per TUTTI i valori dell'enum 0..12 (CHESS_GUI_PIECE_COUNT = 13):
    // Indice 0 => CHESS_GUI_EMPTY => nessun file => NULL
    // Indice 1..12 => i file .png corrispondenti
    static const char* pieceFilenames[CHESS_GUI_PIECE_COUNT] = {
        NULL,          // 0 = CHESS_GUI_EMPTY, nessuna texture
        "pawnW.png",   // 1 = CHESS_GUI_PAWN_WHITE
        "rookW.png",   // 2 = CHESS_GUI_ROOK_WHITE
        "knightW.png", // 3 = CHESS_GUI_KNIGHT_WHITE
        "bishopW.png", // 4 = CHESS_GUI_BISHOP_WHITE
        "queenW.png",  // 5 = CHESS_GUI_QUEEN_WHITE
        "kingW.png",   // 6 = CHESS_GUI_KING_WHITE
        "pawnB.png",   // 7 = CHESS_GUI_PAWN_BLACK
        "rookB.png",   // 8 = CHESS_GUI_ROOK_BLACK
        "knightB.png", // 9 = CHESS_GUI_KNIGHT_BLACK
        "bishopB.png", // 10 = CHESS_GUI_BISHOP_BLACK
        "queenB.png",  // 11 = CHESS_GUI_QUEEN_BLACK
        "kingB.png"    // 12 = CHESS_GUI_KING_BLACK
    };

    // 3) Ciclo da 0 a 12
    for (int e = 0; e < CHESS_GUI_PIECE_COUNT; e++) {
        if (e == CHESS_GUI_EMPTY) {
            // CHESS_GUI_EMPTY => nessuna texture
            PiecesTextures.textures[e] = NULL;
            TRACE_DEBUG(&stdtrace, "PiecesTextures.textures[%d] = NULL (CHESS_GUI_EMPTY)", e);
            continue;
        }

        // Altrimenti, pieceFilenames[e] deve essere un nome di file .png
        if (!pieceFilenames[e]) {
            TRACE_ERROR(&stdtrace, "Errore: pieceFilenames[%d] == NULL, impossibile caricare", e);
            PiecesTextures.textures[e] = NULL;
            success = false;
            continue;
        }

        char path[256];
        snprintf(path, sizeof(path), "./images/%s", pieceFilenames[e]);
        TRACE_DEBUG(&stdtrace, "Caricamento piece e=%d => file=%s", e, path);

        // Carichiamo la texture
        PiecesTextures.textures[e] = loadTexture(renderer, path);
        if (!PiecesTextures.textures[e]) {
            TRACE_ERROR(&stdtrace, "Impossibile caricare la texture: %s => textures[%d] = NULL", path, e);
            success = false;
        } else {
            TRACE_DEBUG(&stdtrace, "Caricato con successo: %s => PiecesTextures.textures[%d] = %p",
                        path, e, (void*)PiecesTextures.textures[e]);
        }
    }

    // 4) Restituiamo true se TUTTO è stato caricato correttamente,
    //    false se almeno un caricamento è fallito
    return success;
}


/* ------------------------------------------------------------------
   getTextureByName, getBoardTexture
   ------------------------------------------------------------------ */

SDL_Texture* getTextureByName(const ChessGuiPieces_t chessPiece)
{
    return PiecesTextures.textures[chessPiece];
}

SDL_Texture* getBoardTexture(void)
{
    return BoardTexture; 
}

/* ------------------------------------------------------------------
   initMainWindow, closeSDL
   ------------------------------------------------------------------ */

bool initMainWindow(SDL_Window** window, SDL_Renderer** renderer) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "Errore SDL_Init: %s\n", SDL_GetError());
        return false;
    }

    int imgFlags = IMG_INIT_PNG;
    if ((IMG_Init(imgFlags) & imgFlags) != imgFlags) {
        fprintf(stderr, "Errore IMG_Init: %s\n", IMG_GetError());
        SDL_Quit();
        return false;
    }

    *window = SDL_CreateWindow(
        "Chess GUI",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        SDL_WINDOW_SHOWN
    );
    if (!*window) {
        fprintf(stderr, "Errore SDL_CreateWindow: %s\n", SDL_GetError());
        IMG_Quit();
        SDL_Quit();
        return false;
    }

    *renderer = SDL_CreateRenderer(*window, -1, SDL_RENDERER_ACCELERATED);
    if (!*renderer) {
        fprintf(stderr, "Errore SDL_CreateRenderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(*window);
        IMG_Quit();
        SDL_Quit();
        return false;
    }

    SDL_SetRenderDrawBlendMode(*renderer, SDL_BLENDMODE_BLEND);
    return true;
}

void closeSDL(SDL_Window* window, SDL_Renderer* renderer) {
    if (renderer) {
        SDL_DestroyRenderer(renderer);
    }
    if (window) {
        SDL_DestroyWindow(window);
    }
    IMG_Quit();
    SDL_Quit();
}

/* ------------------------------------------------------------------
   Funzioni di disegno
   ------------------------------------------------------------------ */

void drawBoardEmpty(SDL_Renderer* renderer, GuiColor lightColor, GuiColor darkColor)
{
    // 1) Otteniamo la board via getBoardTexture()
    SDL_Texture* boardTex = getBoardTexture();
    if (boardTex) {
        // Disegno la board (es: in (BOARD_X, BOARD_Y))
        SDL_Rect boardRect = {BOARD_X, BOARD_Y, BOARD_W, BOARD_H};
        SDL_RenderCopy(renderer, boardTex, NULL, &boardRect);
    }

    // 2) Disegno le caselle colorate su 8×8
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            bool isDark = ((row + col) % 2 == 0);
            GuiColor c = isDark ? darkColor : lightColor;

            SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);

            int x = BOARD_X + OFFSET + col * CELL_SIZE;
            int y = BOARD_Y + OFFSET + row * CELL_SIZE;
            SDL_Rect squareRect = { x, y, CELL_SIZE, CELL_SIZE };
            SDL_RenderFillRect(renderer, &squareRect);
        }
    }
}

void drawSinglePiece(SDL_Renderer* renderer, ChessGuiPieces_t chessPiece, int row, int col)
{
    if (chessPiece == CHESS_GUI_EMPTY) {
        // Non disegniamo nulla
        return;
    }

    // Otteniamo la texture del pezzo
    SDL_Texture* tex = getTextureByName(chessPiece);
    if (!tex) {
        // Nessuna texture caricata? Non disegna
        return;
    }

    SDL_Rect dst;
    dst.x = BOARD_X + OFFSET + col * CELL_SIZE;
    dst.y = BOARD_Y + OFFSET + row * CELL_SIZE;
    dst.w = CELL_SIZE;
    dst.h = CELL_SIZE;

    SDL_RenderCopy(renderer, tex, NULL, &dst);
}

void drawPieces(SDL_Renderer* renderer, const ChessGuiBoard_t board)
{
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            ChessGuiPieces_t piece = board[row][col];
            if (piece != CHESS_GUI_EMPTY) {
                drawSinglePiece(renderer, piece, row, col);
            }
        }
    }
}

void drawDeadPiecesTray(SDL_Renderer* renderer, const ChessGuiDeadPieces_t *dp)
{
    // Esempio di aree orizzontali per bianchi e neri
    SDL_Rect whiteTrayRect = { DEAD_WHITE_X, DEAD_WHITE_Y, DEAD_WIDTH, DEAD_HEIGHT };
    SDL_Rect blackTrayRect = { DEAD_BLACK_X, DEAD_BLACK_Y, DEAD_WIDTH, DEAD_HEIGHT };

    // Preleva il colore di sfondo globale (dark grey, ad es.)
    GuiColor bg = getBackgroundColor();
    SDL_SetRenderDrawColor(renderer, bg.r, bg.g, bg.b, bg.a);

    // Riempie le due aree con lo sfondo
    SDL_RenderFillRect(renderer, &whiteTrayRect);
    SDL_RenderFillRect(renderer, &blackTrayRect);

    // Disegniamo i pezzi bianchi “morti”
    for (int i = 0; i < GUI_MAX_DEAD_PIECES; i++) {
        ChessGuiPieces_t piece = dp->dead[GUI_WHITE_PLAYER][i];
        if (piece != CHESS_GUI_EMPTY) {
            SDL_Texture* tex = getTextureByName(piece);
            if (tex) {
                SDL_Rect dst = {
                    DEAD_WHITE_X + i * DEAD_PIECE_SIZE,
                    DEAD_WHITE_Y,
                    DEAD_PIECE_SIZE,
                    DEAD_PIECE_SIZE
                };
                SDL_RenderCopy(renderer, tex, NULL, &dst);
            }
        }
    }

    // Disegniamo i pezzi neri “morti”
    for (int i = 0; i < GUI_MAX_DEAD_PIECES; i++) {
        ChessGuiPieces_t piece = dp->dead[GUI_BLACK_PLAYER][i];
        if (piece != CHESS_GUI_EMPTY) {
            SDL_Texture* tex = getTextureByName(piece);
            if (tex) {
                SDL_Rect dst = {
                    DEAD_BLACK_X + i * DEAD_PIECE_SIZE,
                    DEAD_BLACK_Y,
                    DEAD_PIECE_SIZE,
                    DEAD_PIECE_SIZE
                };
                SDL_RenderCopy(renderer, tex, NULL, &dst);
            }
        }
    }
}

void ChessGui_showUserMessage(SDL_Renderer* renderer, const char* message)
{
    // 1) Pulisce l’area di testo usando il colore di sfondo (BackGroundColor)
    GuiColor bg = getBackgroundColor();
    SDL_SetRenderDrawColor(renderer, bg.r, bg.g, bg.b, bg.a);

    SDL_Rect textAreaRect = {
        TEXT_AREA_X,
        TEXT_AREA_Y,
        TEXT_AREA_WIDTH,
        TEXT_AREA_HEIGHT
    };
    SDL_RenderFillRect(renderer, &textAreaRect);

    // 2) Disegna il “message” usando la primitiva ChessGuiFont_DrawText
    //    - dimensione del font = TEXT_FONT_SIZE
    //    - posizione (TEXT_AREA_X, TEXT_AREA_Y)
    ChessGuiFont_DrawText(renderer, message, TEXT_AREA_X, TEXT_AREA_Y, TEXT_FONT_SIZE);

    // 3) Presentazione immediata (se desiderato)
    SDL_RenderPresent(renderer);
}

static void formatSquare(int c, int r, char* out, size_t outSize)
{
    char file = 'a' + c; 
    char rank = '1' + r;
    snprintf(out, outSize, "%c%c", file, rank);
}

static void buildHistoryFields(const move_record_t* rec,
                               char* progressBuf, size_t progressSize,
                               char* moveBuf,     size_t moveSize,
                               char* captureBuf,  size_t captureSize,
                               char* checkBuf,    size_t checkSize,
                               char* mateBuf,     size_t mateSize,
                               char* drawBuf,     size_t drawSize,
                               char* valueBuf,    size_t valueSize)
{
    // progressivo => "/b.../r" per grassetto
    // es. "/b 12/r"
    snprintf(progressBuf, progressSize, "/b%3d/r", rec->move_id);

    // mossa
    if (rec->castling_right) {
        snprintf(moveBuf, moveSize, "O-O");
    } else if (rec->castling_left) {
        snprintf(moveBuf, moveSize, "O-O-O");
    } else {
        char fromS[4], toS[4];
        formatSquare(rec->from_c, rec->from_r, fromS, sizeof(fromS));
        formatSquare(rec->to_c,   rec->to_r,   toS,   sizeof(toS));
        if (rec->en_passant) {
            snprintf(moveBuf, moveSize, "%s%s e.p.", fromS, toS);
        } else {
            snprintf(moveBuf, moveSize, "%s%s", fromS, toS);
        }
    }

    // cattura => "x" o "-"
    captureBuf[0] = (rec->captured_piece != CHESS_GUI_EMPTY) ? 'x' : '-';
    captureBuf[1] = '\0';

    // check => "c" o "-"
    checkBuf[0] = rec->opponent_under_check ? 'c' : '-';
    checkBuf[1] = '\0';

    // mate => "m" o "-"
    mateBuf[0] = rec->checkmate ? 'm' : '-';
    mateBuf[1] = '\0';

    // draw => "d" o "-"
    drawBuf[0] = rec->draw ? 'd' : '-';
    drawBuf[1] = '\0';

    // value => se != 0 => l’intero, altrimenti "-"
    if (rec->board_status_evaluation != 0) {
        snprintf(valueBuf, valueSize, "%d", rec->board_status_evaluation);
    } else {
        snprintf(valueBuf, valueSize, "-");
    }
}

void ChessGui_showHistory(SDL_Renderer* renderer)
{
    // 1) Puliamo l'area
    GuiColor bg = getBackgroundColor();
    SDL_SetRenderDrawColor(renderer, bg.r, bg.g, bg.b, bg.a);

    SDL_Rect historyArea = {
        HISTORY_X,
        HISTORY_Y,
        HISTORY_WIDTH,
        HISTORY_HEIGHT
    };
    SDL_RenderFillRect(renderer, &historyArea);

    // 2) Quante mosse totali
    int totalMoves = game_history_length();
    if (totalMoves == 0) {
        ChessGuiFont_DrawText(renderer, "No moves", HISTORY_X, HISTORY_Y, HISTORY_FONT_SIZE);
        return;
    }

    // 3) Altezza riga + spacing
    int lineHeight = ChessGuiFont_GetLineHeight(HISTORY_FONT_SIZE);
    if (lineHeight <= 0) lineHeight = HISTORY_FONT_SIZE;
    int perLine = lineHeight + HISTORY_LINE_SPACING;

    // 4) Quante righe massime
    int maxLines = HISTORY_HEIGHT / perLine;
    if (maxLines < 1) return;

    // 5) Indice di partenza
    int startIndex = 0;
    if (totalMoves > maxLines) {
        startIndex = totalMoves - maxLines;
    }

    // 6) Disegno riga per riga
    int posY = HISTORY_Y;
    for (int i = startIndex; i < totalMoves; i++) {
        move_record_t rec;
        if (!game_history_get_move(i, &rec)) {
            continue;
        }

        // Prepara i "campi"
        char progressBuf[16], moveBuf[32], captureBuf[2], checkBuf[2], mateBuf[2], drawBuf[2], valueBuf[16];
        buildHistoryFields(&rec,
                           progressBuf, sizeof(progressBuf),
                           moveBuf, sizeof(moveBuf),
                           captureBuf, sizeof(captureBuf),
                           checkBuf, sizeof(checkBuf),
                           mateBuf, sizeof(mateBuf),
                           drawBuf, sizeof(drawBuf),
                           valueBuf, sizeof(valueBuf));

        // Ora disegniamo ogni campo in una colonna fissa
        // Cfr. definizioni in alto
        ChessGuiFont_DrawText(renderer, progressBuf, HISTORY_COL1_X, posY, HISTORY_FONT_SIZE);
        ChessGuiFont_DrawText(renderer, moveBuf,     HISTORY_COL2_X, posY, HISTORY_FONT_SIZE);
        ChessGuiFont_DrawText(renderer, captureBuf,  HISTORY_COL3_X, posY, HISTORY_FONT_SIZE);
        ChessGuiFont_DrawText(renderer, checkBuf,    HISTORY_COL4_X, posY, HISTORY_FONT_SIZE);
        ChessGuiFont_DrawText(renderer, mateBuf,     HISTORY_COL5_X, posY, HISTORY_FONT_SIZE);
        ChessGuiFont_DrawText(renderer, drawBuf,     HISTORY_COL6_X, posY, HISTORY_FONT_SIZE);
        ChessGuiFont_DrawText(renderer, valueBuf,    HISTORY_COL7_X, posY, HISTORY_FONT_SIZE);

        // Avanziamo
        posY += perLine;
        if (posY > HISTORY_Y + HISTORY_HEIGHT) {
            break;
        }
    }

    SDL_RenderPresent(renderer);
}
