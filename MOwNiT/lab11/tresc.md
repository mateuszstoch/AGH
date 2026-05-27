Optymalizacja
Zadanie 1. Preconditioning.
Dana jest funkcja f : R2 →R
f(x,y) = 1
2x2 + 9
2y2 . (1)
(a) Znajdź minimum funkcji f metodą gradientu prostego, wyznaczając w każ-
dej iteracji optymalną wartość kroku αk. Narysuj wykres konturowy funkcji
f, na którym zaznacz iteracje algorytmu. Jako punkt startowy przyjmij
(x0,y0) = (9,1).
(b) Oblicz współczynnik uwarunkowania macierzy ∇2f(x,y).
(c) Znajdź macierz L taką, że: ∇2f(x,y) = LLT .
(d) Definiując zmienne x′,y′:
[x′
y′
]
= L
[x
y
]
(2)
otrzymujemy [x
y
]
= L−T
[x′
y′
]
(3)
oraz
g(x′,y′) = f(L−T
[x′
y′
]
) . (4)
Znajdź minimum funkcji g metodą gradientu prostego. Na podstawie wzoru
(3) wyznacz minimum funkcji f.
Zadanie 2. Wiszący łańcuch.
Łańcuch reprezentujemy jako n+1 punktowych mas o współrzędnych (x(i),y(i))
i masie m każda. Każde kolejne dwie masy połączone są sprężyną, której dłu-
gość w stanie spoczynku wynosi L. Współczynnik sprężystości sprężyny wynosi
k. Energia potencjalna sprężyny wynosi
V (x(i),y(i),x(i+1),y(i+1)) = 1
2 k(√(x(i) −x(i+1))2 + (y(i) −y(i+1))2 −L)2
1
Energia potencjalna grawitacji każdej masy wynosi mgy(i). Całkowita ener-
gia potencjalna łańcucha jest równa
V (x(0),...,x(n),y(0),...,y(n)) =
1
2
n−1∑
i=0
k
(√
(x(i) −x(i+1))2 + (y(i) −y(i+1))2 −L
)2
+ g
n∑
i=0
my(i)
Położenie pierwszej i ostatniej masy jest ustalone: (x(0),y(0)) = (0,0), (x(n),y(n)) =
(3,1). Przyjmij następujące wartości: n = 40, L = 1/10 m, m = 1/10 kg, k = 70
N/m, g = 9.81 m/s2.
Znajdź końcowe położenie wszystkich mas łańcucha, tj. takie, które mini-
malizuje jego całkowitą energię potencjalną. W celu minimalizacji całkowitej
energii potencjalnej należy użyć metody gradientu prostego.
Wyznacz wyrażenie na gradient ∇V funkcji celu V względem położeń x(i) i
y(i).
Następnie porównaj trzy wersje algortymu gradientu prostego:
(a) ze stałym współczynnikiem uczenia α
(b) ze współczynnikiem uczenia α malejącym wykładniczo co pewną liczbę ite-
racji
(c) z przeszukiwaniem liniowym.
Stwórz wykres ilustrujący położenie mas łańcucha po przyjęciu minimalnej
energii.
Zadanie 3. Rozwiąż ponownie problem predykcji roku wydania utworu (labo-
ratorium 2), używając metody gradientu prostego (ang. gradient descent ). Stałą
uczącą możesz wyznaczyć na podstawie najmniejszej i największej wartości wła-
snej macierzy AT A reprezentującej cały zbiór treningowy. Porównaj uzyskane
rozwiązanie z metodą najmniejszych kwadratów, biorąc pod uwagę następujące
kryteria:
Dokładność predykcji na zbiorze testowym
Teoretyczną złożoność obliczeniową
Czas obliczeń.