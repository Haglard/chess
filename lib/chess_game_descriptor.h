/******************************************************************************
# ##VERSION## "chess_game_descriptor.h 1.0"
#
# Nome Progetto  : ChessEngine
# Versione       : 1.0
# Data Creazione : 17/12/2024
# Autore         : [Il tuo nome]
#
# Descrizione    : Questo file header definisce l'interfaccia per il descrittore
#                  di gioco degli scacchi. Fornisce la dichiarazione della funzione
#                  per l'inizializzazione del descrittore, che include le
#                  strutture dati necessarie per rappresentare lo stato del gioco.
#
# Dipendenze     : minimax.h
#
# Funzionalità   :
#   - Inizializza il descrittore di gioco per gli scacchi.
#
# Uso:
#   - Includere questo header nei file che necessitano di utilizzare il descrittore
#     di gioco.
#   - Chiamare `initialize_chess_game_descriptor()` per ottenere un nuovo
#     descrittore di gioco inizializzato.
#
# Esempio:
#   #include "chess_game_descriptor.h"
#
#   int main() {
#       game_descriptor_t* game_desc = initialize_chess_game_descriptor();
#       if (!game_desc) {
#           // Gestione errore
#       }
#       // Utilizzo del descrittore di gioco
#       return 0;
#   }
#
******************************************************************************/

#ifndef CHESS_GAME_DESCRIPTOR_H
#define CHESS_GAME_DESCRIPTOR_H

#include "minimax.h"

/**
 * @brief Inizializza il descrittore di gioco per gli scacchi.
 *
 * Questa funzione alloca e configura un nuovo descrittore di gioco per gli
 * scacchi, impostando lo stato iniziale della scacchiera, i parametri dell'algoritmo
 * minimax e altre proprietà necessarie per gestire il gioco.
 *
 * @return Un puntatore a `game_descriptor_t` inizializzato per il gioco degli scacchi.
 *         In caso di errore durante l'allocazione della memoria, ritorna `NULL`.
 *
 * @note
 *   - È responsabilità dell'utente liberare la memoria allocata per il descrittore
 *     di gioco una volta che non è più necessario.
 *   - Assicurarsi che tutte le dipendenze, come `minimax.h`, siano correttamente incluse.
 */
game_descriptor_t* initialize_chess_game_descriptor();

#endif /* CHESS_GAME_DESCRIPTOR_H */
