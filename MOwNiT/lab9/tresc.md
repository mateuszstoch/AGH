Równania różniczkowe zwyczajne – część I
Zadanie 1. Przedstaw każde z poniższych równań różniczkowych zwyczajnych
jako równoważny układ równań pierwszego rzędu (ang. first-order system of
ODEs):
(a) równanie Van der Pol’a:
y
′′ = y
′
(1 − y
2
) − y.
(b) równanie Blasiusa:
y
′′′ = −yy′′
.
(c) II zasada dynamiki Newtona dla problemu dwóch ciał:
y
′′
1 = −GMy1/(y
2
1 + y
2
2
)
3/2
, (1)
y
′′
2 = −GMy2/(y
2
1 + y
2
2
)
3/2
. (2)
Zadanie 2. Przekształć poniższy problem początkowy do autonomicznego problemu początkowego:
y
′
1 = y1/t + y2t
y
′
2 = t(y
2
2 − 1)/y1
y1(1) = 1
y2(1) = 0
Zadanie 3. Dany jest problem początkowy:
y
′ =
p
1 − y
y(0) = 0
Pokaż, że funkcja y(t) = t(4 − t)/4 spełnia równanie i warunek początkowy
oraz wyznacz dziedzinę, dla której y(t) jest rozwiązaniem problemu początkowego.
Zadanie 4. Dane jest równanie różniczkowe zwyczajne
1
y
′ = −5y
z warunkiem początkowym y(0) = 1. Równanie rozwiązujemy numerycznie z
krokiem h = 0.5.
(a) Analityczna stabilność (uwarunkowanie problemu). Wyjaśnij, czy rozwiązania powyższego równania są stabilne?
(b) Udowodnij, że metoda Euler’a jest zbieżna, tzn., że
lim
h→0
n→∞ nh=t
yn = y(t),
gdzie y(t) oznacza wartość analitycznego rozwiązania w ustalonym punkcie
t = nh, a yn wartość rozwiązania numerycznego w punkcie t.
Wykorzystaj fakt, że limn→∞(1 + 1
n
)
n = limh→0(1 + h)
1
h = e.
(c) Numeryczna stabilność. Wyjaśnij, czy metoda Euler’a jest stabilna dla tego
równania z użytym krokiem h?
(d) Oblicz numerycznie wartości przybliżonego rozwiązania dla t = 0.5 metodą
Euler’a.
(e) Wyjaśnij, czy niejawna metoda Euler’a jest stabilna dla tego równania z
użytym krokiem h?
(f) Oblicz numerycznie wartości przybliżonego rozwiązania dla t = 0.5 niejawną
metodą Euler’a.
(g) Wyznacz maksymalną dopuszczalną wartość kroku h w metodzie Eulera,
jeśli żądamy, aby błąd rozwiązania w punkcie tn = 0.5 nie przekraczał 0.001,
tzn. |yn − y(tn)| < tol = 0.001. Ile kroków należy w tym celu wykonać?
(h) Do wyznaczenia wartości yn+1 w niejawnej metodzie Euler’a użyto metody
bezpośredniej iteracji:
y
(0)
n+1 = yn
y
(k+1)
n+1 = ϕ

y
(k)
n+1
Wyznacz maksymalną dopuszczalną wartość kroku h, przy której metoda
pozostanie zbieżna. Czy uzasadnione byłoby użycie metody Newtona do
wyznaczenia yn+1?
Zadanie 5. Dany jest układ równań różniczkowych zwyczajnych
y
′
1 = −2y1 + y2
y
′
2 = −y1 − 2y2
2
Dla jakich wartości kroku h metoda Euler’a jest stabilna dla tego układu
równań?
Zadanie 6. Dany jest problem początkowy:
y
′ = αtα−1
y(0) = 0
gdzie parametr α > 0. Rozwiązaniem powyższego problemu początkowego jest
funkcja y(t) = t
α.
Rozwiąż powyższy problem metodą Eulera dla α = 2.5, 1.5, 1.1. Dla każdego
problemu zastosuj kroki h = 0.2, 0.1, 0.05, oblicz błąd numeryczny w węzłach
rozwiązania, a następnie wyznacz empiryczny rząd zbieżności metody Eulera.
Wyjaśnij otrzymane wyniki.
3