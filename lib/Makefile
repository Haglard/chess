###############################################################################
#                              M A K E F I L E
#
# Nome Progetto  : NomeDelProgetto
# Versione       : 1.1
# Data Creazione : 17/12/2024
# Autore         : [Il tuo nome]
#
# Descrizione    : Questo Makefile permette di:
#   - Stampare un messaggio di aiuto se invocato senza target (default: help).
#   - build:    Compilare il target specificato con TARGET=<nome>.
#                Determina dinamicamente i sorgenti in base al target scelto.
#                Genera automaticamente le dipendenze con -MMD -MP.
#                Cerca nei sorgenti del target le stringhe "<nome_modulo_ver> = versione".
#   - clean:    Pulire i file compilati, i .o, i .d, i file delle versioni e la documentazione.
#   - doc:      Generare la documentazione con Doxygen.
#   - backup:   Copiare i file `.c`, `.h` e il Makefile nella directory `backup`.
#   - cloc:     Contare le righe di codice di ogni modulo `.c`, `.h` e Makefile.
#
# Uso Esempio:
#   make build TARGET=obj_mem_test CFLAGS="-DDEBUG"
#   make build TARGET=trace_test CFLAGS="-DOPTIMIZE_DEBUG"
#   make doc
#   make clean
#   make backup
#   make cloc
#
# Version: 1.1
###############################################################################

CC      = gcc
CFLAGS ?= -O2 -Wall -MMD -MP
LDLIBS  =

###############################################################################
# Macro per il Debug della Memoria
#
# La macro OBJ_MEM_DEBUG_ON può essere definita utilizzando il flag -D durante
# la compilazione per abilitare funzionalità avanzate di debug della memoria.
#
# Esempio di utilizzo:
#   make build TARGET=obj_mem_test CFLAGS="-DOBJ_MEM_DEBUG_ON"
#
# Effetti dell'attivazione di OBJ_MEM_DEBUG_ON:
#   - Vengono attivate stampe diagnostiche per il tracciamento delle allocazioni
#     e deallocazioni di memoria.
#   - Viene abilitata la verifica di errori comuni di gestione della memoria,
#     come buffer overflow o double free.
#   - Le funzioni specifiche per il debug della memoria, se presenti nel codice,
#     vengono abilitate.
#
# Nota:
#   L'utilizzo di questa macro potrebbe influire sulle prestazioni del programma,
#   pertanto dovrebbe essere abilitata solo in ambienti di sviluppo e test.
#
###############################################################################
# Macro per l'ottimizzazione del Debug
#
# La macro OPTIMIZE_DEBUG può essere definita utilizzando il flag -D durante
# la compilazione per rimuovere completamente dal codice tutte le istruzioni
# di debug (es. printf di debug).
#
# Esempio di utilizzo:
#   make build TARGET=obj_mem_test CFLAGS="-DOPTIMIZE_DEBUG"
#
# Effetti dell'attivazione di OPTIMIZE_DEBUG:
#   - Le righe di codice condizionate da `#ifdef DEBUG` vengono rimosse.
#   - Riduce la dimensione del binario finale.
#
# Nota:
#   Usare questa macro solo per le build di produzione.
#
###############################################################################

###############################################################################
# Selezione dinamica dei sorgenti in base al TARGET
#
# Se TARGET=obj_mem_test: utilizza obj_mem.c, obj_mem_test.c e trace.c
# Se TARGET=trace_test  : utilizza trace.c e trace_test.c
# Se TARGET=test_obj_dynamic_vector:
#    utilizza obj_trace.c obj_ver.c obj_mem.c obj_dynamic_vector.c test_obj_dynamic_vector.c 
###############################################################################
ifeq ($(TARGET),obj_mem_test)
  TARGET_SRCS = obj_mem.c obj_mem_test.c obj_trace.c obj_ver.c minimax.c obj_dynamic_vector.c obj_cache.c
else ifeq ($(TARGET),trace_test)
  TARGET_SRCS = obj_trace.c trace_test.c obj_ver.c
else ifeq ($(TARGET),test_obj_dynamic_vector)
  TARGET_SRCS = obj_trace.c obj_ver.c obj_mem.c obj_dynamic_vector.c test_obj_dynamic_vector.c
else ifeq ($(TARGET),test_obj_cache)
  TARGET_SRCS = obj_trace.c obj_ver.c obj_mem.c obj_cache.c test_obj_cache.c
else ifeq ($(TARGET),test_minimax)
  TARGET_SRCS = obj_trace.c obj_ver.c obj_mem.c obj_cache.c obj_dynamic_vector.c minimax.c test_minimax.c
else ifeq ($(TARGET),tictactoe)
  TARGET_SRCS = obj_trace.c obj_ver.c obj_mem.c obj_cache.c obj_dynamic_vector.c minimax.c tictactoe.c
else ifeq ($(TARGET),forza4)
  TARGET_SRCS = obj_trace.c obj_ver.c obj_mem.c obj_cache.c obj_dynamic_vector.c minimax.c forza4.c
else ifeq ($(TARGET),test_chess_state)
  TARGET_SRCS = obj_trace.c \
  				obj_ver.c \
				obj_mem.c \
				obj_cache.c \
				obj_dynamic_vector.c \
				minimax.c \
				chess_state.c \
				chess_hash.c \
				chess_game_descriptor.c \
				chess_game_dynamics.c \
				test_chess_state.c
else ifeq ($(TARGET),test_chess_moves)
  TARGET_SRCS = obj_trace.c \
  				obj_ver.c \
				obj_mem.c \
				obj_cache.c \
				obj_dynamic_vector.c \
				minimax.c \
				chess_state.c \
				chess_moves.c \
				chess_moves_pawn.c \
				chess_moves_knight.c \
				chess_moves_bishop.c \
				chess_moves_rook.c \
				chess_moves_queen.c \
				chess_moves_king.c \
				chess_hash.c \
				chess_game_descriptor.c \
				chess_game_dynamics.c \
				test_chess_moves.c
else ifeq ($(TARGET),test_chess_moves_module)
  TARGET_SRCS = obj_trace.c \
  				obj_ver.c \
				obj_mem.c \
				obj_cache.c \
				obj_dynamic_vector.c \
				minimax.c \
				chess_state.c \
				chess_moves.c \
				chess_moves_pawn.c \
				chess_moves_knight.c \
				chess_moves_bishop.c \
				chess_moves_rook.c \
				chess_moves_queen.c \
				chess_moves_king.c \
				chess_hash.c \
				chess_game_descriptor.c \
				chess_game_dynamics.c \
				test_chess_moves_module.c 
else ifeq ($(TARGET),test_chess_apply_moves)
  TARGET_SRCS = obj_trace.c \
  				obj_ver.c \
				obj_mem.c \
				obj_cache.c \
				obj_dynamic_vector.c \
				minimax.c \
				chess_state.c \
				chess_moves.c \
				chess_moves_pawn.c \
				chess_moves_knight.c \
				chess_moves_bishop.c \
				chess_moves_rook.c \
				chess_moves_queen.c \
				chess_moves_king.c \
				chess_hash.c \
				chess_game_descriptor.c \
				chess_game_dynamics.c \
				test_chess_apply_moves.c 
else ifeq ($(TARGET),test_chess_c_vs_c)
  TARGET_SRCS = obj_trace.c \
  				obj_ver.c \
				obj_mem.c \
				obj_cache.c \
				obj_dynamic_vector.c \
				minimax.c \
				chess_state.c \
				chess_moves.c \
				chess_moves_pawn.c \
				chess_moves_knight.c \
				chess_moves_bishop.c \
				chess_moves_rook.c \
				chess_moves_queen.c \
				chess_moves_king.c \
				chess_hash.c \
				chess_game_descriptor.c \
				chess_game_dynamics.c \
				test_chess_c_vs_c.c 
else ifneq ($(TARGET),)
  # Se il TARGET è definito ma non riconosciuto
  $(error "Target sconosciuto: $(TARGET)")
endif

###############################################################################
# Da TARGET_SRCS ricaviamo gli oggetti
###############################################################################
TARGET_OBJS = $(TARGET_SRCS:.c=.o)

###############################################################################
# Target di default
###############################################################################
default: help

###############################################################################
# help: Mostra un messaggio di aiuto
###############################################################################
help:
	@echo "============================================================"
	@echo "                    M A K E F I L E   H E L P               "
	@echo "============================================================"
	@echo "Utilizzo:"
	@echo "  make help                        -> Mostra questo messaggio"
	@echo "  make build TARGET=<nome>         -> Compila il target specificato"
	@echo "     Esempio: make build TARGET=obj_mem_test"
	@echo "     Possibile passare macro a CFLAGS, es:"
	@echo "              make build TARGET=obj_mem_test CFLAGS='-DDEBUG'"
	@echo "              make build TARGET=obj_mem_test CFLAGS='-DOBJ_MEM_DEBUG_ON'"
	@echo "              make build TARGET=obj_mem_test CFLAGS='-DOPTIMIZE_DEBUG'"
	@echo "  make clean                       -> Pulisce i file compilati e la documentazione"
	@echo "  make doc                         -> Genera la documentazione con Doxygen"
	@echo "  make backup                      -> Copia .c, .h e Makefile nella dir 'backup'"
	@echo "  make cloc                        -> Conta le righe di codice di ogni modulo .c, .h e Makefile con cloc"
	@echo "============================================================"

###############################################################################
# scan_versions: Genera dinamicamente obj_ver.c con le versioni dei moduli
###############################################################################
scan_versions:
	@echo "Generazione dinamica di obj_ver.c con le versioni dei moduli..."

	# Calcola la nuova versione
	@if [ -f obj_ver.c ]; then \
		VERSION=$$(grep '##VERSION##' obj_ver.c | sed -n 's/.*##VERSION##.*obj_ver\.c \([0-9]\+\.[0-9]\+\).*/\1/p'); \
		MAJOR=$$(echo $$VERSION | cut -d. -f1); \
		MINOR=$$(echo $$VERSION | cut -d. -f2); \
		if [ -z "$$MAJOR" ] || [ -z "$$MINOR" ]; then \
			NEW_VERSION="1.0"; \
		else \
			NEW_MINOR=$$((MINOR + 1)); \
			NEW_VERSION="$$MAJOR.$$NEW_MINOR"; \
		fi; \
	else \
		NEW_VERSION="1.0"; \
	fi; \
	echo "// Questo file è generato automaticamente dal Makefile" > obj_ver.c; \
	echo "/*" >> obj_ver.c; \
	echo " * ##VERSION## \"obj_ver.c $$NEW_VERSION\"" >> obj_ver.c; \
	echo " */" >> obj_ver.c; \
	echo "#include \"obj_ver.h\"" >> obj_ver.c; \
	echo "#include <stdio.h>" >> obj_ver.c; \
	echo "" >> obj_ver.c; \
	echo "const char *module_versions[] = {" >> obj_ver.c; \
	grep -h '##VERSION##' $(TARGET_SRCS) *.h | \
	grep -E '##VERSION## *\"[a-zA-Z0-9_]+\.[ch] [0-9]+\.[0-9]+\"' | \
	sed 's|.*##VERSION## *\"\(.*\)\".*|    \"\1\",|' >> obj_ver.c; \
	echo "    \"obj_ver.c $$NEW_VERSION\"," >> obj_ver.c; \
	echo "    NULL" >> obj_ver.c; \
	echo "};" >> obj_ver.c; \
	echo "" >> obj_ver.c; \
	echo "void print_versions(void) {" >> obj_ver.c; \
	echo "    printf(\"=== Moduli e Versioni ===\\\\n\");" >> obj_ver.c; \
	echo "    for (int i = 0; module_versions[i] != NULL; i++) {" >> obj_ver.c; \
	echo "        printf(\"%s\\\\n\", module_versions[i]);" >> obj_ver.c; \
	echo "    }" >> obj_ver.c; \
	echo "    printf(\"=========================\\\\n\");" >> obj_ver.c; \
	echo "}" >> obj_ver.c

###############################################################################
# build: Compila il target specificato nella variabile TARGET
#
# Se TARGET non è specificato, errore. Se è riconosciuto, compila il target.
#
# Una volta compilato:
# - Cerca nei sorgenti associati a quel target stringhe '<nome_modulo_ver> = versione'
# - Le salva in <target>.versions e le stampa a schermo
###############################################################################
build: scan_versions
	@if [ -z "$(TARGET)" ]; then \
	  echo "Errore: Specificare un target con TARGET=<nome>. Esempio:"; \
	  echo "  make build TARGET=obj_mem_test"; \
	  echo "  make build TARGET=trace_test"; \
	  echo "  make build TARGET=test_obj_dynamic_vector"; \
	  echo "  make build TARGET=test_obj_cache"; \
	  echo "  make build TARGET=test_minimax"; \
	  echo "  make build TARGET=tictactoe"; \
	  echo "  make build TARGET=forza4"; \
	  echo "  make build TARGET=test_chess_moves"; \
	  echo "  make build TARGET=test_chess_state"; \
	  echo "  make build TARGET=test_chess_moves_module"; \
	  echo "  make build TARGET=test_chess_apply_moves"; \
	  echo "  make build TARGET=test_chess_c_vs_c"; \
	  exit 1; \
	fi
	@echo "Compilazione di $(TARGET)..."
	@$(MAKE) $(TARGET)

###############################################################################
# Regola di linking del target: linkage da TARGET_OBJS
###############################################################################
$(TARGET): $(TARGET_OBJS)
	$(CC) $(CFLAGS) $^ $(LDLIBS) -o $@

###############################################################################
# Regola generica per compilare i .c in .o con generazione automatica .d
###############################################################################
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

###############################################################################
# clean: Pulisce i file generati e la documentazione
###############################################################################
clean:
	@echo "Rimozione file compilati e documentazione..."
	rm -f *.o *.d *.versions
	rm -f test_obj_dynamic_vector
	rm -rf doc/*
	rm -rf latex
	rm -rf html
	rm -f *.log
	rm -f test_obj_cache trace_test test_obj_dynamic_vector \
		  test_minimax tictactoe forza4 test_chess_moves \
		  test_chess_state test_chess_moves_module \
		  test_chess_apply_moves test_chess_c_vs_c

###############################################################################
# doc: Genera documentazione con Doxygen
###############################################################################
doc:
	@echo "Generazione documentazione..."
	doxygen Doxyfile

###############################################################################
# backup: Copia i file sorgenti .c, .h e il Makefile nella directory backup/
###############################################################################
backup:
	@echo "Eseguo il backup dei file .c, .h e Makefile nella directory 'backup'..."
	mkdir -p ../backup/backup
	cp *.c *.h Makefile ../backup/backup

###############################################################################
# cloc: Conta le righe di codice di ogni modulo .c, .h e Makefile con cloc
#
# Utilizzo:
#   make cloc
#
# Requisiti:
#   - Installare cloc (https://github.com/AlDanial/cloc)
#
# Descrizione:
#   Questo target utilizza il comando `cloc` per contare le righe di codice
#   di ogni file `.c`, `.h` e del Makefile, mostrando un riepilogo dettagliato.
###############################################################################
cloc:
	@echo "Contando le righe di codice con cloc..."
	@cloc --by-file *.c *.h Makefile

###############################################################################
# Regola di inclusione delle dipendenze generate
###############################################################################
ifneq ($(TARGET_SRCS),)
-include $(TARGET_OBJS:.o=.d)
endif


