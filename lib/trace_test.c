#include "obj_trace.h"
#include "obj_ver.h"

int main(void) {
    /* Inizializzazione del canale standard: di default manda su stderr */
    trace_set_channel_output(&stdtrace, stderr);
    trace_set_channel_level(&stdtrace, TRACE_LEVEL_DEBUG);

    print_versions();

    TRACE_DEBUG(&stdtrace, "Questo è un messaggio di debug: x=%d", 42);
    TRACE_INFO(&stdtrace, "Questo è un messaggio di info");
    TRACE_WARN(&stdtrace, "Questo è un messaggio di warning");
    TRACE_ERROR(&stdtrace, "Questo è un messaggio di errore");
    TRACE_FATAL(&stdtrace, "Questo è un messaggio FATAL!");

    /* Creiamo un canale di file trace senza allocare dinamicamente, 
       ma tramite una variabile locale */
    trace_channel_t filetrace;

    if (trace_open_file_channel(&filetrace, "filetrace", "mylog.txt", TRACE_LEVEL_INFO, true)) {
        TRACE_DEBUG(&filetrace, "Non verrà stampato perché il livello è INFO");
        TRACE_INFO(&filetrace, "Questo va su file");
        TRACE_ERROR(&filetrace, "Questo va su file come errore");
        
        trace_close_channel(&filetrace);
    } else {
        TRACE_ERROR(&stdtrace, "Impossibile aprire il file per il logging");
    }

    return 0;
}
