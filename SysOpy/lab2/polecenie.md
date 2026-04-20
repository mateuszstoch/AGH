Zadanie 1 (50%)
Napisz program demonstrujący różne reakcje na sygnał SIGUSR1. Program:
Ustawia reakcję na sygnał określoną w argumencie wywołania programu — wywołuje jedną z funkcji: sig_default(), sig_mask(), sig_ignore() albo sig_handle() — patrz opis poniżej.
Wypisuje, co sekundę, kolejne wartości zmiennej i — wartość początkowa to 1, a końcowa, to 20.
Jeżeli wartość zmiennej i wynosi 5 lub 15, to:
Wypisuje komunikat "Wysyłam sygnał USR1"
Przy użyciu funkcji raise() wysyła wspomniany sygnał.
Jeżeli wartość zmiennej i wynosi 10, to sprawdza, czy sygnał SIGUSR1 oczekuje (ang. pending) na dostarczenie do procesu. Jeśli tak, to:
Wypisuje komunikat "Odblokowuję USR1".
Wywołuje funkcję sig_unblock() — zadaniem tej funkcji jest odblokowanie, oczekującego, sygnału SIGUSR1.
Po zakończeniu wykonywania ww. pętli wypisuje Pętla została wykonana w całości.
Reakcja na sygnał powinna zależeć od wartości argumentu z linii poleceń. Argument ten może przyjmować wartości:
default
Ustawia domyślną reakcję na sygnał — zaimplementuj funkcję sig_default().
mask
Maskuje / Blokuje sygnał — zdefiniuj funkcję sig_mask().
ignore
Ustawia ignorowanie sygnału — zdefiniuj funkcję sig_ignore().
handle
Instaluje handler obsługujący sygnał — zdefiniuj funkcję sig_handle(), która podpina funkcję zdefiniowaną przez Ciebie — handler (podpięta funkcja) ma wypisywać komunikat "Wywołano handler dla sygnału <nr>", gdzie nr jest numerem, bieżąco, obsługiwanego sygnału.
Przykłady wywołania
$ ./main
Wywołanie: ./main default|mask|ignore|handle

$./main ignore
Wywołano funkcję 'sig_ignore()'
1
2
3
...
19
20
Pętla została wykonana w całości
Zadanie 2 (25%)
Zmodyfikuj zadanie 1:
Wykonaj komendę cp ../zad1/main.c ./child.c.
Dodaj, do child.c, obsługę sygnału USR2.
Utwórz plik main.c i przenieś do niego obsługę linii komend (z child.c).
Program main po rozpoznaniu opcji linii komend:
Tworzy proces potomny child.
Wysyła do programu potomnego, za pomocą sigqueue(), sygnał USR2 wraz z informacją o typie reakcji: default, ignore ...
Program main wywołujemy jak poprzednio.
Zadanie 3 (25%)
Przenieś definicję funkcji sig_default(), sig_mask(), sig_ignore() oraz sig_handle() do odrębnych plików, odpowiednio: sig_default.c, sig_mask.c, sig_ignore.c oraz sig_handle.c.
Utwórz plik Makefile zawierający reguły tworzenia (z powyższych plików) biblioteki:
Statycznej.
Współdzielonej (linkowanej dynamicznie).
Ładowanej dynamicznie. Do realizacji tego rodzaju biblioteki użyj definicji stałej (-D) i dyrektywy preprocesora, aby zmodyfikować sposób działania programu głównego.
Biblioteka ma być generowana pod warunkiem, że: dowolny z plików źródłowych został zmodyfikowany lub nie istnieje plik z biblioteką; kompilowane mają być tylko te części projektu, które uległy zmianie.