Równania różniczkowe zwyczajne – część II
Zadanie 1. Problem trzech ciał (ang. Planar Circular Restricted Three-Body
Problem).
Asteroida o masie pomijalnej w stosunku do masy Ziemi i Księżycą znajduje
się w pozycji (x, y) i porusza się z prędkością (u, v). Stosunek masy Księżyca
do masy układu Ziema-Księżyc wynosi µ = 0.012150582. Przyjęto układ jednostek, w którym odległość Ziema-Księżyc wynosi 1, łączna masa Ziemi i Księżyca
wynosi 1 oraz prędkość kątowa obrotu układu Ziema-Księżyc wokół wspólnego
środa masa wynosi 1. Środek masy układu Ziemia-Księżyc znajduje się w pozycji (0, 0), Ziemia znajduje się w pozycji (−µ, 0), a Księżyc w pozycji (1 − µ, 0).
Odległości asteroidy od Ziemi i Księżyca wynoszą odpowiednio r1 i r2, gdzie
r1 =
p
(x + µ)
2 + y
2
r2 =
p
(x − 1 + µ)
2 + y
2
Ruch asteroidy określony jest prawami dynamiki Newtona z uwzględnieniem
siły odśrodkowej oraz siły Coriolisa. Ruch ten opisany jest układem równań:
x
′ = u (1)
u
′ = x + 2v −

(1 − µ)(x + µ)
r
3
1
+
µ(x − 1 + µ)
r
3
2

(2)
y
′ = v (3)
v
′ = y − 2u −

(1 − µ)y
r
3
1
+
µy
r
3
2

(4)
Położenie początkowe asteroidy (x(0), y(0)) = (1.05, 0.1), prędkość początkowa
(u(0), v(0)) = (−0.45, −0.25). Przyjmij czas symulacji t ∈ [0, 5].
(a) Rozwiąż powyższy układ równań:
 jawną metodą Eulera
yk+1 = yk + hkf(tk, yk)
 niejawną metodą Eulera
yk+1 = yk + hkf(tk+1, yk+1)
1
 półjawną metodę Eulera
xn+1 = xn + hnf(xn, yn+1)
yn+1 = yn + hng(xn, yn+1)
lub
xn+1 = xn + hnf(xn+1, yn)
yn+1 = yn + hng(xn+1, yn)
 metodą Rungego-Kutty czwartego rzędu (RK4)
yk+1 = yk +
hk
6
(k1 + 2k2 + 2k3 + k4), gdzie
k1 = f(tk, yk)
k2 = f(tk + hk/2, yk + hkk1/2)
k3 = f(tk + hk/2, yk + hkk2/2)
k4 = f(tk + hk, yk + hkk3)
W podpunktach (b)–(e) zastosuj przynajmniej metodę RK4.
W podpunktach (f)–(g) zastosuj wszystkie metody.
(b) Zaznacz położenie Ziemi, Księżyca oraz położenie punktów libracyjnych Lagrange’a:
L1 =

1 −
µ
3
1/3
, 0

(5)
L2 =

1 + µ
3
1/3
, 0

(6)
L3 =

−

1 +
5µ
12

, 0

(7)
L4 =
1
2
− µ,
√
3
2

(8)
L5 =
1
2
− µ, −
√
3
2

(9)
(c) Narysuj trajektorię ruchu asteroidy (x(t), y(t)).
(d) Narysuj trajektorię ruchu Ziemi, Księżyca oraz asteroidy w układzie inercjalnym, wykorzystując przekształcenie

X
Y

=

cost − sin t
sin t cost
 x
y

gdzie (x, y) to współrzędne w układzie synodycznym, a (X, Y ) to współrzędne w układzie inercjalnym.
Na jaką najbliższą odległość asteroida zbliży się do Ziemi?
2
(e) Narysuj wykres prędkości asteroidy w układzie inercjalnym, wykorzystując
przekształcenie

U
V

=

cost − sin t
sin t cost
 u
v

+

sin t cost
− cost sin t
 x
y

gdzie (u, v) to prędkość w układzie synodycznym, a (U, V ) to prędkość w
układzie inercjalnym.
(f) Narysuj wykresy energii kinetycznej T oraz energii potencjalnej U asteroidy
w funkcji czasu:
T =
1
2
(u
2 + v
2
)
U =
1
2
(x
2 + y
2
) + 1 − µ
r1
+
µ
r2
(g) Sprawdź, czy otrzymane numerycznie rozwiązania zachowują niezmiennik
równania – całkę Jacobiego C:
C = x
2 + y
2 + 2
1 − µ
r1
+ 2
µ
r2
− u
2 − v
2
Narysuj wykres wartości C w funkcji czasu.
3