Zadanie 1 (50%)
Zaimplementuj system wieloprocesowy składający się z:
N producentów (klientów)
M konsumentów (pracowników)
wspólnego bufora o rozmiarze K
Wymagania: Każdy producent:
generuje losowy łańcuch znaków (10 znaków),
zapisuje go do bufora współdzielonego.
Każdy konsument:
pobiera zadanie z bufora,
wypisuje je znak po znaku (opóźnienie 0.3s).
Synchronizacja:
semafor empty
semafor full
semafor mutex
Zadanie 2 (25%)
Dodaj:
2 kolejki:
NORMAL
PRIORITY
30% zadań trafia do PRIORITY
konsumenci zawsze obsługują PRIORITY najpierw
Zadanie 3 (25%)
Dodaj proces Managera, który:
co 5 sekund:
przenosi 1 zadanie z NORMAL → PRIORITY
zapobiega starvation
monitoruje system