CC      = gcc
CFLAGS  = -Wall -Wextra -std=c11

# Pliki źródłowe funkcji sygnałowych
SRCS    = sig_default.c sig_mask.c sig_ignore.c sig_handle.c sig_unblock.c
OBJS    = $(SRCS:.c=.o)

# Nazwy bibliotek
LIB_STATIC  = libsignals_static.a
LIB_SHARED  = libsignals_shared.so
LIB_DYNAMIC = libsignals_dynamic.so

# Pliki wynikowe (wykonywalne)
BIN_STATIC  = main_static
BIN_SHARED  = main_shared
BIN_DYNAMIC = main_dynamic

# ─────────────────────────────────────────────────────────────────────────────
# Domyślny cel – buduje wszystkie trzy warianty
# ─────────────────────────────────────────────────────────────────────────────
.PHONY: all clean

all: $(BIN_STATIC) $(BIN_SHARED) $(BIN_DYNAMIC)

# ─────────────────────────────────────────────────────────────────────────────
# Obiekty "normalnych" plików .o (do biblioteki statycznej i linkowania shared)
# ─────────────────────────────────────────────────────────────────────────────
%.o: %.c signal_handlers.h
	$(CC) $(CFLAGS) -c $< -o $@

# Obiekty PIC potrzebne do bibliotek współdzielonych
%_pic.o: %.c signal_handlers.h
	$(CC) $(CFLAGS) -fPIC -c $< -o $@

PIC_OBJS = $(SRCS:.c=_pic.o)

# ─────────────────────────────────────────────────────────────────────────────
# 1. Biblioteka STATYCZNA (.a)
# ─────────────────────────────────────────────────────────────────────────────
$(LIB_STATIC): $(OBJS)
	ar rcs $@ $^

$(BIN_STATIC): main.c $(LIB_STATIC) signal_handlers.h
	$(CC) $(CFLAGS) -DUSE_STATIC -o $@ main.c -L. -lsignals_static

# ─────────────────────────────────────────────────────────────────────────────
# 2. Biblioteka WSPÓŁDZIELONA – linkowana dynamicznie przez linker (.so shared)
# ─────────────────────────────────────────────────────────────────────────────
$(LIB_SHARED): $(PIC_OBJS)
	$(CC) -shared -o $@ $^

$(BIN_SHARED): main.c $(LIB_SHARED) signal_handlers.h
	$(CC) $(CFLAGS) -DUSE_SHARED -o $@ main.c -L. -lsignals_shared -Wl,-rpath,.

# ─────────────────────────────────────────────────────────────────────────────
# 3. Biblioteka ŁADOWANA DYNAMICZNIE – dlopen() w czasie działania programu
# ─────────────────────────────────────────────────────────────────────────────
$(LIB_DYNAMIC): $(PIC_OBJS)
	$(CC) -shared -o $@ $^

$(BIN_DYNAMIC): main.c $(LIB_DYNAMIC) signal_handlers.h
	$(CC) $(CFLAGS) -DUSE_DYNAMIC -o $@ main.c -ldl

# ─────────────────────────────────────────────────────────────────────────────
# Czyszczenie
# ─────────────────────────────────────────────────────────────────────────────
clean:
	rm -f $(OBJS) $(PIC_OBJS) \
	      $(LIB_STATIC) $(LIB_SHARED) $(LIB_DYNAMIC) \
	      $(BIN_STATIC) $(BIN_SHARED) $(BIN_DYNAMIC)
