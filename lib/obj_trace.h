/**
 * @file obj_trace.h
 * @brief Libreria C per il tracing/logging in programmi.
 *
 * ##VERSION## "obj_trace.h 1.0"
 *
 * Questa libreria fornisce un sistema di logging semplice ma estensibile. È possibile
 * definire uno o più "canali di tracing" (ad es. stdtrace, filetrace), ognuno con un proprio
 * livello di logging, abilitazione e destinazione di output (un FILE*).
 *
 * Caratteristiche principali:
 * - Diversi livelli di log: DEBUG, INFO, WARN, ERROR, FATAL.
 * - Possibilità di associare ogni canale a un differente FILE* (stdout, stderr, file di log, ...).
 * - Possibilità di aprire direttamente un canale su file tramite `trace_open_file_channel()`.
 * - Macro di logging simili a printf (TRACE_DEBUG, TRACE_INFO, ...) con numero variabile di parametri.
 * - Ogni messaggio include timestamp, nome canale, livello, file e numero di linea di origine del log.
 *
 * Uso tipico:
 * - Dichiarare o usare un canale (es. `stdtrace`) come canale di default.
 * - Impostare il livello di logging desiderato su quel canale (`trace_set_channel_level()`).
 * - Scrivere messaggi di log con le macro \c TRACE_DEBUG, \c TRACE_INFO, ecc.
 *
 * API fornite:
 * - \ref trace_level_t: enum con i livelli di logging (DEBUG, INFO, WARN, ERROR, FATAL, DISABLED).
 * - \ref trace_channel_t: struttura che rappresenta un canale di logging.
 * - \c stdtrace: canale standard già inizializzato.
 * - Funzioni di configurazione canali: 
 *   - \ref trace_set_channel_output()
 *   - \ref trace_set_channel_level()
 *   - \ref trace_enable_channel()
 * - Funzioni di inizializzazione di canali file-based:
 *   - \ref trace_open_file_channel()
 *   - \ref trace_close_channel()
 * - Macro di log (da usare nel codice): \c TRACE_DEBUG, \c TRACE_INFO, \c TRACE_WARN, \c TRACE_ERROR, \c TRACE_FATAL
 *
 * Esempio:
 * \code{.c}
 *   trace_set_channel_level(&stdtrace, TRACE_LEVEL_DEBUG);
 *   TRACE_DEBUG(&stdtrace, "Messaggio di debug: valore=%d", 42);
 * \endcode
 *
 * Per aprire un file di log:
 * \code{.c}
 *   trace_channel_t filetrace;
 *   if (trace_open_file_channel(&filetrace, "filetrace", "log.txt", TRACE_LEVEL_INFO, true)) {
 *       TRACE_INFO(&filetrace, "Questo messaggio va sul file log.txt");
 *       trace_close_channel(&filetrace);
 *   }
 * \endcode
 */

#ifndef OBJ_TRACE_H
#define OBJ_TRACE_H

#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>

/**
 * @def OPTIMIZE_DEBUG
 * @brief Se definita, elimina dal codice il trace di DEBUG.
 *
 * Se \c OPTIMIZE_DEBUG è definita, le macro \c TRACE_DEBUG non verranno espanse
 * e i messaggi di debug non verranno generati.
 */
// #define OPTIMIZE_DEBUG 1

/**
 * @enum trace_level_t
 * @brief Livelli di logging supportati.
 *
 * I messaggi con livello inferiore o uguale a quello del canale vengono effettivamente stampati.
 * Ad esempio, se il livello di un canale è \c TRACE_LEVEL_INFO, i messaggi \c DEBUG non verranno mostrati.
 */
typedef enum {
    TRACE_LEVEL_DEBUG = 0, /**< Messaggi di debug dettagliati, generalmente per lo sviluppo. */
    TRACE_LEVEL_INFO,      /**< Messaggi informativi generali. */
    TRACE_LEVEL_WARN,      /**< Messaggi di avviso su possibili problemi. */
    TRACE_LEVEL_ERROR,     /**< Messaggi di errore. */
    TRACE_LEVEL_FATAL,     /**< Messaggi di errore critici, dopo cui il programma potrebbe terminare. */
    TRACE_LEVEL_DISABLED   /**< Nessun messaggio viene stampato a partire da questo livello. */
} trace_level_t;

/**
 * @struct trace_channel_s
 * @brief Rappresenta un canale di logging.
 *
 * Un canale ha un nome, un livello di logging, uno stato (abilitato o disabilitato),
 * un puntatore a FILE* per l'output e un flag per indicare se la libreria possiede tale FILE*.
 *
 * La libreria fornisce un canale standard \c stdtrace già pronto, che scrive su \c stderr.
 */
typedef struct trace_channel_s {
    const char *name;       /**< Nome del canale (ad es. "stdtrace"). */
    FILE *output;           /**< Stream di output associato (es. stderr, file, stdout). */
    trace_level_t level;    /**< Livello corrente del canale. */
    bool enabled;           /**< \c true se il canale è abilitato, \c false altrimenti. */
    bool own_output;        /**< \c true se il canale possiede l'output (aperto da \c trace_open_file_channel). */
} trace_channel_t;

/**
 * @var stdtrace
 * @brief Canale di logging standard fornito dalla libreria.
 *
 * \c stdtrace è un canale già pronto, impostato di default su \c stderr con livello \c TRACE_LEVEL_DEBUG.
 * Può essere riconfigurato con le funzioni di setup (ad es. \ref trace_set_channel_level).
 */
extern trace_channel_t stdtrace;

/**
 * @brief Imposta l'output di un canale su uno stream \c FILE fornito dal chiamante.
 *
 * @param channel  Canale da configurare.
 * @param output   Nuovo \c FILE* su cui scrivere i log.
 *
 * Il canale non acquisisce la proprietà dell'output, quindi non lo chiuderà.
 */
void trace_set_channel_output(trace_channel_t *channel, FILE *output);

/**
 * @brief Imposta il livello di logging di un canale.
 *
 * @param channel Canale da configurare.
 * @param level   Livello di logging desiderato.
 *
 * Se, ad esempio, viene impostato \c TRACE_LEVEL_WARN, i messaggi \c DEBUG e \c INFO non verranno mostrati.
 */
void trace_set_channel_level(trace_channel_t *channel, trace_level_t level);

/**
 * @brief Abilita o disabilita un canale.
 *
 * @param channel Canale da configurare.
 * @param enable  \c true per abilitare, \c false per disabilitare.
 *
 * Un canale disabilitato non stamperà alcun messaggio, indipendentemente dal suo livello.
 */
void trace_enable_channel(trace_channel_t *channel, bool enable);

/**
 * @brief Funzione interna per la stampa del messaggio di log.
 *
 * Non dovrebbe essere chiamata direttamente dall'utente, bensì tramite le macro \c TRACE_*.
 *
 * @param channel Canale su cui loggare.
 * @param level   Livello del messaggio.
 * @param file    Nome del file sorgente (di solito \c __FILE__).
 * @param line    Numero di linea sorgente (di solito \c __LINE__).
 * @param fmt     Stringa di formato stile \c printf.
 * @param ...     Parametri variabili per il formato.
 *
 * @warning \c trace_log utilizza \c vfprintf (o simili) internamente e non valida la correttezza dei formati.
 */
void trace_log(trace_channel_t *channel, 
               trace_level_t level, 
               const char *file, 
               int line, 
               const char *fmt, ...)
    __attribute__((format(printf, 5, 6)));

/**
 * @brief Inizializza un canale di logging per scrivere su un file.
 *
 * @param channel   Puntatore a un canale (già allocato dal chiamante) da inizializzare.
 * @param name      Nome del canale (es. "filetrace").
 * @param filename  Nome del file su cui scrivere.
 * @param level     Livello di logging da impostare.
 * @param enabled   \c true per abilitare subito il canale, \c false per lasciarlo inizialmente disabilitato.
 *
 * @return \c true se il file è stato aperto correttamente, \c false altrimenti.
 *
 * Se la funzione restituisce \c true, \c channel->own_output viene impostato a \c true,
 * indicando che il canale possiede il file e dovrà chiuderlo con \ref trace_close_channel.
 */
bool trace_open_file_channel(trace_channel_t *channel, const char *name, const char *filename, trace_level_t level, bool enabled);

/**
 * @brief Chiude l'output del canale, se il canale lo possiede.
 *
 * @param channel  Il canale da chiudere.
 *
 * Se il canale possiede l'output (\c own_output == \c true) e non è \c stdout / \c stderr,
 * il file viene chiuso. Il canale rimane comunque valido, ma non avrà più un \c FILE* associato.
 */
void trace_close_channel(trace_channel_t *channel);

/**
 * @def TRACE_DEBUG
 * @brief Logga un messaggio di livello \c DEBUG sul canale specificato.
 *
 * Se \c OPTIMIZE_DEBUG è definito, questa macro è disabilitata (non produce codice).
 *
 * @param CH   Puntatore a \c trace_channel_t (canale).
 * @param FMT  Stringa di formato stile \c printf.
 * @param ...  Parametri variabili.
 */

/**
 * @def TRACE_INFO
 * @brief Logga un messaggio di livello \c INFO sul canale specificato.
 *
 * @param CH   Puntatore a \c trace_channel_t (canale).
 * @param FMT  Stringa di formato stile \c printf.
 * @param ...  Parametri variabili.
 */

/**
 * @def TRACE_WARN
 * @brief Logga un messaggio di livello \c WARN sul canale specificato.
 *
 * @param CH   Puntatore a \c trace_channel_t (canale).
 * @param FMT  Stringa di formato stile \c printf.
 * @param ...  Parametri variabili.
 */

/**
 * @def TRACE_ERROR
 * @brief Logga un messaggio di livello \c ERROR sul canale specificato.
 *
 * @param CH   Puntatore a \c trace_channel_t (canale).
 * @param FMT  Stringa di formato stile \c printf.
 * @param ...  Parametri variabili.
 */

/**
 * @def TRACE_FATAL
 * @brief Logga un messaggio di livello \c FATAL sul canale specificato.
 *
 * @param CH   Puntatore a \c trace_channel_t (canale).
 * @param FMT  Stringa di formato stile \c printf.
 * @param ...  Parametri variabili.
 */

#if !defined(OPTIMIZE_DEBUG)
#define TRACE_DEBUG(CH, FMT, ...)  do { \
    if ((CH)->enabled && (CH)->level <= TRACE_LEVEL_DEBUG) \
        trace_log((CH), TRACE_LEVEL_DEBUG, __FILE__, __LINE__, FMT, ##__VA_ARGS__); \
} while(0)
#else
#define TRACE_DEBUG(CH, FMT, ...) do { } while (0)
#endif

#define TRACE_INFO(CH, FMT, ...)   do { \
    if ((CH)->enabled && (CH)->level <= TRACE_LEVEL_INFO) \
        trace_log((CH), TRACE_LEVEL_INFO,  __FILE__, __LINE__, FMT, ##__VA_ARGS__); \
} while(0)

#define TRACE_WARN(CH, FMT, ...)   do { \
    if ((CH)->enabled && (CH)->level <= TRACE_LEVEL_WARN) \
        trace_log((CH), TRACE_LEVEL_WARN,  __FILE__, __LINE__, FMT, ##__VA_ARGS__); \
} while(0)

#define TRACE_ERROR(CH, FMT, ...)  do { \
    if ((CH)->enabled && (CH)->level <= TRACE_LEVEL_ERROR) \
        trace_log((CH), TRACE_LEVEL_ERROR, __FILE__, __LINE__, FMT, ##__VA_ARGS__); \
} while(0)

#define TRACE_FATAL(CH, FMT, ...)  do { \
    if ((CH)->enabled && (CH)->level <= TRACE_LEVEL_FATAL) \
        trace_log((CH), TRACE_LEVEL_FATAL, __FILE__, __LINE__, FMT, ##__VA_ARGS__); \
} while(0)

#endif /* OBJ_TRACE_H */
