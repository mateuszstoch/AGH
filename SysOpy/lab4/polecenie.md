Zadanie 1. Prosty chat
Napisz prosty program typu klient-serwer, w którym komunikacja zrealizowana jest za pomocą kolejek komunikatów.
Serwer po uruchomieniu tworzy nową kolejkę komunikatów. Za pomocą tej kolejki klienci będą wysyłać komunikaty do serwera. Komunikaty wysyłane przez klientów mogą zawierać polecenie oznaczające pierwsze nawiązanie połaczenia z serwerem (INIT) lub jeśli wcześniej połączenie zostało już nawiązane: identyfikator klienta wraz z komunikatem, który ma zostać przekazany przez serwer do wszystkich pozostałych klientów. W odpowiedzi na polecenie INIT, serwer ma przesłać identyfikator nadany nowemu klientowi.
Klient bezpośrednio po uruchomieniu tworzy kolejkę z unikalnym kluczem IPC i wysyła jej klucz do serwera wraz z komunikatem INIT. Po otrzymaniu takiego komunikatu, serwer otwiera kolejkę klienta, przydziela klientowi identyfikator (np. numer w kolejności zgłoszeń) i odsyła ten identyfikator do klienta (komunikacja w kierunku serwer->klient odbywa się za pomocą kolejki klienta). Po otrzymaniu identyfikatora, klient może wysłać do serwera komunikaty, które serwer będzie przesyłał do wszystkich pozostałych klientów. Komunikaty te są czytane ze standardowego wejścia. Klient po uruchomieniu tworzy drugi proces, który powinien odbierać komunikaty wysyłane przez serwer (przy użyciu kolejki komunikatów klienta) i wyświetlać te komunikaty na standardowym wyjściu.

Klient i serwer należy napisać w postaci osobnych programów. Serwer musi być w stanie pracować jednocześnie z wieloma klientami. Dla uproszczenia można przyjąć, że serwer przechowuje informacje o klientach w statycznej tablicy (rozmiar tablicy ogranicza liczbę klientów, którzy mogą się zgłosić do serwera).

Powyższe zadanie można zrealizować wykorzystując mechanizmy System V lub POSIX.

Zadanie 2.

Prosty chat
Napisz prosty chat typu klient-serwer w którym komunikacja pomiędzy uczestnikami czatu / klientami
/ klientami i serwerem realizowana jest za pośrednictwem socketów z użyciem protokołu
strumieniowego.
Adres / port serwera podawany jest jako argument jego uruchomienia
Klient przyjmuje jako swoje argumenty:
- swoją nazwę/identyfikator (string o z góry ograniczonej długości)
- adres serwera (adres IPv4 i numer portu)
Protokół komunikacyjny powinien obsługiwać następujące operacje:
- LIST:
Pobranie z serwera i wylistowanie wszystkich aktywnych klientów
- 2ALL string:
Wysłania wiadomości do wszystkich pozostałych klientów. Klient wysyła ciąg znaków do
serwera, a serwer rozsyła ten ciąg wraz z identyfikatorem nadawcy oraz aktualną datą do
wszystkich pozostałych klientów
- 2ONE id_klienta string:
Wysłanie wiadomości do konkretnego klienta. Klient wysyła do serwera ciąg znaków podając
jako adresata konkretnego klienta o identyfikatorze z listy aktywnych klientów. Serwer wysyła
ten ciąg wraz z identyfikatorem klienta-nadawcy oraz aktualną datą do wskazanego klienta.
- STOP: Zgłoszenie zakończenia pracy klienta. Powinno skutkować usunięciem klienta z listy
klientów przechowywanej na serwerze
- ALIVE - serwer powinien cyklicznie "pingować" zarejestrowanych klientów, aby zweryfikować
że wciąż odpowiadają na żądania, a jeśli nie - usuwać ich z listy klientów.
- Klient przy wyłączeniu Ctrl+C powinien wyrejestrować się z serwera
Dla uproszczenia można przyjąć, że serwer przechowuje informacje o klientach w statycznej tablicy
(rozmiar tablicy ogranicza liczbę klientów, którzy mogą jednocześnie byc uczestnikami czatu).