# Lab 2 – Sygnały POSIX w C

## Struktura katalogów

```
lab2/
├── zad1/
│   ├── main.c      ← cały program w jednym pliku
│   └── Makefile
├── zad2/
│   ├── main.c      ← proces rodzica (fork + sigqueue)
│   ├── child.c     ← proces potomny (odbiera SIGUSR2, wykonuje pętlę)
│   └── Makefile
└── zad3/
    ├── sig_default.c
    ├── sig_mask.c
    ├── sig_ignore.c
    ├── sig_handle.c
    ├── sig_unblock.c
    ├── signal_handlers.h
    ├── main.c
    └── Makefile
```

---

## Zadanie 1 – Różne reakcje na SIGUSR1

### Co robi program?

Program startuje, ustawia wybraną reakcję na sygnał `SIGUSR1` a potem liczy od 1 do 20 (co sekundę). Na wartościach **5 i 15** wysyła do siebie sygnał przez `raise()`. Na wartości **10** sprawdza czy sygnał oczekuje i ewentualnie go odblokowuje.

### Kluczowe funkcje

| Funkcja | Co robi |
|---|---|
| `sig_default()` | Przywraca domyślną reakcję systemu — `SIG_DFL` (zwykle zabija proces) |
| `sig_mask()` | Blokuje sygnał przez `sigprocmask(SIG_BLOCK,...)` — sygnał czeka w kolejce |
| `sig_ignore()` | Ustawia `SIG_IGN` — jądro wyrzuca sygnał, żadna akcja |
| `sig_handle()` | Podpina własną funkcję `signal_handler()` przez `sigaction()` |
| `sig_unblock()` | Zdejmuje blokadę przez `sigprocmask(SIG_UNBLOCK,...)` |

### Jak działa maskowanie vs ignorowanie?

- **ignore** → sygnał jest **tracony** na zawsze
- **mask** → sygnał **czeka** w zbiorze `pending` — gdy go odblokujesz, zostanie dostarczony

Dlatego przy trybie `mask` na i=10 pojawia się `"Odblokowuję USR1"` i sygnał zostaje wtedy obsłużony.

### Jak działa `sigaction`?

```c
struct sigaction sa;
sa.sa_handler = SIG_IGN;  // co zrobić z sygnałem
sigemptyset(&sa.sa_mask); // które sygnały blokować PODCZAS obsługi (żadne)
sa.sa_flags = 0;
sigaction(SIGUSR1, &sa, NULL); // rejestrujemy dla SIGUSR1
```

### Jak sprawdzić sygnały oczekujące?

```c
sigset_t pending;
sigpending(&pending);                  // pobiera zbiór oczekujących sygnałów
if (sigismember(&pending, SIGUSR1)) {  // sprawdza czy SIGUSR1 jest w zbiorze
    ...
}
```

### Kompilacja i uruchomienie (Linux)

```bash
cd zad1 && make
./main ignore   # lub: default | mask | handle
```

---

## Zadanie 2 – Rodzic i dziecko + sigqueue

### Co nowego w stosunku do zad1?

1. Kod z zad1 przeniesiony do `child.c` (wykonywany jako osobny proces)
2. `child.c` dostał obsługę `SIGUSR2` — odbiera przez niego tryb od rodzica
3. `main.c` = nowy plik z rodzicem który robi `fork()` + `execl()` + `sigqueue()`

### Jak komunikuje się rodzic z dzieckiem?

Zamiast zwykłego `kill()` używamy `sigqueue()` — pozwala przesłać dodatkową **wartość całkowitą** razem z sygnałem:

```c
union sigval sv;
sv.sival_int = mode;  // 0=default, 1=mask, 2=ignore, 3=handle
sigqueue(pid, SIGUSR2, sv);  // wysyłamy SIGUSR2 + wartość do dziecka
```

### Jak dziecko odbiera tę wartość?

Handler musi być zarejestrowany z flagą `SA_SIGINFO`, żeby dostać `siginfo_t`:

```c
struct sigaction sa2;
sa2.sa_sigaction = usr2_handler;  // inna nazwa pola niż sa_handler!
sa2.sa_flags = SA_SIGINFO;        // kluczowe — bez tego nie dostaniemy si_value
sigaction(SIGUSR2, &sa2, NULL);
```

Handler `usr2_handler` ma wtedy dodatkowe parametry:

```c
void usr2_handler(int signo, siginfo_t *info, void *context) {
    int mode = info->si_value.sival_int; // tutaj jest wartość z sigqueue
}
```

### Synchronizacja

Dziecko używa `sigsuspend()` żeby atomowo odblokować `SIGUSR2` i zasnąć — bez ryzyka że sygnał przyjdzie zanim zaczniemy czekać (race condition):

```c
sigprocmask(SIG_BLOCK, &waitset, &oldset); // blokujemy USR2
// ... rejestrujemy handler ...
sigsuspend(&oldset); // atomowo: odblokuj USR2 i czekaj na sygnał
// tu wracamy dopiero po obsłużeniu SIGUSR2
```

### Kompilacja i uruchomienie (Linux)

```bash
cd zad2 && make
./main handle   # lub: default | mask | ignore
```

> [!NOTE]
> `sigqueue()` jest funkcją **tylko linuksową** (POSIX RT signals). Na macOS ta funkcja nie istnieje — kod działa poprawnie na Linuxie.

---

## Zadanie 3 – Biblioteki statyczna, shared i dynamiczna

### Co się zmieniło?

Funkcje `sig_*` zostały przeniesione do oddzielnych plików `.c`. Makefile buduje z nich **trzy rodzaje bibliotek** i **trzy pliki wykonywalne**.

### Trzy rodzaje bibliotek

#### 1. Biblioteka statyczna (`.a`)

```makefile
ar rcs libsignals_static.a sig_default.o sig_mask.o sig_ignore.o sig_handle.o sig_unblock.o
gcc -o main_static main.c -L. -lsignals_static
```

- Kod biblioteki jest **wklejany do pliku wykonywalnego** podczas kompilacji
- Plik `.a` po kompilacji nie jest już potrzebny
- Największy plik wynikowy

#### 2. Biblioteka współdzielona – linkowana przez linker (`.so`)

```makefile
gcc -fPIC -c sig_default.c -o sig_default_pic.o  # PIC = Position Independent Code
gcc -shared -o libsignals_shared.so *_pic.o
gcc -o main_shared main.c -L. -lsignals_shared -Wl,-rpath,.
```

- Kod biblioteki jest **osobnym plikiem** — ładowany przez system gdy program startuje
- `-fPIC` = kod nie zakłada stałych adresów w pamięci (wymagane dla .so)
- `-Wl,-rpath,.` = szukaj .so w bieżącym katalogu (Linux)

#### 3. Biblioteka ładowana dynamicznie – przez `dlopen()` w trakcie działania

```makefile
gcc -DUSE_DYNAMIC -o main_dynamic main.c -ldl  # -ldl = biblioteka dla dlopen
```

Kod programu sam otwiera bibliotekę i szuka funkcji **po nazwie**:

```c
void *handle = dlopen("./libsignals_dynamic.so", RTLD_LAZY);
typedef void (*sig_fn_t)(void);
sig_fn_t fn = (sig_fn_t)dlsym(handle, "sig_default"); // adres funkcji po nazwie
fn();        // wywołujemy
dlclose(handle); // zwalniamy
```

- `RTLD_LAZY` = symbole rozwiązywane dopiero gdy są potrzebne
- Program może załadować bibliotekę z dowolnego miejsca w trakcie działania
- Używane np. w systemach pluginów

### Dyrektywy preprocesora w main.c

Jeden plik `main.c` obsługuje wszystkie 3 tryby dzięki `#ifdef`:

```c
#ifdef USE_DYNAMIC
    // kod z dlopen
#else
    // kod z normalnymi wywołaniami funkcji przez linker
#endif
```

Makefile przekazuje odpowiednią flagę przy kompilacji:
```makefile
gcc -DUSE_DYNAMIC ...  # definiuje USE_DYNAMIC
gcc -DUSE_STATIC  ...  # definiuje USE_STATIC
gcc -DUSE_SHARED  ...  # definiuje USE_SHARED
```

### Reguły Makefile i inkrementalna kompilacja

```makefile
%.o: %.c signal_handlers.h
    $(CC) $(CFLAGS) -c $< -o $@
```

- Make automatycznie wykrywa że plik `.o` jest starszy niż `.c` → rekompiluje tylko zmienione pliki
- `$<` = pierwszy prerequisit (plik .c), `$@` = cel (plik .o)
- Biblioteka jest przebudowywana **tylko gdy zmienił się któryś plik .o**

### Kompilacja i uruchomienie (Linux)

```bash
cd zad3 && make          # buduje wszystkie 3 warianty
./main_static ignore
./main_shared handle
./main_dynamic mask
```

---

## Podsumowanie kluczowych funkcji POSIX

| Funkcja | Co robi |
|---|---|
| `sigaction()` | Rejestruje handler dla sygnału (nowoczesne API, zastępuje `signal()`) |
| `sigprocmask()` | Blokuje/odblokowuje sygnały w procesie |
| `sigpending()` | Zwraca zbiór sygnałów oczekujących na dostarczenie |
| `raise()` | Wysyła sygnał do samego siebie |
| `kill()` | Wysyła sygnał do innego procesu |
| `sigqueue()` | Jak `kill()` ale z dodatkową wartością (Linux only) |
| `sigsuspend()` | Atomowo zmienia maskę i śpi do kolejnego sygnału |
| `fork()` | Tworzy kopię bieżącego procesu |
| `execl()` | Zastępuje bieżący proces nowym programem |
| `waitpid()` | Czeka na zakończenie procesu potomnego |
| `dlopen()` | Otwiera bibliotekę .so w trakcie działania programu |
| `dlsym()` | Pobiera adres symbolu (funkcji/zmiennej) z biblioteki |
| `dlclose()` | Zwalnia bibliotekę |
