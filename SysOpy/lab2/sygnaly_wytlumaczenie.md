# Sygnały w systemie Linux — od podstaw

## Czym w ogóle jest sygnał?

Sygnał to **powiadomienie wysyłane do procesu** przez system operacyjny lub inny proces.  
Można to porównać do dzwonka do drzwi — nie wiesz kiedy zadzwoni, możesz otworzyć drzwi, zignorować dzwonek albo go zablokować (np. wyjąć baterie).

> [!NOTE]
> Sygnał **nie przenosi żadnych danych** (poza numerem) — to tylko informacja że coś się wydarzyło.  
> Wyjątek: `sigqueue()` na Linuxie pozwala dołączyć jedną liczbę całkowitą.

Przykłady z życia:
- Wciskasz `Ctrl+C` w terminalu → system wysyła `SIGINT` do procesu
- Proces wykonuje dzielenie przez zero → system wysyła `SIGFPE`
- Wpisujesz `kill 1234` w terminalu → wysyłasz `SIGTERM` do procesu 1234
- System zamknie proces potomny → rodzic dostaje `SIGCHLD`

---

## Numery sygnałów

Każdy sygnał ma numer i symboliczną nazwę. Kilka ważnych:

| Numer | Nazwa | Kiedy się pojawia | Domyślna reakcja |
|---|---|---|---|
| 1 | `SIGHUP` | terminal się zamknął | zakończenie |
| 2 | `SIGINT` | Ctrl+C | zakończenie |
| 3 | `SIGQUIT` | Ctrl+\\ | zakończenie + core dump |
| 9 | `SIGKILL` | `kill -9` | **zabicie (nie da się zablokować!)** |
| 11 | `SIGSEGV` | odwołanie do złej pamięci | zakończenie + core dump |
| 15 | `SIGTERM` | `kill` bez argumentu | zakończenie |
| 10 | `SIGUSR1` | zdefiniowany przez użytkownika | zakończenie |
| 12 | `SIGUSR2` | zdefiniowany przez użytkownika | zakończenie |

`SIGUSR1` i `SIGUSR2` są specjalne — system nigdy ich sam nie wysyła, możesz je wykorzystać do dowolnej komunikacji między procesami.

---

## Co się dzieje gdy sygnał dotrze do procesu?

Jądro sprawdza jaką reakcję ustawił proces i wykonuje ją. Są **4 możliwe reakcje**:

```
Sygnał przybywa
       │
       ▼
┌─────────────────────────────────────────────────────┐
│  Jak zareagować?                                    │
│                                                     │
│  1. DEFAULT  → jądro robi to co zwykle              │
│               (zwykle: zabija proces)               │
│                                                     │
│  2. IGNORE   → sygnał jest wyrzucany, nic się nie   │
│               dzieje                                │
│                                                     │
│  3. MASK     → sygnał czeka w poczekalni (pending), │
│               zostanie dostarczony gdy go           │
│               odblokujesz                           │
│                                                     │
│  4. HANDLE   → twoja własna funkcja (handler)       │
│               zostaje wywołana                      │
└─────────────────────────────────────────────────────┘
```

---

## Jak ustawić reakcję? — `sigaction()`

```c
#include <signal.h>

struct sigaction sa;
sa.sa_handler = SIG_IGN;   // reakcja: ignoruj
sigemptyset(&sa.sa_mask);  // żadne dodatkowe sygnały nie są blokowane podczas obsługi
sa.sa_flags = 0;

sigaction(SIGUSR1, &sa, NULL);
//         ↑            ↑
//         dla tego     NULL = nie interesuje nas poprzednia reakcja
//         sygnału
```

Pole `sa.sa_handler` może przyjąć:
- `SIG_DFL` — domyślna reakcja systemu
- `SIG_IGN` — ignoruj
- `wskaźnik na funkcję` — twój własny handler

### Własny handler

```c
void moj_handler(int signo) {
    // signo = numer sygnału który właśnie przyszedł
    printf("Dostałem sygnał numer %d\n", signo);
}

// rejestracja:
sa.sa_handler = moj_handler;
sigaction(SIGUSR1, &sa, NULL);
```

> [!WARNING]
> W handlerze wolno używać tylko **async-signal-safe** funkcji — czyli bardzo niewiele.
> `printf()` technicznie nie jest bezpieczny w handlerze, ale w laborkach na to przymykamy oko.
> W prawdziwym kodzie produkcyjnym używa się `write()`.

---

## Maskowanie sygnałów — `sigprocmask()`

Maskowanie = **tymczasowe zablokowanie sygnału**. Sygnał nie jest tracony — trafia do zbioru "pending" i czeka aż go odblokujesz.

```
Normalnie:          Z maską:
                    
Sygnał → Handler    Sygnał → [PENDING - czeka]
                                  ↓
                    Odblokujesz → Handler
```

```c
sigset_t maska;
sigemptyset(&maska);          // tworzymy pusty zbiór
sigaddset(&maska, SIGUSR1);   // dodajemy SIGUSR1

// Blokujemy:
sigprocmask(SIG_BLOCK, &maska, NULL);

// ... tu SIGUSR1 nie zostanie dostarczony, tylko zaczeka ...

// Odblokowujemy:
sigprocmask(SIG_UNBLOCK, &maska, NULL);
// → jeśli sygnał czekał, zostanie teraz dostarczony
```

Trzy tryby `sigprocmask()`:
- `SIG_BLOCK` — dodaj do maski (zablokuj)
- `SIG_UNBLOCK` — usuń z maski (odblokuj)
- `SIG_SETMASK` — zastąp całą maskę nowym zbiorem

---

## Sprawdzanie oczekujących sygnałów — `sigpending()`

```c
sigset_t pending;
sigpending(&pending);   // pobiera zbiór sygnałów które czekają

if (sigismember(&pending, SIGUSR1)) {
    printf("SIGUSR1 czeka na dostarczenie!\n");
}
```

**Kiedy sygnał czeka?** Gdy jest zablokowany (umaskowany) i ktoś go wysłał — siedzi w `pending` aż go odblokujesz.

---

## Różnica: IGNORE vs MASK

To jest **kluczowa różnica**, często mylona:

```
IGNORE (SIG_IGN):
  Sygnał przybywa → jądro wyrzuca go do kosza
  Sygnał jest TRACONY na zawsze

MASK (SIG_BLOCK):
  Sygnał przybywa → jądro odkłada go do "poczekalni" (pending)
  Sygnał CZEKA aż go odblokujesz
  Po odblokowaniu → sygnał jest dostarczany normalnie
```

Przykład z labki:
```
Tryb: mask
i=5:  raise(SIGUSR1) → sygnał zablokowany → trafia do PENDING
i=10: sigpending() → widzimy że SIGUSR1 czeka
      → "Odblokowuję USR1"
      → sigprocmask(SIG_UNBLOCK) → SIGUSR1 dostarczony teraz!
      → domyślna reakcja → PROCES SIĘ ZABIJA (SIG_DFL + SIGUSR1 = koniec)

Tryb: ignore
i=5:  raise(SIGUSR1) → sygnał wyrzucony → nic się nie dzieje
i=10: sigpending() → SIGUSR1 nie czeka (już dawno wyrzucony)
      → nic nie robimy
```

---

## Wysyłanie sygnałów

### `raise()` — wyślij do samego siebie

```c
raise(SIGUSR1);
// to samo co:
kill(getpid(), SIGUSR1);
```

### `kill()` — wyślij do innego procesu

```c
kill(pid, SIGUSR1);  // wysyła SIGUSR1 do procesu o danym PID
kill(0, SIGUSR1);    // wysyła do całej grupy procesów
kill(-1, SIGTERM);   // wysyła do wszystkich procesów użytkownika
```

Nazwa `kill` jest myląca — nie zabija procesu, tylko **wysyła sygnał**. Jeśli sygnałem jest `SIGKILL` to faktycznie zabija, ale można wysłać dowolny.

### `sigqueue()` — wyślij z wartością (tylko Linux)

```c
union sigval sv;
sv.sival_int = 42;  // dołączamy liczbę
sigqueue(pid, SIGUSR2, sv);  // wysyłamy SIGUSR2 + liczba 42
```

Żeby odebrać tę wartość handler musi mieć inną sygnaturę + flagę `SA_SIGINFO`:

```c
void handler(int signo, siginfo_t *info, void *ctx) {
    int wartosc = info->si_value.sival_int; // tutaj jest nasza 42
}

struct sigaction sa;
sa.sa_sigaction = handler;  // inne pole niż sa_handler!
sa.sa_flags = SA_SIGINFO;   // kluczowa flaga
sigaction(SIGUSR2, &sa, NULL);
```

---

## `sigsuspend()` — atomowe czekanie na sygnał

Problem: co jeśli sygnał przyjdzie **między** odblokowaniem a zaśnięciem?

```c
// BEZ sigsuspend — race condition!
sigprocmask(SIG_UNBLOCK, &maska, NULL);  // ← sygnał może przyjść TU
pause();  // śpimy... ale sygnał już przyszedł i nie obudzimy się!
```

Rozwiązanie — `sigsuspend()` robi to **atomowo**:

```c
sigsuspend(&stara_maska);
// atomowo:
//   1. zastępuje maskę starej_maska (odblokowanie)
//   2. zasypia
//   3. po obsłużeniu sygnału: przywraca starą maskę i wraca
```

W labce w `child.c` używamy tego żeby bezpiecznie czekać na `SIGUSR2` od rodzica.

---

## Cały przepływ sygnału — od A do Z

```
Ktoś wysyła sygnał (raise/kill/sigqueue)
           │
           ▼
   Jądro sprawdza maskę procesu
           │
    ┌──────┴──────┐
    │ zablokowany │          │ nie zablokowany │
    ▼             ▼          ▼
  pending      [dalej]    sprawdza reakcję
  (czeka)                     │
                       ┌──────┼──────┐
                       │      │      │
                    default ignore handle
                       │      │      │
                    (SIG_DFL)(wyrzuć)(twój handler)
                       │             │
                  zwykle koniec   handler się wykonuje,
                  procesu         potem program wraca
                                  do miejsca przerwania
```

---

## Szybka ściągawka

```c
// === Ustawianie reakcji ===
sigaction(SIGUSR1, &sa, NULL);  // rejestracja

// === Maska (blokowanie) ===
sigset_t s;
sigemptyset(&s);
sigaddset(&s, SIGUSR1);
sigprocmask(SIG_BLOCK,   &s, NULL); // zablokuj
sigprocmask(SIG_UNBLOCK, &s, NULL); // odblokuj

// === Sprawdzanie oczekujących ===
sigset_t p;
sigpending(&p);
sigismember(&p, SIGUSR1); // 1 jeśli czeka

// === Wysyłanie ===
raise(SIGUSR1);             // do siebie
kill(pid, SIGUSR1);         // do innego procesu
sigqueue(pid, SIGUSR2, sv); // do innego + wartość (Linux)

// === Atomowe czekanie ===
sigsuspend(&stara_maska);
```
