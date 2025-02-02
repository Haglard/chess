/**
 * ##VERSION## "obj_trace.c 1.0"
*/

#include "obj_trace.h"
#include <time.h>
#include <string.h>
#include <stdlib.h>

/* Canale standard di default */
trace_channel_t stdtrace = {
    .name = "stdtrace",
    .output = NULL,
    .level = TRACE_LEVEL_DEBUG,
    .enabled = true,
    .own_output = false
};

/* Funzione per cambiare l'output di un canale */
void trace_set_channel_output(trace_channel_t *channel, FILE *output) {
    if (channel) {
        channel->output = output;
        channel->own_output = false; /* Non possediamo l'output se lo settiamo da fuori */
    }
}

/* Funzione per cambiare il livello di tracing di un canale */
void trace_set_channel_level(trace_channel_t *channel, trace_level_t level) {
    if (channel) {
        channel->level = level;
    }
}

/* Abilita o disabilita il canale */
void trace_enable_channel(trace_channel_t *channel, bool enable) {
    if (channel) {
        channel->enabled = enable;
    }
}

/* Funzione interna per ottenere una stringa con il timestamp */
static const char* trace_get_timestamp(void) {
    static char buffer[64];
    time_t t = time(NULL);
    struct tm tm_info;
    localtime_r(&t, &tm_info);
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &tm_info);
    return buffer;
}

/* Funzione interna per convertire il livello in stringa */
static const char* trace_level_to_str(trace_level_t level) {
    switch (level) {
        case TRACE_LEVEL_DEBUG: return "DEBUG";
        case TRACE_LEVEL_INFO:  return "INFO";
        case TRACE_LEVEL_WARN:  return "WARN";
        case TRACE_LEVEL_ERROR: return "ERROR";
        case TRACE_LEVEL_FATAL: return "FATAL";
        default:                return "UNKNOWN";
    }
}

/* Funzione di logging con file e linea inclusi */
void trace_log(trace_channel_t *channel, 
               trace_level_t level, 
               const char *file, 
               int line, 
               const char *fmt, ...) 
{
    if (!channel || !channel->enabled || level < channel->level) {
        return;
    }

    FILE *out = channel->output ? channel->output : stderr;
    va_list args;
    va_start(args, fmt);

    fprintf(out, "[%s] [%s] [%s] (%s:%d): ", 
            trace_get_timestamp(), 
            channel->name, 
            trace_level_to_str(level), 
            file, 
            line);
    vfprintf(out, fmt, args);
    fprintf(out, "\n");

    va_end(args);
    fflush(out);
}

/* Inizializza un canale di trace per un file. 
   Se successso, ritorna true. Altrimenti false.
   Il canale è già allocato dal chiamante. */
bool trace_open_file_channel(trace_channel_t *channel, 
                             const char *name, 
                             const char *filename, 
                             trace_level_t level, 
                             bool enabled) 
{
    if (!channel || !filename || !name) {
        return false;
    }

    FILE *f = fopen(filename, "a");
    if (!f) {
        return false;
    }

    channel->name = name;
    channel->output = f;
    channel->level = level;
    channel->enabled = enabled;
    channel->own_output = true; /* Canale possiede il file */

    return true;
}

/* Chiude il canale se possiede l'output */
void trace_close_channel(trace_channel_t *channel) {
    if (!channel) return;
    if (channel->own_output && channel->output && channel->output != stderr && channel->output != stdout) {
        fclose(channel->output);
        channel->output = NULL;
    }
    /* Non deallochiamo il channel, è responsabilità del chiamante */
}
